#ifndef SYMBOLICS_H_INCLUDED
#define SYMBOLICS_H_INCLUDED

#include <string>
#include <map>
//#include <>
namespace Symbolics
{
    enum Result
    {
        resError=0,
        resOk,
        resOverflow,
    };
    enum ValueType

    {
        typeConst,
        typeVariable,
        typeBlock,
        typeFunction,
        typeExpression,
        ADD,
        SUB,
        MUL,
        DIV,
    };

    class Value
    {
    protected:
        ValueType type;
    public:
        Value(ValueType t):type(t){}
        virtual ~Value(){}
        virtual int get(float &v)
        {
            v=0;
            return resOk;
        }
    };
    class Constant: public Value
    {
    public:
        Constant(float val):value(val),Value(typeConst){}
        virtual int get(float &v)
        {
            v=value;
            return resOk;
        }
    protected:
        float value;
    };
    class Arythmetic: public Value
    {
    public:
        Arythmetic(ValueType t,Value *l,Value *r):Value(t),left(l),right(r)
        {}
        Arythmetic(ValueType t,Value &l,Value &r):Value(t),left(&l),right(&r)
        {}
        virtual int get(float &v)
        {
            if(!left || !right)
                return resError;
            float l=0,r=0;
            int res_l=left->get(l);
            int res_r=right->get(r);
            if(res_l!=resOk || res_r!=resOk)
                return resError;
            float res=0;
            switch(type)
            {
            case ADD:
                v=l+r;
            break;
            case SUB:
                v=l-r;
            break;
            case MUL:
                v=l*r;
            break;
            case DIV:
                v=l/r;
            break;
            }
            return resOk;
        }
    protected:
        Value *left,*right;
    };

    class Expression: public Value
    {
        std::string source;
        Value *root;
        struct Token;
    public:
        Expression(const char *string)
            :source(string),root(NULL),Value(typeExpression)
        {}
        int build();
        int level_1(int l);
        int level_2(int l);
        int level_3(int l);
        virtual int get(float &v)
        {
            if(!root)
                return resError;
            return root->get(v);
        }

    };
}
#endif // SYMBOLICS_H_INCLUDED
