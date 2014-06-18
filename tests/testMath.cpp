#include "frosttools/3dmath.h"
#include "frosttools/ringbuffer.hpp"
#include <vector>
#include <stdio.h>

using namespace frosttools;
namespace Test
{
// solve square equation a*x^2+b*x+c=0
template<class Real> int solve(const Real &a,const Real &b,const Real &c,Real result[2])
{	
	if(a!=Real(0))
	{
		Real D=b*b-4*a*c;
		if(D<Real(0))
			return 0;
		else if(D==Real(0.0f))
		{
			result[0]=-b/(2*a);
			return 1;
		}
		else
		{
			D=sqrt(D);
			result[0]=(-b-D)/(2*a);
			result[1]=(-b+D)/(2*a);
			if(result[0]>result[1])
			{
				Real tmp=result[0];result[0]=result[1];result[1]=tmp;
			}
			return 2;
		}
	}
	else
	{
		if(b!=Real(0))
		{
			result[0]=-c/b;
			return 1;
		}
		return 0;
	}
}
// calc aiming target
// v1 - velocity of the projectile used to shot target down
vec3 getWeaponTarget(const geometry::Edge &edge,const vec3 &pos,float v1)
{
	vec3 result=pos;
	// �������� ��������� ��������� �� ����������	
	vec3 O=edge.project(pos);
	// ������, ���������������� ����������
	vec3 H=O-pos;
	float h=H.length();
	float l=edge.projectLen(pos);
	float impactTime[2]={0,0};
	float v0=edge.length();				// �������� ������� ������� ���� �����
	//float v1=def->velocity;				// �������� ������� ������� �������
	int res=solve(v0*v0-v1*v1,-2*l*v0,h*h+l*l,impactTime);
	if(res>0)
	{
		float time=impactTime[res-1];	// ���� 2 �����, ���� impactTime[1], ���� ���� ������ - impactTime[0]
		float vx=h;						// �������� � �����������, ���������������� ����������
		float vy=v0*time-l;				// �������� � �����������, ������������ ����������
		result=pos+(vecNormalize(H)*vx+edge.direction()*vy);
	}
	return result;
}

float testAiming(const geometry::Edge &edge,const vec3 &source,float velocity)
{	
	vec3 target=getWeaponTarget(edge,source,velocity);
	float t=vecLength(target-source)/velocity;
	vec3 predict=edge(t);
	float error=vecDistance(target,predict);
	printf("Aiming object(%g,%g,%g)-(%g,%g,%g), from (%g,%g,%g) with shot velocity %g\n",
			edge.start[0],edge.start[1],edge.start[2],
			edge.end[0],edge.end[1],edge.end[2],
			source[0],source[1],source[2],velocity);
	printf("-calculated impact at(%g,%g,%g)\n",target[0],target[1],target[2]);
	printf("-calculation error=%g\n",error);
	return error;
}

/*
void testRingBuffer()
{
	Ring<float,3> ring;
	ring.push_back(1);
	ring.push_back(2);
	ring.push_back(3);
	ring.push_back(4);
	float val[3];
	for(int i=0;i<ring.size();i++)
		val[i]=ring[i];
}*/

void testSphereIntersection()
{
	geometry::_Sphere<vec2f> s0(vec2f(100,0),100),s1(vec2f(230,0),50);
	vec2f res_v;
	float res_t;
	int res = geometry::intersection(s0,s1,&res_v,&res_t);
}

void testMatrix()
{
	Mt4x4 a = Mt4x4::identity(1.0),b=Mt4x4::identity(1.0);
	Mt4x4 c;
	c=a+b;
}

void testMatrix4r()
{
	float test[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	float row0[]={0,1,2,3};
	float row1[]={4,5,6,7};
	float col0[]={0,4,8,12};
	float col1[]={1,5,9,13};
	Matrix4<float,true> m(test);
	vec4f r0=m.row(0),r1=m.row(1);
	assert(m.row(0)==vec4f(row0) && "test row order Mt4x4.row(0)");
	assert(m.row(1)==vec4f(row1) && "test row order Mt4x4.row(1)");
	assert(m.col(0)==vec4f(col0) && "test row order Mt4x4.col(0)");
	assert(m.col(1)==vec4f(col1) && "test row order Mt4x4.col(1)");
}

void testMatrixNM()
{
	typedef MatrixNM<float> Mat;
	Mat mat(5,4);
	Mat::col_type c=mat.col(2);
}

float testMatrixInversion()
{
	typedef Matrix4<double,true> Mat;
	Mat::value_type source[]=
	{
		1,	4,	3,	8,
		0,	1,	1.2,0,
		3,	0,	1,	0,
		0.5,2,	2,	1,
	};

	Mat::value_type inverted[]=
	{
		-0.1388889,	-1.666667,	0.1944444,	1.111111,
		-0.5,	-5.0,	-0.5,	4.0,
		0.4166667,	5.0,	0.4166667,	-3.333333,
		0.2361111,	0.8333333,	0.06944444,	-0.8888889,
	};

	Mat s=invert(Mat(source)),check(inverted);
	Mat diff=check-s;
	Mat d=Mat(source)*invert(Mat(source));
	float dist=diff.length();
	return dist;
}

void testMatrix4c()
{
	float test[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	float row0[]={0,1,2,3};
	float row1[]={4,5,6,7};
	float col0[]={0,4,8,12};
	float col1[]={1,5,9,13};
	Matrix4<float,false> m(test);
	assert(m.col(0)==vec4f(row0) && "test row order Mt4x4.row(0)");
	assert(m.col(1)==vec4f(row1) && "test row order Mt4x4.row(1)");
	assert(m.row(0)==vec4f(col0) && "test row order Mt4x4.col(0)");
	assert(m.row(1)==vec4f(col1) && "test row order Mt4x4.col(1)");
}
template<class Real>struct Dynamic
{
	std::vector<Real> data;
	int size() const
	{
		return data.size();
	}
};
template<class Real,int N> struct Static
{
	Real data[N];
	static int size()
	{
		return N;
	}
};

template<template<class> class Base,class Real>
struct SomeObject: public Base<Real>
{
	void set(int i,Real v)
	{
		assert(i<this->size());
	}
};

template<class Real>struct Static4:public Static<Real,4>{};

void testMath()
{
	SomeObject<Static4,float> matrix;
	const float minError=0.001;
	testMatrixNM();
	testMatrixInversion();
	//testMatrix4r();
	//testMatrix4c();
	//printf("Aiming test\n");
	//
	//if(testAiming(Edge(vec3(0,0,0),vec3(0,2,0)),vec3(10,0,0),3)>minError)
	//	printf("-test failed\n");
	//if(testAiming(Edge(vec3(100,0,0),vec3(0,100,0)),vec3(0,0,0),100)>minError)
	//	printf("-test failed\n");	
	//if(testAiming(Edge(vec3(0,100,0),vec3(20,100,0)),vec3(0,0,0),30)>minError)
	//	printf("-test failed\n");
	system("pause");
}
}
int main()
{
	Test::testMath();
	return 0;
}
