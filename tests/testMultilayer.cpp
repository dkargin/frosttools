#include "stdafx.h"
#include "../frostTools.h"


namespace Test
{
struct Test2
{
	int i;
	float f;
};
void testMultilayer()
{
	int arrayIntOut[10]={0,1,2,3,4,5,6,7,8,9},arrayIntIn[10];
	float arrayFloatOut[10]={.9f,.8f,.7f,.6f,.5f,.4f,.3f,.2f,.1f,0.0f},arrayFloatIn[10];

	/*class Test1: public Multilayer
	{		
	public:
		int l1;
		int l2;
		Test1()
		{
			l1=addLayer(sizeof(int));
			l2=addLayer(sizeof(float));	
		}
	};
	Test1 multilayer;
	int l1=multilayer.addLayer(sizeof(int));
	int l2=multilayer.addLayer(sizeof(float));	
	multilayer.allocNodes(10);
	multilayer.writeLayer(l1,arrayIntOut);
	multilayer.writeLayer(l2,arrayFloatOut);

	multilayer.readLayer(l1,arrayIntIn);
	multilayer.readLayer(l2,arrayFloatIn);

	Test2 *p=(Test2*)multilayer.getNodePointer(1);*/
}
};