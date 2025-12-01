/*
    =============================================================================
    JSON Parser - Copyright (c) 2025 Your Philipp Havemann
    =============================================================================

    All code in this file is original work and is protected under copyright law.
    Unauthorized copying, redistribution, or use without proper credit is strictly
    prohibited. 

    Usage is permitted **only** if proper credit is given to the original author.
    Any use of this code is entirely at the user's own risk. The author accepts
    no responsibility for any damage, loss, or issues arising from the use of
    this code.

    For questions or permissions beyond crediting, please contact the author.
    
    =============================================================================
*/


#ifndef JSONPARSER_HPP
#define JSONPARSER_HPP

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <stack>
#include "Token.hpp"
#include "TokenType.hpp"
#include "Lexer.hpp"

#define JSON_UNEXPECTED_TOKEN_TEXT "Unexpected Token while parsing JSON"

namespace JSON
{

    enum ValueType
    {
        STRING_LITERAL,
        NUMBER_LITERAL,
        OBJECT,
        ARRAY,
        BOOL_LITERAL,
        NULL_LITERAL,
        INVALID
    };

    struct BaseValue
    {
        BaseValue() {}
        virtual ~BaseValue() = default;

        BaseValue(const BaseValue&) = delete;
        BaseValue& operator=(const BaseValue&) = delete;

        BaseValue(BaseValue&&) noexcept = default;
        BaseValue& operator=(BaseValue&&) noexcept = default;
    };

    template<typename T>
    struct Value : BaseValue
    {
        T value;
        Value(T&& value) : BaseValue(), value{std::move(value)} {}

        Value(const Value&) = delete;
        Value& operator=(const Value&) = delete;

        Value(Value&& other) noexcept : value(std::move(other.value)) {}
        Value& operator=(Value&& other) noexcept {
            value = std::move(other.value);
            return *this;
        }

        T& GetValue() { return value;}
        const T& GetValue() const { return value; }

    };

    struct JSONElement
    {
        ValueType valueType;
        std::unique_ptr<BaseValue> value;

        JSONElement() 
        : valueType(ValueType::INVALID), value(nullptr) {}

        JSONElement(ValueType t, std::unique_ptr<BaseValue>&& baseValue)
            : valueType(t),
            value(std::move(baseValue))
        {}

        JSONElement(const JSONElement&) = delete;
        JSONElement& operator=(const JSONElement&) = delete;

        JSONElement(JSONElement&&) noexcept = default;
        JSONElement& operator=(JSONElement&&) noexcept = default;

        ValueType GetValueType() { return valueType; }

        template<typename T>
        Value<T>& GetValueAs() { throw std::runtime_error("Type conversion to provided type not allowed!"); }
    };

    typedef std::unordered_map<std::string, JSONElement> JSONContainer;
    typedef std::vector<JSONElement> JSONArrayContainer;

    class JSONParser
    {
        public:

            JSONParser(const std::string fname)
            : lexer{fname}
            {
                
            }

            JSONElement Parse()
            {
                lexer.ReadSourceFile();
                tokens = lexer.scanTokens();
                if(Peek().tokenType != TokenType::LBRACE && Peek().tokenType != TokenType::LBRACKET)
                    throw std::runtime_error("Invalid token at start of file!");
                if(Peek().tokenType == TokenType::LBRACE) { GetNext(); return BeginParseObject(); }
                if(Peek().tokenType == TokenType::LBRACKET) { GetNext(); return BeginParseArray(); }
                return {JSON::ValueType::INVALID, nullptr};
            }

        private:

            JSONElement BeginParseObject()
            {
                JSONContainer container;
                JSONElement result{JSON::ValueType::OBJECT, nullptr};

                while(!isEnd())
                {
                    auto key = GetNext();

                    if(key.getTokenType() == TokenType::RBRACE) 
                    {
                        if(container.size() != 0) throw std::runtime_error("Unexpected token! To early close of object!");
                        result.value = std::make_unique<Value<JSON::JSONContainer>>(std::move(container));
                        return result; // early close for empty object;
                    }

                    auto delimiter = GetNext();
                    auto value = GetNext();

                    if(key.tokenType != TokenType::STRING) throw std::runtime_error(JSON_UNEXPECTED_TOKEN_TEXT);
                    if(delimiter.tokenType != TokenType::COLON) throw std::runtime_error(JSON_UNEXPECTED_TOKEN_TEXT);

                    std::string keyStr = key.GetLiteralValueAs<std::string>();

                    switch (value.tokenType)
                    {
                    case TokenType::STRING: 
                    {
                        auto valPtr = std::make_unique<Value<std::string>>(
                            std::move(value.GetLiteralValueAs<std::string>())
                        );

                        JSONElement element{ValueType::STRING_LITERAL, std::move(valPtr)};
                        container.emplace(std::move(keyStr), std::move(element));
                        break;
                    }
                    case TokenType::NUMBER: 
                    {
                        auto valPtr = std::make_unique<Value<double>>(
                            std::move(value.GetLiteralValueAs<double>())
                        );

                        JSONElement element{ValueType::NUMBER_LITERAL, std::move(valPtr)};
                        container.emplace(std::move(keyStr), std::move(element));
                        break;
                    }
                    case TokenType::TRUE_LITERAL: 
                    {
                        auto valPtr = std::make_unique<Value<bool>>( true );
                        JSONElement element{ValueType::BOOL_LITERAL, std::move(valPtr)};
                        container.emplace(std::move(keyStr), std::move(element));
                        break;
                    }
                    case TokenType::FALSE_LITERAL: 
                    {
                        auto valPtr = std::make_unique<Value<bool>>( false );
                        JSONElement element{ValueType::BOOL_LITERAL, std::move(valPtr)};
                        container.emplace(std::move(keyStr), std::move(element));
                        break;
                    }
                    case TokenType::NULL_LITERAL: 
                    {
                        JSONElement element{ValueType::NULL_LITERAL, nullptr};
                        container.emplace(std::move(keyStr), std::move(element));
                        break;
                    }
                    
                    case TokenType::LBRACE: 
                    {
                        container.emplace(std::move(keyStr), std::move(BeginParseObject()));
                        break;
                    }
                    case TokenType::LBRACKET: 
                    {
                        container.emplace(std::move(keyStr), std::move(BeginParseArray()));
                        break;
                    }


                    default:
                        throw std::runtime_error(JSON_UNEXPECTED_TOKEN_TEXT);
                    }

                    switch (Peek().tokenType)
                    {
                    case TokenType::COMMA: GetNext(); break;
                    case TokenType::RBRACE: 
                        GetNext(); 
                        result.value = std::make_unique<Value<JSON::JSONContainer>>(std::move(container));
                        return result;
                    default:
                        std::runtime_error(JSON_UNEXPECTED_TOKEN_TEXT);
                    }
                }
                result.value = std::make_unique<Value<JSON::JSONContainer>>(std::move(container));
                return result;
            }

            

            JSONElement BeginParseArray()
            {
                JSONArrayContainer container;
                JSONElement result{JSON::ValueType::ARRAY, nullptr};

                while(!isEnd())
                {
                    auto value = GetNext();

                    switch (value.tokenType)
                    {
                    case TokenType::STRING: 
                    {
                        auto valPtr = std::make_unique<Value<std::string>>(
                            std::move(value.GetLiteralValueAs<std::string>())
                        );

                        JSONElement element{ValueType::STRING_LITERAL, std::move(valPtr)};
                        container.emplace_back(std::move(element));
                        break;
                    }
                    case TokenType::NUMBER: 
                    {
                        auto valPtr = std::make_unique<Value<double>>(
                            std::move(value.GetLiteralValueAs<double>())
                        );

                        JSONElement element{ValueType::NUMBER_LITERAL, std::move(valPtr)};
                        container.emplace_back(std::move(element));
                        break;
                    }
                    case TokenType::TRUE_LITERAL: 
                    {
                        auto valPtr = std::make_unique<Value<bool>>( true );
                        JSONElement element{ValueType::BOOL_LITERAL, std::move(valPtr)};
                        container.emplace_back(std::move(element));
                        break;
                    }
                    case TokenType::FALSE_LITERAL: 
                    {
                        auto valPtr = std::make_unique<Value<bool>>( false );
                        JSONElement element{ValueType::BOOL_LITERAL, std::move(valPtr)};
                        container.emplace_back(std::move(element));
                        break;
                    }
                    case TokenType::NULL_LITERAL: 
                    {
                        JSONElement element{ValueType::NULL_LITERAL, nullptr};
                        container.emplace_back(std::move(element));
                        break;
                    }
                    
                    case TokenType::LBRACE: 
                    {
                        container.emplace_back(std::move(BeginParseObject()));
                        break;
                    }
                    case TokenType::LBRACKET: 
                    {
                        container.emplace_back(std::move(BeginParseArray()));
                        break;
                    }
                    case TokenType::RBRACKET: 
                        result.value = std::make_unique<Value<JSON::JSONArrayContainer>>(std::move(container));
                        return result; //Early exit [ ] <- empty or also directly after a value

                    default:
                        throw std::runtime_error(JSON_UNEXPECTED_TOKEN_TEXT);
                    }

                    switch (Peek().tokenType)
                    {
                    case TokenType::COMMA: 
                        GetNext(); 
                        break;
                    case TokenType::RBRACKET: 
                        GetNext(); 
                        result.value = std::make_unique<Value<JSON::JSONArrayContainer>>(std::move(container));
                        return result;
                    default:
                        std::runtime_error(JSON_UNEXPECTED_TOKEN_TEXT);
                    }
                }
                result.value = std::make_unique<Value<JSON::JSONArrayContainer>>(std::move(container));
                return result;
            }

            const Token& Peek() {
                return curser < tokens.size() ? tokens[curser] : EMPTY_TOKEN;
            }

            const Token& PeekPrev() {
                return curser - 1 > 0 ? tokens[curser - 1] : EMPTY_TOKEN;
            }

            const Token& PeekNext() {
                return curser + 1 < tokens.size() ? tokens[curser + 1] : EMPTY_TOKEN;
            }

            const Token& GetNext() {
                return tokens[curser++];
            }

            bool isEnd(){
                return curser >= tokens.size();
            }

            std::vector<Token> tokens;
            JsonLexer lexer;

            int curser = 0;

            const Token EMPTY_TOKEN{TokenType::INVALID, "", nullptr};
    };

}

template<>
JSON::Value<double>& JSON::JSONElement::GetValueAs<double>()
{
    if(this->valueType != JSON::ValueType::NUMBER_LITERAL) throw std::runtime_error("Value is not of type number");
    auto val = dynamic_cast<JSON::Value<double>*>(this->value.get());
    if (!val) throw std::runtime_error("Wrong type");
    return *val;
}

template<>
JSON::Value<std::string>& JSON::JSONElement::GetValueAs<std::string>()
{
    if(this->valueType != JSON::ValueType::STRING_LITERAL) throw std::runtime_error("Value is not of type string");
    auto val = dynamic_cast<JSON::Value<std::string>*>(this->value.get());
    if (!val) throw std::runtime_error("Wrong type");
    return *val;
}

template<>
JSON::Value<bool>& JSON::JSONElement::GetValueAs<bool>()
{
    if(this->valueType != JSON::ValueType::BOOL_LITERAL) throw std::runtime_error("Value is not of type bool");
    auto val = dynamic_cast<JSON::Value<bool>*>(this->value.get());
    if (!val) throw std::runtime_error("Wrong type");
    return *val;
}

template<>
JSON::Value<JSON::JSONArrayContainer>& JSON::JSONElement::GetValueAs<JSON::JSONArrayContainer>()
{
    if(this->valueType != JSON::ValueType::ARRAY) throw std::runtime_error("Value is not of type array");
    auto val = dynamic_cast<JSON::Value<JSON::JSONArrayContainer>*>(this->value.get());
    if (!val) throw std::runtime_error("Wrong type");
    return *val;
}

template<>
JSON::Value<JSON::JSONContainer>& JSON::JSONElement::GetValueAs<JSON::JSONContainer>()
{
    if(this->valueType != JSON::ValueType::OBJECT) throw std::runtime_error("Value is not of type object");
    auto val = dynamic_cast<JSON::Value<JSON::JSONContainer>*>(this->value.get());
    if (!val) throw std::runtime_error("Wrong type");
    return *val;
}

#endif // JSONPARSER_HPP