#include <iostream>
#include <crtdbg.h>

#include "math.h"
#include "frostTools.h"

using namespace std;

enum
{
	symbolTypeConst,
	symbolTypeVariable,
	symbolTypeArythmetic,
	symbolTypeFunction,
};
enum FnType
{
	typeSin,
	typeCos,
	typeTg,
	typeCtg,
	typeASin,
	typeACos,
	typeATg,
	typeACtg,
	typeExp,
	typeLog
};

enum
{
	operatorSum,
	operatorSub,
	operatorMul,
	operatorDiv,
};

class MathSymbol;
ostream & operator << (ostream &stream,MathSymbol &symb);
class MathSymbol
{	
	class Value
	{
	public:		
		virtual float calc()=0;
		virtual Value & addRef()=0;
		virtual void decRef()=0;	
		virtual void print(ostream &stream)=0;
		virtual int getType()const =0; 
		virtual int depth(int)=0;
		virtual Value* rebuild(int)=0;
		virtual bool equalOne()=0;
		virtual bool equalZero()=0;
		virtual int compare(const Value *val)=0;
		virtual Value * derivative(Value *val)=0;
	};
	class ValueConst: public Value
	{
	public:		
		float value;		

		ValueConst(float v)
			:value(v)
		{}

		explicit ValueConst(const ValueConst &v)
			:value(v.value)
		{}

		virtual float calc()
		{
			return value;
		}

		virtual Value & addRef()
		{			
#ifdef _DEBUG
			cout<<" ValueConst addRef"<<endl;		
#endif
			return *new ValueConst(*this);
		}

		virtual void decRef()
		{			
#ifdef _DEBUG
			cout<<" ValueConst decRef"<<endl;			
#endif
			delete this;
		}

		virtual void print(ostream &stream)
		{
			stream<<value;
		}

		virtual int getType() const
		{
			return symbolTypeConst;
		}

		virtual bool equalOne()
		{
			return value==1.0f;				
		}

		virtual bool equalZero()
		{
			return value==0.0f;
		}

		virtual Value * rebuild(int depth)
		{
			return this;
		}
		virtual int depth(int depth)
		{
			return depth+1;
		}
		virtual int compare(const Value *val)
		{
			if(val->getType()==symbolTypeConst)
			{
				ValueConst *v=(ValueConst*)val;
				return v->value==value;
			}
			return false;
		}

		virtual Value * derivative(Value *val)
		{
			return new ValueConst(0.0f);
		}
	};
	class ValueVariable: public Value
	{
	public:
		std::string name;
		float val;
		int numRef;

		ValueVariable(std::string n,float v)
			:val(v),numRef(0),name(n)
		{}

		explicit ValueVariable(const ValueVariable &v)
			:val(v.val),numRef(0),name(v.name)
		{}
		virtual float calc()
		{
			return val;
		}	
		virtual Value & addRef()
		{
			numRef++;
#ifdef _DEBUG
			cout<<" ValueVariable addRef, numRef="<<numRef<<endl;		
#endif
			return *this;
		}
		virtual void decRef()
		{		
			_ASSERT(numRef>=0);
			numRef--;
#ifdef _DEBUG
			cout<<" ValueVariable decRef, numRef="<<numRef<<endl;
#endif
			if(!numRef)
				delete this;
		}
		virtual void print(ostream &stream)
		{
			stream<<name.c_str();
		}
		virtual int getType() const
		{
			return symbolTypeVariable;
		}
		virtual bool equalOne()
		{
			return false;				
		}
		virtual bool equalZero()
		{
			return false;
		}
		virtual Value * rebuild(int depth)
		{
			return this;
		}
		virtual int depth(int depth)
		{
			return depth+1;
		}
		virtual int compare(const Value *val)
		{
			if(val->getType()==symbolTypeVariable)
			{
				ValueVariable *v=(ValueVariable*)val;
				return v==this;
			}
			return false;
		}
		virtual Value * derivative(Value *val)
		{
			if(compare(val))
				return new ValueConst(1.0f);
			else
				return new ValueConst(0.0f);
		}
	};

	class ValueArythmetic: public Value
	{		
	public:
		Value *a;
		Value *b;
		int operation;	

		explicit ValueArythmetic(Value &na,Value &nb,int op);
		ValueArythmetic(const ValueArythmetic &v);
		virtual ~ValueArythmetic();
		virtual Value & addRef();
		virtual void decRef();
		virtual float calc();
		static void _print_val(Value *val,ostream &stream);
		virtual void print(ostream &stream);
		virtual int getType() const ;
		virtual bool equalOne();
		virtual bool equalZero();
		virtual Value * rebuild(int depth);
		virtual int depth(int depth);
		virtual int compare(const Value *val);
		virtual Value * derivative(Value *val);
	};
	class ValueFunction : public Value
	{
		Value *argument;
		FnType fnType;
	public:
		ValueFunction(Value &val,FnType ft)
			:argument(&val.addRef()),fnType(ft)
		{}
		ValueFunction(const ValueFunction &val)
			:argument(&val.argument->addRef()),fnType(val.fnType)
		{}
		~ValueFunction()
		{
			if(argument)
				argument->decRef();		
		}
		
		virtual float calc()
		{
			switch(fnType)
			{
			case typeSin:
				return sinf(argument->calc());				
			case typeCos:
				return cosf(argument->calc());				
			case typeTg:
				return tanf(argument->calc());				
			case typeCtg:
				return 1/tanf(argument->calc());
			case typeASin:
				return 0;
			case typeACos:
				return 0;
			case typeATg:
				return 0;
			case typeACtg:
				return 0;
			case typeExp:
				return exp(argument->calc());
			case typeLog:
				return log(argument->calc());
			}
		}
		virtual Value & addRef()
		{
			return *new ValueFunction(*this);
		}
		virtual void decRef()
		{
			delete this;
		}
		virtual void print(ostream &stream)
		{
			switch(fnType)
			{
			case typeSin:
				stream<<"sin";				
				break;				
			case typeCos:
				stream<<"cos";				
				break;
			case typeTg:
				stream<<"tg";				
				break;
			case typeCtg:
				stream<<"ctg";				
				break;
			case typeASin:
				stream<<"asin";				
				break;
			case typeACos:
				stream<<"acos";				
				break;
			case typeATg:
				stream<<"atan";				
				break;
			case typeACtg:
				stream<<"actg";				
				break;
			case typeExp:
				stream<<"exp";				
				break;
			case typeLog:
				stream<<"ln";				
				break;
			}
			stream<<"(";
			argument->print(stream);
			stream<<")";
		}
		virtual int getType()const
		{
			return symbolTypeFunction;
		}
		virtual int depth(int i)
		{
			if(argument)
				return argument->depth(i+1);
		}
		virtual Value* rebuild(int)
		{
			return this;
		}
		virtual bool equalOne()
		{
			return false;
		}
		virtual bool equalZero()
		{
			return false;
		}
		virtual int compare(const Value *val)
		{
			return 0;
		}
		virtual Value * derivative(Value *val)
		{
			MathSymbol::Value *res=NULL;
			MathSymbol::Value *da=argument->derivative(val);
			switch(fnType)
			{
			case typeSin:
				res=new ValueArythmetic(ValueFunction(*argument,typeCos),*da,operatorMul);
				break;
				//return sinf(argument->calc());				
			case typeCos:
				//return cosf(argument->calc());				
			case typeTg:
				//return tanf(argument->calc());				
			case typeCtg:
				//return 1/tanf(argument->calc());
			case typeASin:
				//return 0;
			case typeACos:
				//return 0;
			case typeATg:
				//return 0;
			case typeACtg:
				//return 0;
			case typeExp:
				//return exp(argument->calc());
			case typeLog:
				//return log(argument->calc());
				break;
			}
			da->decRef();
			return res;
		}
	};
protected:
	Value *value;	
#ifdef _DEBUG
	int id;
#endif
	MathSymbol(Value *v)
		:value(&v->addRef())
	{
#ifdef _DEBUG
		id=idGenerator.genID();
		cout<<"MathSymbol constructor, id="<<id<<" statement={"<<*this<<'}'<<endl;		
#endif		
	}
public:
	MathSymbol(float v)
		:value(new ValueConst(v)),id(idGenerator.genID())
	{}
	MathSymbol()
		:value(NULL)
	{	

#ifdef _DEBUG
		id=idGenerator.genID();
		cout<<"MathSymbol constructor, id="<<id<<" statement={NULL}"<<endl;
#endif		
	}	
	MathSymbol(MathSymbol &m)
	{
		value=&m.value->addRef();
		id=idGenerator.genID();
	}
	MathSymbol(const char *name,float v)		
	{
		value=new ValueVariable(name,v);
		value->addRef();
#ifdef _DEBUG
		id=idGenerator.genID();
		cout<<"MathSymbol constructor, id="<<id<<" statement={"<<*this<<'}'<<endl;		
#endif
	}
	~MathSymbol()
	{
#ifdef _DEBUG
		cout<<"MathSymbol destructor, id="<<id<<" statement={"<<*this<<'}'<<endl;
		idGenerator.freeID(id);
#endif
		if(value)
			value->decRef();
	}
	virtual float calc()
	{
		return value->calc();
	}
	void rebuild()
	{
		value=value->rebuild(0);
	}
	virtual MathSymbol &operator = (MathSymbol &s)
	{
		if(value)
			value->decRef();
		value=&s.value->addRef();
		return *this;
	}
	MathSymbol derivative(MathSymbol symb)
	{
		MathSymbol res;
		res.value=value->derivative(symb.value);
		return res;
	}
	static IDGenerator<int> idGenerator;

	friend MathSymbol operator + (MathSymbol &a,MathSymbol &b); 
	friend MathSymbol operator + (MathSymbol &a,const float &b);
	friend MathSymbol operator + (const float &a,MathSymbol &b);
	friend MathSymbol operator - (MathSymbol &a,MathSymbol &b);
	friend MathSymbol operator - (MathSymbol &a,const float &b);
	friend MathSymbol operator - (const float &a,MathSymbol &b);
	friend MathSymbol operator * (MathSymbol &a,MathSymbol &b);
	friend MathSymbol operator * (MathSymbol &a,const float &b);
	friend MathSymbol operator * (const float &a,MathSymbol &b);
	friend MathSymbol operator / (MathSymbol &a,MathSymbol &b);
	friend MathSymbol operator / (MathSymbol &a,const float &b);
	friend MathSymbol operator / (const float &a,MathSymbol &b);
	friend ostream & operator << (ostream &stream,MathSymbol &symb);

	friend MathSymbol mSin(MathSymbol &s);
	friend MathSymbol mCos(MathSymbol &s);
	friend MathSymbol mTg(MathSymbol &s);
};
inline MathSymbol mSin(MathSymbol &s)
{
	return MathSymbol(&MathSymbol::ValueFunction(*s.value,typeSin));
}
inline MathSymbol mCos(MathSymbol &s)
{
	return MathSymbol(&MathSymbol::ValueFunction(*s.value,typeCos));
}
inline MathSymbol mTg(MathSymbol &s)
{
	return MathSymbol(&MathSymbol::ValueFunction(*s.value,typeTg));
}

MathSymbol operator + (MathSymbol &a,MathSymbol &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(*a.value,*b.value,operatorSum));
}
MathSymbol operator + (MathSymbol &a,const float &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(*a.value,MathSymbol::ValueConst(b),operatorSum));
}
MathSymbol operator + (const float &a,MathSymbol &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(MathSymbol::ValueConst(a),*b.value,operatorSum));
}

MathSymbol operator - (MathSymbol &a,MathSymbol &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(*a.value,*b.value,operatorSub));
}
MathSymbol operator - (MathSymbol &a,const float &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(*a.value,MathSymbol::ValueConst(b),operatorSub));
}
MathSymbol operator - (const float &a,MathSymbol &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(MathSymbol::ValueConst(a),*b.value,operatorSub));
}

MathSymbol operator * (MathSymbol &a,MathSymbol &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(*a.value,*b.value,operatorMul));
}
MathSymbol operator * (MathSymbol &a,const float &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(*a.value,MathSymbol::ValueConst(b),operatorMul));
}
MathSymbol operator * (const float &a,MathSymbol &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(MathSymbol::ValueConst(a),*b.value,operatorMul));
}

MathSymbol operator / (MathSymbol &a,MathSymbol &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(*a.value,*b.value,operatorDiv));
}
MathSymbol operator / (MathSymbol &a,const float &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(*a.value,MathSymbol::ValueConst(b),operatorDiv));
}
MathSymbol operator / (const float &a,MathSymbol &b)
{	
	return MathSymbol(&MathSymbol::ValueArythmetic(MathSymbol::ValueConst(a),*b.value,operatorDiv));
}
#ifdef _DEBUG
IDGenerator<int> MathSymbol::idGenerator;
#endif
ostream & operator << (ostream &stream,MathSymbol &symb)
{
	symb.value->print(stream);
	return stream;
}

////////////////////////////////////////////////////////////////////
// ValueArythmetic implementation
////////////////////////////////////////////////////////////////////

MathSymbol::ValueArythmetic::ValueArythmetic(Value &na,Value &nb,int op)
	:a(&na.addRef()),b(&nb.addRef()),operation(op)
{}
MathSymbol::ValueArythmetic::ValueArythmetic(const ValueArythmetic &v)
	:a(&v.a->addRef()),b(&v.b->addRef()),operation(v.operation)
{}		
MathSymbol::ValueArythmetic::~ValueArythmetic()
{
	if(a)
		a->decRef();
	if(b)
		b->decRef();
}
MathSymbol::Value & MathSymbol::ValueArythmetic::addRef()
{
#ifdef _DEBUG
	cout<<" Arythmetic addRef"<<endl;
#endif
	return *new ValueArythmetic(*this);
}
void MathSymbol::ValueArythmetic::decRef()
{
#ifdef _DEBUG
	cout<<" Arythmetic decRef"<<endl;	
#endif
	delete this;
}
float MathSymbol::ValueArythmetic::calc()
{
	switch(operation)
	{
	case operatorSum: return a->calc() + b->calc();
	case operatorSub: return a->calc() - b->calc();
	case operatorMul: return a->calc() * b->calc();
	case operatorDiv: return a->calc() / b->calc();
	}
}

void MathSymbol::ValueArythmetic::_print_val(MathSymbol::Value *val,ostream &stream)
{
	int d=val->depth(0);
	if(d>1)
	{
		stream<<'(';
		val->print(stream);
		stream<<')';

	}
	else
		val->print(stream);
}
void MathSymbol::ValueArythmetic::print(ostream &stream)
{			
	switch(operation)
	{
	case operatorSum: 
		a->print(stream);
		stream<<"+";
		b->print(stream);
		break;
	case operatorSub:
		a->print(stream);
		stream<<"-";
		b->print(stream);
		break;
	case operatorMul: 
		_print_val(a,stream);
		stream<<'*';
		_print_val(b,stream);				
		break;
	case operatorDiv: 
		_print_val(a,stream);
		stream<<'/';
		_print_val(b,stream);
		break;
	}
}
int MathSymbol::ValueArythmetic::getType() const 
{
	return symbolTypeArythmetic;
}
bool MathSymbol::ValueArythmetic::equalOne()
{
	return false;				
}
bool MathSymbol::ValueArythmetic::equalZero()
{
	return false;
}
MathSymbol::Value * MathSymbol::ValueArythmetic::rebuild(int depth)
{			
	a=a->rebuild(0);
	b=b->rebuild(0);
	int wa=a->depth(0);
	int wb=b->depth(0);
	if(wa>wb)
	{
		Value *tmp=a;a=b;b=tmp;
	}
	if((operation==operatorMul || operation==operatorDiv) && a->equalZero())
	{
		decRef();
		return new ValueConst(0.0f);
	}
	if(operation==operatorMul && b->equalZero())
	{
		decRef();
		return new ValueConst(0.0f);
	}
	if(operation==operatorMul && a->equalOne())
	{
		Value *res=&b->addRef();
		decRef();
		return res;
	}
	if((operation==operatorMul || operation==operatorDiv) && b->equalOne())
	{
		Value *res=&a->addRef();
		decRef();
		return res;
	}
	if(operation==operatorSum && a->equalZero())
	{
		Value *res=&b->addRef();
		decRef();
		return res;
	}
	if(operation==operatorSum && b->equalZero())
	{
		Value *res=&a->addRef();
		decRef();
		return res;
	}
	if(operation==operatorSub  && a->equalZero())
	{
		Value *res=&b->addRef();
		decRef();
		return res;
	}
	if(operation==operatorSub && b->equalZero())
	{
		Value *res=&a->addRef();
		decRef();
		return res;
	}
	if(a->getType()==symbolTypeConst && b->getType()==symbolTypeConst)
	{
		ValueConst *val=new ValueConst(calc());
		decRef();
		return val;
	}
	return this;
}
int MathSymbol::ValueArythmetic::depth(int depth)
{
	return max(a->depth(depth),b->depth(depth))+1;
}
int MathSymbol::ValueArythmetic::compare(const MathSymbol::Value *val)
{
	if(val->getType()==symbolTypeArythmetic)
	{
		ValueArythmetic *v=(ValueArythmetic*)val;
		return	(a->compare(v->a) && b->compare(v->b)) ||
				(a->compare(v->b) && b->compare(v->a));
	}
	return false;
}
MathSymbol::Value * MathSymbol::ValueArythmetic::derivative(MathSymbol::Value *val)
{
	// считаем что объекты динамические, ссылка уже существует, decRef после использования
	MathSymbol::Value *da=a->derivative(val);
	MathSymbol::Value *db=b->derivative(val);
	MathSymbol::Value *res=NULL;
	switch(operation)
	{
	case operatorSum: 				
		res=new MathSymbol::ValueArythmetic(*da,*db,operatorSum);
		break;
	case operatorSub:
		res=new MathSymbol::ValueArythmetic(*da,*db,operatorSub);
		break;
	case operatorMul:
		res=new MathSymbol::ValueArythmetic(
					MathSymbol::ValueArythmetic(*da,*b,operatorMul),
					MathSymbol::ValueArythmetic(*db,*a,operatorMul),operatorSum);
		break;
	case operatorDiv:
		res=new MathSymbol::ValueArythmetic(
					MathSymbol::ValueArythmetic(
						MathSymbol::ValueArythmetic(*da,*b,operatorMul),
						MathSymbol::ValueArythmetic(*db,*a,operatorMul),operatorSub),
					MathSymbol::ValueArythmetic(*b,*b,operatorSum),operatorDiv);
		break;

	}
	da->decRef();
	db->decRef();
	return res;
}
