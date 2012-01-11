#include "stdafx.h"
#include "process.h"
#include <list>

namespace Test
{
	//template<class Container>
	//static unsigned int __stdcall fillNempty(void *param)
	//{
	//	int size=reinterpret_cast<int>(param);
	//	Container<int> local;
	//	while(true)
	//	{			
	//		// 1. fill
	//		for(int i=0;i<size;i++)
	//			local.push_back(size);
	//		// 2. change
	//		for(Container<int>::iterator it=local.begin();it!=local.end();it++)
	//		{
	//			*it=*it*2+1;
	//		}
	//		// 3. empty			
	//		for(int i=0;i<size;i++)
	//			local.pop_back();
	//	}
	//}
	//typedef unsigned int (*pfn)(void*);
	//void testMultithreaded()
	//{
	//	const int maxSize=300;
	//	uintptr_t thread[]=
	//	{
	//		_beginthreadex(0,0,reinterpret_cast<pfn>(&fillNempty<std::list>),reinterpret_cast<void*>(maxSize),0,0),
	//		_beginthreadex(0,0,reinterpret_cast<pfn>(&fillNempty<std::vector>),reinterpret_cast<void*>(maxSize),0,0)
	//	};
	//	while(true)
	//	{
	//		Sleep(5);
	//	}
	//	return;
	//}
}