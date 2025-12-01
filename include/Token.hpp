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

#ifndef JSONTOKEN_HPP
#define JSONTOKEN_HPP

#include "TokenType.hpp"
#include <string>
#include <memory>

struct LiteralBase {
    virtual ~LiteralBase() = default;
    virtual std::unique_ptr<LiteralBase> clone() const = 0;
};

template <typename T>
struct LiteralValue : LiteralBase {
    T value;
    LiteralValue(T v) : value(std::move(v)) {}

    std::unique_ptr<LiteralBase> clone() const override {
        return std::make_unique<LiteralValue<T>>(value);
    }
};

struct Token
{
    TokenType tokenType;
    std::string lexeme;
    std::unique_ptr<LiteralBase> literal_data;

    Token(TokenType t, std::string text, std::unique_ptr<LiteralBase>&& lit)
        : tokenType(t),
          lexeme(std::move(text)),
          literal_data(std::move(lit))
    {}

    // Copy constructor
    Token(const Token& other)
        : tokenType(other.tokenType),
          lexeme(other.lexeme),
          literal_data(other.literal_data ? other.literal_data->clone() : nullptr)
    {}

    // Copy assignment
    Token& operator=(const Token& other) {
        if (this != &other) {
            tokenType = other.tokenType;
            lexeme = other.lexeme;
            literal_data = other.literal_data ? other.literal_data->clone() : nullptr;
        }
        return *this;
    }

    // Move constructor
    Token(Token&& other) noexcept
        : tokenType(other.tokenType),
        lexeme(std::move(other.lexeme)),
        literal_data(std::move(other.literal_data))
    {}

    // Move assignment
    Token& operator=(Token&& other) noexcept = default;

    std::string toString(){
        return TokenTypeToString(tokenType) + " " + lexeme;
    }

    TokenType getTokenType() {
        return tokenType;
    }

    std::string getLexeme() {
        return lexeme;
    }

    std::unique_ptr<LiteralBase>& getLiteral() {
        return literal_data;
    }

    template<typename T>
    T GetLiteralValueAs() const
    {
        throw std::runtime_error("No overload for this type exists");
    }

};

template<>
double Token::GetLiteralValueAs() const
{
    if(this->tokenType != TokenType::NUMBER) throw std::runtime_error("Value is not of type number");
    auto val = dynamic_cast<LiteralValue<double>*>(this->literal_data.get());
    if (!val) throw std::runtime_error("Wrong type");
    return val->value;
}

template<>
std::string Token::GetLiteralValueAs() const
{
    if(this->tokenType != TokenType::STRING) throw std::runtime_error("Value is not of type number");
    auto val = dynamic_cast<LiteralValue<std::string>*>(this->literal_data.get());
    if (!val) throw std::runtime_error("Wrong type");
    return val->value;
}

#endif //TOKEN_HPP