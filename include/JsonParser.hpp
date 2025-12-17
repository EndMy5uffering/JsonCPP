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
#include <ios>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <type_traits>
#include <stack>
#include "Token.hpp"
#include "TokenType.hpp"
#include "Lexer.hpp"

#define JSON_UNEXPECTED_TOKEN_TEXT "Unexpected Token while parsing JSON"

namespace JSON
{
    struct Element;
    
    struct Jsonify { 
        virtual void ToJsonObject(Element& obj) { Obj = false; } 
        virtual void ToJsonArray(Element& arr) { Arr = false; }
    private:
        bool Obj = true;
        bool Arr = true;

        friend Element;
    };

    typedef std::unordered_map<std::string, Element> JObject;
    typedef std::vector<Element> JArray;

    template<typename T> struct allowed_types       : std::false_type {};

    template<> struct allowed_types<double>         : std::true_type {};
    template<> struct allowed_types<int>            : std::true_type {};
    template<> struct allowed_types<long>           : std::true_type {};
    template<> struct allowed_types<float>          : std::true_type {};
    template<> struct allowed_types<std::string>    : std::true_type {};
    template<> struct allowed_types<bool>           : std::true_type {};
    template<> struct allowed_types<nullptr_t>      : std::true_type {};
    template<> struct allowed_types<JObject>     : std::true_type {};
    template<> struct allowed_types<JArray>      : std::true_type {};

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

    template<typename T>
    struct ValueType_of;

    template<> struct ValueType_of<double>      { static constexpr ValueType value = ValueType::NUMBER_LITERAL; };
    template<> struct ValueType_of<int>         { static constexpr ValueType value = ValueType::NUMBER_LITERAL; };
    template<> struct ValueType_of<long>        { static constexpr ValueType value = ValueType::NUMBER_LITERAL; };
    template<> struct ValueType_of<float>       { static constexpr ValueType value = ValueType::NUMBER_LITERAL; };
    template<> struct ValueType_of<std::string> { static constexpr ValueType value = ValueType::STRING_LITERAL; };
    template<> struct ValueType_of<bool>        { static constexpr ValueType value = ValueType::BOOL_LITERAL;   };
    template<> struct ValueType_of<nullptr_t>   { static constexpr ValueType value = ValueType::NULL_LITERAL;   };
    template<> struct ValueType_of<JObject>     { static constexpr ValueType value = ValueType::OBJECT;         };
    template<> struct ValueType_of<JArray>      { static constexpr ValueType value = ValueType::ARRAY;          };
    template<> struct ValueType_of<void>        { static constexpr ValueType value = ValueType::INVALID;        };

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
        virtual std::string GetValueAsString(int indent) noexcept = 0;

    private:
        virtual std::string ToStringFormater(int indent, int baseoffset) noexcept = 0;

        friend struct Element;
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
        std::string GetValueAsString(int indent) noexcept override { return ToStringFormater(indent, indent); }
    private:
        std::string ToStringFormater(int indent, int baseoffset) noexcept override { return ""; }
    };

    struct Element
    {
        ValueType valueType;
        std::unique_ptr<BaseValue> value;

        Element() 
        : valueType(ValueType::NULL_LITERAL), value(nullptr) {}

        Element(ValueType t, std::unique_ptr<BaseValue>&& baseValue)
            : valueType(t),
            value(std::move(baseValue))
        {}

        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;

        Element(Element&&) noexcept = default;
        Element& operator=(Element&&) noexcept = default;

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
        bool CanCastTo() { return this->valueType == ValueType_of<T>::value; }

        template<typename T>
        T& GetValueAs() 
        { 
            if(!allowed_types<T>::value) throw std::runtime_error("Type not supported");
            ValueType vtype = ValueType_of<T>::value;
            if(this->valueType != vtype) throw std::runtime_error("Value is not of type number");
            auto val = dynamic_cast<JSON::Value<T>*>(this->value.get());
            if (!val) throw std::runtime_error("Wrong type");
            return val->value;
        }

        template<typename T>
        const T& GetValueAs() const { throw std::runtime_error("No implementation for the given const type!\nOnly supported types: [ double, bool, std::string, JSON::JArray, JSON::JObject ]\n"); }

        template<typename T>
        bool TryGetValueAs(T*& in) { 
            in = nullptr;
            if(!allowed_types<T>::value) return false;

            ValueType vtype = ValueType_of<T>::value;
            if(this->valueType != vtype) return false;
            
            auto val = dynamic_cast<JSON::Value<T>*>(this->value.get());
            if (!val) return false;
            in = &(val->value);
            return true;
        }

        std::string ToString() noexcept
        {
            if(this->valueType == ValueType::NULL_LITERAL) return "null";
            if(this->valueType == ValueType::INVALID) return "";
            if(this->value == nullptr) return "";
            return this->value->GetValueAsString();
        }

        std::string ToString(int indent) noexcept
        {
            return GetStringFormated(0, indent);
        }

        std::string GetTypeAsString()
        {
            return ValueTypeToString(valueType);
        }

        std::string GetStringFormated(int indent, int offset) noexcept
        {
            if(this->valueType == ValueType::NULL_LITERAL) return "null";
            if(this->valueType == ValueType::INVALID) return "";
            if(this->value == nullptr) return "";
            return this->value->ToStringFormater(indent, offset);
        }

        template<typename V>
        static inline Element From() 
        { 
            if(!allowed_types<V>::value) throw std::runtime_error("Unsopported type surplyed");
            ValueType vtype = ValueType_of<V>::value;
            V value{};
            return JSON::Element{vtype, std::make_unique<JSON::Value<V>>(std::move(value))};    
        }

        template<typename V>
        static inline Element From(V value) 
        { 
            if(!allowed_types<V>::value) throw std::runtime_error("Unsopported type surplyed");
            ValueType vtype = ValueType_of<V>::value;
            return JSON::Element{vtype, std::make_unique<JSON::Value<V>>(std::move(value))};    
        }

        template<typename V>
        bool Add(const std::string& key, V value)
        {
            if(this->valueType != ValueType::OBJECT) return false;
            if(!allowed_types<V>::value) return false;
            this->template GetValueAs<JObject>()[key] = Element::template From<V>(value);
            return true;
        }

        
        bool AddObject(const std::string& key, Jsonify& value, bool skipJsonifyObj = false)
        {
            if(this->valueType != ValueType::OBJECT) return false;
            
            Element obj = Element::From<JObject>();
            value.ToJsonObject(obj);
            if(!skipJsonifyObj && value.Obj)
            {
                this->template GetValueAs<JObject>()[key] = std::move(obj);
                return true;
            }

            Element arr = Element::From<JArray>();
            value.ToJsonArray(arr);
            if(value.Arr)
            {
                this->template GetValueAs<JObject>()[key] = std::move(arr);
                return true;
            }

            return false;
        }

        template<typename V>
        bool Add(V value)
        {
            if(this->valueType != ValueType::ARRAY) return false;
            if(!allowed_types<V>::value) return false;
            this->template GetValueAs<JArray>().emplace_back(Element::template From<V>(value));
            return true;
        }
        
        bool AddObject(Jsonify& value)
        {
            if(this->valueType != ValueType::ARRAY) return false;

            Element obj = Element::From<JObject>();
            value.ToJsonObject(obj);
            if(value.Obj)
            {
                this->template GetValueAs<JArray>().emplace_back(std::move(obj));
                return true;
            }

            Element arr = Element::From<JArray>();
            value.ToJsonArray(arr);
            if(value.Arr)
            {
                this->template GetValueAs<JArray>().emplace_back(std::move(arr));
                return true;
            }

            return false;
        }

        Element& operator[](const std::string& key)
        {
            if(this->valueType != ValueType::OBJECT) throw std::runtime_error("Instance not of type object! Access with key only on instances of type object!");
            return GetValueAs<JObject>()[key];
        }

        Element& operator[](size_t key)
        {
            if(this->valueType != ValueType::ARRAY) throw std::runtime_error("Instance not of type object! Access with key only on instances of type object!");
            return GetValueAs<JArray>()[key];
        }

        template<typename T>
        Element& operator<<(T args)
        {
            if(!allowed_types<T>::value) throw std::runtime_error("Type not allowed to assign!");
            if(this->valueType == ValueType::ARRAY) GetValueAs<JArray>().emplace_back(Element::template From<T>(args));
            if(this->valueType == ValueType::NULL_LITERAL) *this = Element::template From<T>(args);
            return *this;
        }

        template<typename T>
        bool operator>>(T& result)
        {
            if(this->valueType != ValueType_of<T>::value) return false;
            result = GetValueAs<T>();
            return true;
        }

        Element& operator=(Jsonify& value)
        {
            Element obj = From<JObject>();
            value.ToJsonObject(obj);
            
            Element arr = From<JArray>();
            value.ToJsonArray(arr);

            if(!value.Arr && !value.Obj) throw std::runtime_error("Can not assign Jsonify object with object and array json conversions!");

            if(value.Obj) *this = std::move(obj);
            if(value.Arr) *this = std::move(arr);
            return *this;
        }

        bool Remove(const std::string& key)
        {
            if(this->valueType != ValueType::OBJECT) false;
            return GetValueAs<JObject>().erase(key) > 0;
        }

        bool Remove(size_t at)
        {
            if(this->valueType != ValueType::ARRAY) false;
            auto& arr = GetValueAs<JArray>();
            arr.erase(arr.begin() + at);
            return true;
        }

        bool Remove()
        {
            this->value = std::make_unique<Value<nullptr_t>>(nullptr);
            this->valueType = ValueType::NULL_LITERAL;
            return true;
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

            Element Parse()
            {
                lexer.ReadSourceFile();
                return _parse();
            }

            Element Parse(const char* fname)
            {
                lexer.ReadSourceFile(fname);
                return _parse();
            }

            Element Parse(const std::string& fname)
            {
                lexer.ReadSourceFile(fname);
                return _parse();
            }

            Element Parse(const std::filesystem::path& fname)
            {
                lexer.ReadSourceFile(fname);
                return _parse();
            }

            bool SaveToFile(Element& jsonObj, const std::string& path) noexcept
            {
                std::filesystem::path p{path};
                if(!std::filesystem::exists(p.parent_path())) 
                {
                    std::filesystem::create_directories(p.parent_path());
                }
                std::fstream stream;
                stream.open(path, std::fstream::out);
                if(!stream) return false;
                stream << jsonObj.ToString(4);
                stream.close();
                return true;
            }

        private:

            Element _parse()
            {
                curser = 0;
                tokens = lexer.scanTokens();
                if(Peek().tokenType != TokenType::LBRACE && Peek().tokenType != TokenType::LBRACKET)
                    throw std::runtime_error("Invalid token at start of file!");
                if(Peek().tokenType == TokenType::LBRACE) { GetNext(); return BeginParseObject(); }
                if(Peek().tokenType == TokenType::LBRACKET) { GetNext(); return BeginParseArray(); }
                return {JSON::ValueType::INVALID, nullptr};
            }

            Element BeginParseObject()
            {
                JObject container;
                Element result{JSON::ValueType::OBJECT, nullptr};

                while(!isEnd())
                {
                    auto key = GetNext();

                    if(key.getTokenType() == TokenType::RBRACE) 
                    {
                        if(container.size() != 0) throw std::runtime_error("Unexpected token! To early close of object!");
                        result.value = std::make_unique<Value<JSON::JObject>>(std::move(container));
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

                        Element element{ValueType::STRING_LITERAL, std::move(valPtr)};
                        container.emplace(std::move(keyStr), std::move(element));
                        break;
                    }
                    case TokenType::NUMBER: 
                    {
                        auto valPtr = std::make_unique<Value<double>>(
                            std::move(value.GetLiteralValueAs<double>())
                        );

                        Element element{ValueType::NUMBER_LITERAL, std::move(valPtr)};
                        container.emplace(std::move(keyStr), std::move(element));
                        break;
                    }
                    case TokenType::TRUE_LITERAL: 
                    {
                        auto valPtr = std::make_unique<Value<bool>>( true );
                        Element element{ValueType::BOOL_LITERAL, std::move(valPtr)};
                        container.emplace(std::move(keyStr), std::move(element));
                        break;
                    }
                    case TokenType::FALSE_LITERAL: 
                    {
                        auto valPtr = std::make_unique<Value<bool>>( false );
                        Element element{ValueType::BOOL_LITERAL, std::move(valPtr)};
                        container.emplace(std::move(keyStr), std::move(element));
                        break;
                    }
                    case TokenType::NULL_LITERAL: 
                    {
                        Element element{ValueType::NULL_LITERAL, nullptr};
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
                        result.value = std::make_unique<Value<JSON::JObject>>(std::move(container));
                        return result;
                    default:
                        std::runtime_error(JSON_UNEXPECTED_TOKEN_TEXT);
                    }
                }
                result.value = std::make_unique<Value<JSON::JObject>>(std::move(container));
                return result;
            }

            

            Element BeginParseArray()
            {
                JArray container;
                Element result{JSON::ValueType::ARRAY, nullptr};

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

                        Element element{ValueType::STRING_LITERAL, std::move(valPtr)};
                        container.emplace_back(std::move(element));
                        break;
                    }
                    case TokenType::NUMBER: 
                    {
                        auto valPtr = std::make_unique<Value<double>>(
                            std::move(value.GetLiteralValueAs<double>())
                        );

                        Element element{ValueType::NUMBER_LITERAL, std::move(valPtr)};
                        container.emplace_back(std::move(element));
                        break;
                    }
                    case TokenType::TRUE_LITERAL: 
                    {
                        auto valPtr = std::make_unique<Value<bool>>( true );
                        Element element{ValueType::BOOL_LITERAL, std::move(valPtr)};
                        container.emplace_back(std::move(element));
                        break;
                    }
                    case TokenType::FALSE_LITERAL: 
                    {
                        auto valPtr = std::make_unique<Value<bool>>( false );
                        Element element{ValueType::BOOL_LITERAL, std::move(valPtr)};
                        container.emplace_back(std::move(element));
                        break;
                    }
                    case TokenType::NULL_LITERAL: 
                    {
                        Element element{ValueType::NULL_LITERAL, nullptr};
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
                        result.value = std::make_unique<Value<JSON::JArray>>(std::move(container));
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
                        result.value = std::make_unique<Value<JSON::JArray>>(std::move(container));
                        return result;
                    default:
                        std::runtime_error(JSON_UNEXPECTED_TOKEN_TEXT);
                    }
                }
                result.value = std::make_unique<Value<JSON::JArray>>(std::move(container));
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

    namespace
    {
        inline std::string GetIndents(int indent)
        {
            return std::string(indent, ' ');
        }
    } // namespace
}

template<>
std::string JSON::Value<std::string>::GetValueAsString() noexcept { return "\"" + this->value + "\""; }
template<>
std::string JSON::Value<double>::GetValueAsString() noexcept { return std::to_string(this->value); }
template<>
std::string JSON::Value<float>::GetValueAsString() noexcept { return std::to_string(this->value); }
template<>
std::string JSON::Value<int>::GetValueAsString() noexcept { return std::to_string(this->value); }
template<>
std::string JSON::Value<long>::GetValueAsString() noexcept { return std::to_string(this->value); }
template<>
std::string JSON::Value<bool>::GetValueAsString() noexcept { return this->value ? "true" : "false"; }
template<>
std::string JSON::Value<JSON::JArray>::GetValueAsString() noexcept {
    std::stringstream ss;
    ss << "[ ";
    int count = 0;
    for(auto& val : value)
    {
        count++;
        ss << val.ToString() << ((count < value.size()) ? ", " : " ]");
    } 
    return ss.str(); 
}
template<>
std::string JSON::Value<JSON::JObject>::GetValueAsString() noexcept 
{
    std::stringstream ss;
    ss << "{ ";
    int count = 0;
    for(auto& [key, val] : value)
    {
        count++;
        ss << "\"" << key << "\"" << ": " << val.ToString() << ((count < value.size()) ? ", " : " }");
    }
    return ss.str();
}



template<>
std::string JSON::Value<std::string>::ToStringFormater(int indent, int offset) noexcept { return "\"" + this->value + "\""; }
template<>
std::string JSON::Value<double>::ToStringFormater(int indent, int offset) noexcept { return std::to_string(this->value); }
template<>
std::string JSON::Value<int>::ToStringFormater(int indent, int offset) noexcept { return std::to_string(this->value); }
template<>
std::string JSON::Value<long>::ToStringFormater(int indent, int offset) noexcept { return std::to_string(this->value); }
template<>
std::string JSON::Value<float>::ToStringFormater(int indent, int offset) noexcept { return std::to_string(this->value); }
template<>
std::string JSON::Value<bool>::ToStringFormater(int indent, int offset) noexcept { return this->value ? "true" : "false"; }
template<>
std::string JSON::Value<JSON::JArray>::ToStringFormater(int indent, int offset) noexcept {
    std::stringstream ss;
    std::string spacing = JSON::GetIndents(indent);
    std::string ValSpacing = JSON::GetIndents(indent + offset);
    if(value.size() == 0) { ss << "[]"; return ss.str(); }
    ss << "[\n";
    int count = 0;
    for(auto& val : value)
    {
        count++;
        ss << ValSpacing << val.GetStringFormated(indent + offset, offset) << ((count < value.size()) ? ",\n" : "\n");
    } 
    ss << spacing << "]";
    return ss.str(); 
}
template<>
std::string JSON::Value<JSON::JObject>::ToStringFormater(int indent, int offset) noexcept 
{
    std::stringstream ss;
    std::string spacing = JSON::GetIndents(indent);
    std::string KeySpacing = JSON::GetIndents(indent + offset);
    if(value.size() == 0) { ss << "{}"; return ss.str(); }
    ss << "{\n";
    int count = 0;
    for(auto& [key, val] : value)
    {
        count++;
        ss << KeySpacing << "\"" << key << "\"" << ": " << val.GetStringFormated(indent + offset, offset) << ((count < value.size()) ? ",\n" : "\n");
    }
    ss << spacing << "}";
    return ss.str();
}

template<>
bool JSON::Element::Add<JSON::Element&&>(const std::string& key, JSON::Element&& value)
{
    if(this->valueType != ValueType::OBJECT) return false;
    this->template GetValueAs<JObject>()[key] = std::forward<JSON::Element>(value);
    return true;
}

template<>
bool JSON::Element::Add<JSON::Element&&>(JSON::Element&& value)
{
    if(this->valueType != ValueType::ARRAY) return false;
    this->template GetValueAs<JArray>().emplace_back(std::forward<JSON::Element>(value));
    return true;
}

#undef JSON_UNEXPECTED_TOKEN_TEXT

#endif // JSONPARSER_HPP