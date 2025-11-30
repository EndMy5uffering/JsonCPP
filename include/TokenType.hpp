#ifndef JSONTOKENTYPE_HPP
#define JSONTOKENTYPE_HPP

#include <string>

enum class TokenType
{
    // punctuation
    LBRACE,       // {
    RBRACE,       // }
    LBRACKET,     // [
    RBRACKET,     // ]
    COMMA,        // ,
    COLON,        // :

    // literals
    STRING,       // "example"
    NUMBER,       // 123, -5.2e10
    TRUE_LITERAL, // true
    FALSE_LITERAL,// false
    NULL_LITERAL, // null

    // end
    END_OF_FILE,
    INVALID
};


static std::string TokenTypeToString(TokenType tokentype)
{
    switch (tokentype)
    {
        case TokenType::LBRACE:        return "LBRACE";
        case TokenType::RBRACE:        return "RBRACE";
        case TokenType::LBRACKET:      return "LBRACKET";
        case TokenType::RBRACKET:      return "RBRACKET";
        case TokenType::COMMA:         return "COMMA";
        case TokenType::COLON:         return "COLON";

        case TokenType::STRING:        return "STRING";
        case TokenType::NUMBER:        return "NUMBER";
        case TokenType::TRUE_LITERAL:  return "TRUE_LITERAL";
        case TokenType::FALSE_LITERAL: return "FALSE_LITERAL";
        case TokenType::NULL_LITERAL:  return "NULL_LITERAL";

        case TokenType::END_OF_FILE:   return "END_OF_FILE";
        case TokenType::INVALID:       return "INVALID";
    }

    return "UNKNOWN"; // in case someone adds a new enum and forgets the switch
}


#endif //JSONTOKENTYPE_HPP