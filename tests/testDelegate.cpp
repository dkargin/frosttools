#include "frosttools/delegate.hpp"
#include <stdio.h>

struct TestBase
{
	virtual void otherUselessFn()
	{}
};

struct TestStruct
{		
	virtual float memberFn(int a)
	{
		printf("TestStruct::memberFn(%d)",a);
		return 1.0;
	}		
};

struct TestStruct2: public TestStruct,public TestBase
{
	virtual float memberFn1(int b)
	{
		printf("TestStruct2::memberFn1(%d)",b);
		return 2.0;
	}	
};

int main(int argc, char * argv[])
{
	/*
	//Delegate(
	TestStruct2 t;
////		std::mem_fun_t
	TestStruct2 *t2=new TestStruct2;
	Delegate<float,int> d(t2,&TestStruct2::memberFn1);
	typedef float (*StaticFn)(int) ;
	typedef Delegate<float,int> delegate;
	delegate test(&t,&TestStruct::memberFn);
	int n=delegate::signature::N;
	float res=test(1.0f);
	delegate test2(t2,&TestStruct2::memberFn);
	res=test2(1.0f);
//
//		//static_assert(false);
//		n=test2.getSignatureSingle(t2).N;
		delete t2;*/
	return 0;
}
