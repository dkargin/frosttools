// FrostTools.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../frostTools.h"
#include <new>


namespace Test
{
void testManagers();
void testListeners();
void testMultilayer();
void testClassFactory();
void testMath();
void testDelegate();
void testMultithreaded();
};
void testClassFactory();
#include "iobuffer.h"
void testWrite(IOBuffer &buffer,int count)
{
	for(int i=0;i<count;i++)
		buffer.write(i);
}
void testRead(IOBuffer &buffer)
{
	int i=0;
	while(!buffer.eof())
	{
		int val;
		buffer.read(val);
		i++;
	}
}
void testBuffers()
{
	IOBuffer buffer(false);
	struct test
	{
		float fval;
		int ival;
	};
	test v1={10.0f,3};
	//testWrite(buffer,0xffff);
	buffer.write(v1);
	buffer.write(10);
	buffer.flip();
	test v2;
	buffer.read(v2);
	//testRead(buffer);
	return;
}


int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	Test::testManagers();
	//Test::testMath();
	//testListeners();
	//Test::testMultilayer();
	//Test::testMath();
	//Test::testDelegate();
	//testClassFactory();
	//Test::testMultithreaded();
	return 0;
}

