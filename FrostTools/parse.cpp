#include "symbolics.h"
#include <vector>
#include <iostream>
using namespace Symbolics;
struct Expression::Token
{
    enum Type
    {
        typeUnknown,
        typeNumber,
        typeOperator,
        typeString,
        typeEnd,
    }type;

    const char *start;
    int length;
    Token()
        :start(NULL),length(0),type(typeUnknown)
    {}
    Token(const Token &token)
        :start(token.start),length(token.length),type(token.type)
    {}

    std::string get()
    {
        char *tmp=new char[length+1];
        memcpy(tmp,start,length);
        tmp[length]=0;
        std::string res(tmp);
        delete []tmp;
        return res;
    }
};

struct Template
{
    //Token::Type result;
};

bool isoperator(char ch)
{
    switch(ch)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '(':
        case ')':
            return true;
    }
    return false;
}
int Expression::build()
{
    //1. tokenise
    const char *ptr=source.c_str();
    const char *start=ptr;
    int len=0;
    std::vector<Token> tokens;
    Token::Type lastType=Token::typeUnknown;
    Token *last=NULL;
    while(*ptr)
    {
        Token::Type newType=Token::typeUnknown;
        char ch=*ptr;
        if(isalpha(ch))
            newType=Token::typeString;
        else if(isdigit(ch))
            newType=Token::typeNumber;
        else if(isoperator(ch))
            newType=Token::typeOperator;
        if(newType!=lastType)
        {
            tokens.push_back(Token());
            last=&tokens.back();
            last->start=ptr;
            last->length=1;
            last->type=newType;
        }
        else if(last)
        {
            last->length++;
        }
        lastType=newType;
        ptr++;
    }
    for(int i=0;i<tokens.size();i++)
        std::cout<<" Token <"<<tokens[i].get().c_str()<<" >\n";
    //2. build tree

    return 0;
}
