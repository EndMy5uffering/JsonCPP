#ifndef JSONLEXER_HPP
#define JSONLEXER_HPP

#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include "Token.hpp"

class JsonLexer
{
public:

    JsonLexer(const std::string fname) :
    source{},
    sourcePath{fname}
    {
    }

    void ReadSourceFile()
    {
        std::ifstream stream{sourcePath};
        if(!stream) throw std::runtime_error("Could not open file!");
        std::ostringstream ss;
        ss << stream.rdbuf();
        source = ss.str();
    }

    std::vector<Token> scanTokens() {
        while(!isEnd()){
            curserStart = curser;
            scanToken();
        }

        tokens.emplace_back(TokenType::END_OF_FILE, "", nullptr);
        return tokens;
    }

private:

    void scanToken() {

        char c = GetNext();
        switch(c){
            case '[': AddToken(TokenType::LBRACKET); break;
            case ']': AddToken(TokenType::RBRACKET); break;
            case '{': AddToken(TokenType::LBRACE); break;
            case '}': AddToken(TokenType::RBRACE); break;
            case ',': AddToken(TokenType::COMMA); break;
            case ':': AddToken(TokenType::COLON); break;
            case '"': ReadString(); break;
            case 't': ReadTrueLiteral(); break;
            case 'T': ReadTrueLiteral(); break;
            case 'f': ReadFalseLiteral(); break;
            case 'F': ReadFalseLiteral(); break;
            case 'n': ReadNullLiteral(); break;
            case 'N': ReadNullLiteral(); break;
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                break;
            default: 
                if(isDigit(c)){
                    ReadNumber();
                }else{
                    throw std::runtime_error("Unexpected character: \n[ " + GetRangeArroundCurrent(5) + " ]\n"); 
                }
                break;
        }

    }

    void ReadNumber() {
        while(isDigit(Peek())) GetNext();

        if(Peek() == '.' && isDigit(PeekNext())){
            GetNext();

            while(isDigit(Peek())) GetNext();
        }

        if((Peek() == 'e' || Peek() == 'E') && isDigit(PeekNext()))
        {
            GetNext();
        }
        while(isDigit(Peek())) GetNext();

        std::string tvalue = source.substr(curserStart, curser - curserStart);
        double tdvalue = atof(tvalue.c_str());

        std::unique_ptr<LiteralBase> lit =
            std::make_unique<LiteralValue<double>>(tdvalue);
        AddToken(TokenType::NUMBER, std::move(lit));
    }

    void ReadTrueLiteral() {
        char c = GetNext();
        if(c != 'r' && c != 'R') throw std::runtime_error("Melformed true value: \n[ " + GetRangeArroundCurrent(6) + " ]\n");
        c = GetNext();
        if(c != 'u' && c != 'U') throw std::runtime_error("Melformed true value: \n[ " + GetRangeArroundCurrent(6) + " ]\n");
        c = GetNext();
        if(c != 'e' && c != 'E') throw std::runtime_error("Melformed true value: \n[ " + GetRangeArroundCurrent(6) + " ]\n");
        std::unique_ptr<LiteralBase> lit =
            std::make_unique<LiteralValue<bool>>(true);
        AddToken(TokenType::TRUE_LITERAL, std::move(lit));
    }

    void ReadFalseLiteral() {
        char c = GetNext();
        if(c != 'a' && c != 'A') throw std::runtime_error("Melformed false value: \n[ " + GetRangeArroundCurrent(6) + " ]\n");
        c = GetNext();
        if(c != 'l' && c != 'L') throw std::runtime_error("Melformed false value: \n[ " + GetRangeArroundCurrent(6) + " ]\n");
        c = GetNext();
        if(c != 's' && c != 'S') throw std::runtime_error("Melformed false value: \n[ " + GetRangeArroundCurrent(6) + " ]\n");
        c = GetNext();
        if(c != 'e' && c != 'E') throw std::runtime_error("Melformed false value: \n[ " + GetRangeArroundCurrent(6) + " ]\n");
        std::unique_ptr<LiteralBase> lit =
            std::make_unique<LiteralValue<bool>>(false);
        AddToken(TokenType::FALSE_LITERAL, std::move(lit));
    }

    void ReadNullLiteral() {
        char c = GetNext();
        if(c != 'u' && c != 'U') throw std::runtime_error("Melformed Null value: \n[ " + GetRangeArroundCurrent(6) + " ]\n");
        c = GetNext();
        if(c != 'l' && c != 'L') throw std::runtime_error("Melformed Null value: \n[ " + GetRangeArroundCurrent(6) + " ]\n");
        c = GetNext();
        if(c != 'l' && c != 'L') throw std::runtime_error("Melformed Null value: \n[ " + GetRangeArroundCurrent(6) + " ]\n");
        AddToken(TokenType::NULL_LITERAL, nullptr);
    }

    bool isDigit(char c)
    {
        return isdigit(c) || c == '-';
    }

    void ReadString() {
        std::string value;

        while((Peek() != '"' || PeekPrev() == '\\') && !isEnd()){
            GetNext();
        }
        if(isEnd()){
            throw std::runtime_error("Unterminated String: \n[ " + GetRangeArroundCurrent(5) + " ]\n");
        }
        
        GetNext();

        value = source.substr(curserStart + 1, (curser - 1) - (curserStart + 1));

        std::unique_ptr<LiteralBase> lit =
            std::make_unique<LiteralValue<std::string>>(value);

        AddToken(TokenType::STRING, std::move(lit));
    }

    bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') || 
                (c >= 'A' && c <= 'Z') ||
                c == '_';
    }

    bool isAlphaNumerica(char c) {
        return isAlpha(c) || isdigit(c);
    }

    void AddToken(TokenType token) {
        AddToken(token, nullptr);
    }

    void AddToken(TokenType token, std::unique_ptr<LiteralBase> literal) {
        std::string text = source.substr(curserStart, curser - curserStart);
        tokens.emplace_back(token, text, std::move(literal));
    }

    char Peek() {
        return curser < source.size() ? source[curser] : '\0';
    }

    char PeekPrev() {
        return curser - 1 > 0 ? source[curser - 1] : '\0';
    }

    char PeekNext() {
        return curser + 1 < source.size() ? source[curser + 1] : '\0';
    }

    char GetNext() {
        return source[curser++];
    }

    bool isEnd(){
        return curser >= source.length();
    }

    std::string GetRangeArroundCurrent(int range)
    {
        int lower = curser - range > 0 ? curser - range : curser - range;
        int upper = curser + range < source.length() ? curser + range : source.length() - 1;
        return source.substr(lower, upper - lower);
    }

    std::vector<Token> tokens;
    std::string source;
    std::string sourcePath;

    int curserStart = 0;
    int curser = 0;
};

#endif //JSONLEXER_HPP