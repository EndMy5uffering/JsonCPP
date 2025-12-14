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
    struct JSONElement;

    typedef std::unordered_map<std::string, JSONElement> JSONObject;
    typedef std::vector<JSONElement> JSONArray;

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

    static std::string ValueTypeToString(ValueType type)
    {
        switch (type)
        {
        case JSON::ValueType::STRING_LITERAL:   return "STRING_LITERAL";
        case JSON::ValueType::NUMBER_LITERAL:   return "NUMBER_LITERAL";
        case JSON::ValueType::OBJECT:           return "OBJECT";
        case JSON::ValueType::ARRAY:            return "ARRAY";
        case JSON::ValueType::BOOL_LITERAL:     return "BOOL_LITERAL";
        case JSON::ValueType::NULL_LITERAL:     return "NULL_LITERAL";
        case JSON::ValueType::INVALID:          return "INVALID";
        default:
            return "INVALID_TYPE";
        }
    }

    struct BaseValue
    {
        BaseValue() {}
        virtual ~BaseValue() = default;

        BaseValue(const BaseValue&) = delete;
        BaseValue& operator=(const BaseValue&) = delete;

        BaseValue(BaseValue&&) noexcept = default;
        BaseValue& operator=(BaseValue&&) noexcept = default;

        virtual std::string GetValueAsString() noexcept = 0;

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

        std::string GetValueAsString() noexcept override { return ""; }
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

        bool operator==(ValueType type) noexcept
        {
            return this->valueType == type;
        }

        ValueType GetValueType() { return valueType; }
        bool IsOfType(ValueType type)
        {
            return this->valueType == type;
        }

        template<typename T>
        bool CanCastTo() { return false; }

        template<typename T>
        T& GetValueAs() { throw std::runtime_error("No implementation for the given type!\nOnly supported types: [ double, bool, std::string, JSON::JSONArray, JSON::JSONObject ]\n"); }

        template<typename T>
        const T& GetValueAs() const { throw std::runtime_error("No implementation for the given const type!\nOnly supported types: [ double, bool, std::string, JSON::JSONArray, JSON::JSONObject ]\n"); }

        template<typename T>
        bool TryGetValueAs(T*& in) { 
            std::cerr << "Unsupported type cast!\nOnly supports types: [ double, bool, std::string, JSON::JSONArray, JSON::JSONObject ]\n";
            in = nullptr; 
            return false; 
        }

        std::string GetValueAsString() noexcept
        {
            if(this->valueType == ValueType::NULL_LITERAL) return "null";
            if(this->valueType == ValueType::INVALID) return "";
            if(this->value == nullptr) return "";
            return this->value->GetValueAsString();
        }

        std::string GetTypeAsString()
        {
            return ValueTypeToString(valueType);
        }

    };

    class JSONParser
    {
        public:

            JSONParser()
            {}                

            JSONParser(const std::string fname)
            : lexer{fname}
            {}

            JSONParser(const char* fname)
            : lexer{fname}
            {}

            JSONParser(const std::filesystem::path& fname)
            : lexer{fname}
            {}

            JSONElement Parse()
            {
                lexer.ReadSourceFile();
                return _parse();
            }

            JSONElement Parse(const char* fname)
            {
                lexer.ReadSourceFile(fname);
                return _parse();
            }

            JSONElement Parse(const std::string& fname)
            {
                lexer.ReadSourceFile(fname);
                return _parse();
            }

            JSONElement Parse(const std::filesystem::path& fname)
            {
                lexer.ReadSourceFile(fname);
                return _parse();
            }

        private:

            JSONElement _parse()
            {
                curser = 0;
                tokens = lexer.scanTokens();
                if(Peek().tokenType != TokenType::LBRACE && Peek().tokenType != TokenType::LBRACKET)
                    throw std::runtime_error("Invalid token at start of file!");
                if(Peek().tokenType == TokenType::LBRACE) { GetNext(); return BeginParseObject(); }
                if(Peek().tokenType == TokenType::LBRACKET) { GetNext(); return BeginParseArray(); }
                return {JSON::ValueType::INVALID, nullptr};
            }

            JSONElement BeginParseObject()
            {
                JSONObject container;
                JSONElement result{JSON::ValueType::OBJECT, nullptr};

                while(!isEnd())
                {
                    auto key = GetNext();

                    if(key.getTokenType() == TokenType::RBRACE) 
                    {
                        if(container.size() != 0) throw std::runtime_error("Unexpected token! To early close of object!");
                        result.value = std::make_unique<Value<JSON::JSONObject>>(std::move(container));
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
                        result.value = std::make_unique<Value<JSON::JSONObject>>(std::move(container));
                        return result;
                    default:
                        std::runtime_error(JSON_UNEXPECTED_TOKEN_TEXT);
                    }
                }
                result.value = std::make_unique<Value<JSON::JSONObject>>(std::move(container));
                return result;
            }

            

            JSONElement BeginParseArray()
            {
                JSONArray container;
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
                        result.value = std::make_unique<Value<JSON::JSONArray>>(std::move(container));
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
                        result.value = std::make_unique<Value<JSON::JSONArray>>(std::move(container));
                        return result;
                    default:
                        std::runtime_error(JSON_UNEXPECTED_TOKEN_TEXT);
                    }
                }
                result.value = std::make_unique<Value<JSON::JSONArray>>(std::move(container));
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

#define GET_VALUE_AS(TYPE, VALUE_TYPE) \
template<> \
TYPE& JSON::JSONElement::GetValueAs<TYPE>() \
{ \
    if(this->valueType != VALUE_TYPE) throw std::runtime_error("Value is not of type number"); \
    auto val = dynamic_cast<JSON::Value<TYPE>*>(this->value.get()); \
    if (!val) throw std::runtime_error("Wrong type"); \
    return val->value; \
}


GET_VALUE_AS(double, JSON::ValueType::NUMBER_LITERAL)
GET_VALUE_AS(std::string, JSON::ValueType::STRING_LITERAL)
GET_VALUE_AS(bool, JSON::ValueType::BOOL_LITERAL)
GET_VALUE_AS(JSON::JSONObject, JSON::ValueType::OBJECT)
GET_VALUE_AS(JSON::JSONArray, JSON::ValueType::ARRAY)

#undef GET_VALUE_AS

#define TRY_GET_VALUE_AS(TYPE, VALUE_TYPE) \
template<> \
bool JSON::JSONElement::TryGetValueAs<TYPE>(TYPE*& result) \
{ \
    result = nullptr; \
    if(this->valueType != VALUE_TYPE) return false; \
    auto val = dynamic_cast<JSON::Value<TYPE>*>(this->value.get()); \
    if (!val) return false; \
    result = &(val->value); \
    return true; \
}

TRY_GET_VALUE_AS(double, JSON::ValueType::NUMBER_LITERAL)
TRY_GET_VALUE_AS(std::string, JSON::ValueType::STRING_LITERAL)
TRY_GET_VALUE_AS(bool, JSON::ValueType::BOOL_LITERAL)
TRY_GET_VALUE_AS(JSON::JSONObject, JSON::ValueType::OBJECT)
TRY_GET_VALUE_AS(JSON::JSONArray, JSON::ValueType::ARRAY)

#undef TRY_GET_VALUE_AS

#define CAN_CAST_TO(TYPE, VALUE_TYPE)\
template<>\
bool JSON::JSONElement::CanCastTo<TYPE>(){ return this->valueType == VALUE_TYPE; }

CAN_CAST_TO(double, JSON::ValueType::NUMBER_LITERAL)
CAN_CAST_TO(std::string, JSON::ValueType::STRING_LITERAL)
CAN_CAST_TO(bool, JSON::ValueType::BOOL_LITERAL)
CAN_CAST_TO(JSON::JSONObject, JSON::ValueType::OBJECT)
CAN_CAST_TO(JSON::JSONArray, JSON::ValueType::ARRAY)

#undef CAN_CAST_TO

template<>
std::string JSON::Value<std::string>::GetValueAsString() noexcept { return "\"" + this->value + "\""; }
template<>
std::string JSON::Value<double>::GetValueAsString() noexcept { return std::to_string(this->value); }
template<>
std::string JSON::Value<bool>::GetValueAsString() noexcept { return this->value ? "True" : "False"; }
template<>
std::string JSON::Value<JSON::JSONArray>::GetValueAsString() noexcept {
    std::stringstream ss;
    ss << "[ ";
    int count = 0;
    for(auto& val : value)
    {
        count++;
        ss << val.GetValueAsString() << ((count < value.size()) ? ", " : " ]");
    } 
    return ss.str(); 
}
template<>
std::string JSON::Value<JSON::JSONObject>::GetValueAsString() noexcept 
{
    std::stringstream ss;
    ss << "{ ";
    int count = 0;
    for(auto& [key, val] : value)
    {
        count++;
        ss << "\"" << key << "\"" << ": " << val.GetValueAsString() << ((count < value.size()) ? ", " : " }");
    }
    return ss.str();
}

#undef JSON_UNEXPECTED_TOKEN_TEXT

#endif // JSONPARSER_HPP