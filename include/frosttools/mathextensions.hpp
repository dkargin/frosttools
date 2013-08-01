#pragma once
#include <vector>
#include <list>
#ifndef MATH_MATRIX
#error "include <mathMatrix.hpp> first"
#endif

namespace Geom
{

template<class _Vr>
class _Edge
{
public:
	_Vr start,end;
public:
	typedef typename _Vr::value_type value_type;
	typedef _Edge<_Vr> my_type;

	_Edge()
	{}
	inline _Edge(const my_type &edge)
		:start(edge.start),end(edge.end)
	{}
	inline _Edge(const _Vr &s,const _Vr &e)
		:start(s),end(e)
	{}
	inline value_type length() const
	{
		return vecDistance(start,end);
	}
	// interpolation
	inline _Vr operator()(value_type t) const
	{
		return start+t*(end-start);
	}
	inline _Vr direction() const
	{
		return vecNormalise(end-start);
	}
	inline value_type projectLen(const _Vr &v) const
	{
		return vecProjectLen(v-start,direction());
	}
	inline _Vr project(const _Vr &v) const
	{
		return start+vecProject(v-start,direction());
	}
	inline my_type project(const my_type &edge)
	{
		return my_type(start+vecProject(edge.start-start,direction()),start+vecProject(edge.end-start,direction()));
	}
	inline bool operator ==(const my_type &edge) const
	{
		return start==edge.start && end==edge.end;
	}
	inline bool operator !=(const my_type &edge) const
	{
		return start!=edge.start || end!=edge.end;
	}
};

template<class Vector>
class _Sphere
{
public:
	typedef typename Vector::value_type value_type;
	_Sphere()
	{}
	inline _Sphere(const _Sphere<Vector> &sphere)
		:center(sphere.center),radius(sphere.radius)
	{}
	inline _Sphere(const Vector &v,value_type r)
		:center(v),radius(r)
	{}
	// return distance to sphere surface. Positive if <v> is outside, negative if <v> is inside
	inline value_type distance(const Vector &v) const
	{
		return vecDistance(v,center)-radius;
	}
public:
	Vector center;
	value_type radius;
};

template<class Vector>
class _Plane
{
public:
	typedef typename Vector::value_type value_type;
	typedef Vector vector_type;
	Vector normal;
	value_type d;
public:
	inline _Plane(const _Plane<Vector> &plane)
		:normal(plane.normal),d(plane.d)
	{}
	inline _Plane(const Vector &v,value_type D)
		:normal(v),d(D)
	{}
	_Plane()
	{}

	float distance(const vector_type &v) const
	{
		return (v&normal)-d;
	}
	vector_type project(const vector_type &v) const
	{
		return v-normal*((v&normal)-d);
	}
	vector_type projectDir(const vector_type &v) const
	{
		return v-vecProject(v,normal);
	}
	_Edge<vector_type> project(const _Edge<vector_type> &edge)
	{
		return _Edge<vector_type>(project(edge.start),project(edge.end));
	}
	int classify(const vector_type &v) const
	{
		register float res=(v&normal)-d;
		if(res>0)
			return prFront;
		if(res<0)
			return prBack;
		return prIntersect;
	}
	int classify(const vector_type &a, const vector_type &b) const
	{
		register float resa=(a&normal)-d;
		register float resb=(b&normal)-d;
		if(resa*resb<0)
			return prIntersect;
		if(resa<0)
			return prBack;
		return prFront;
	}
	int intersectEdge(const vector_type &a, const vector_type &b, vector_type &result) const
	{
		register float resa=(a&normal)-d;
		register float resb=(b&normal)-d;
		if(resa*resb<0)
		{
			float t=resa/(resa+resb);
			result=a*(1-t)+b*t;
			return prIntersect;
		}
		if(resa<0)
			return prBack;
		return prFront;
	}
	int intersectRay(const vector_type &a, const vector_type &dir, vector_type &result) const
	{
		register float resa=(a&normal)-d;
		register float resb=(dir&normal);

		/*
		1. спереди (resa>0), направление от плоскости (resb>0) - сзади
		2. сзади, (resa<0)направление от плоскости (resb<0)	- сзади
		3. спереди, resa>0, направление к плоскости, resb<0
		4. сзади, resa<0, направление к плоскости, resb>0
		*/

		int res=classify(a);
		if(resa>=0 && resb>=0)
			return prFront;

		if(resa>=0 && resb>=0)
			return prBack;
		if(fZero(resb) || (resa>0 && resb > 0) || (resa<0 && resb < 0))
			return res;
		result=a-dir*distance(a)/resb;
		return prIntersect;
	}
};
template<class Vector> _Plane<Vector> planeFromPointDir(const Vector &pt,const Vector &dir)
{
	_Plane<Vector> res;
	res.normal=dir;
	res.d=pt&dir;
	return res;
}
template<class Vector> _Plane<Vector> planeFromEdgeStart(const _Edge<Vector> &edge)
{
	_Plane<Vector> res;
	res.normal=normalise(edge.end-edge.start);
	res.d=edge.start&res.normal;
	return res;
}

template<class Vector> _Plane<Vector> planeFromEdgeEnd(const _Edge<Vector> &edge)
{
	_Plane<Vector> res;
	res.normal=normalise(edge.end-edge.start);
	res.d=edge.end&res.normal;
	return res;
}

template<class Vector> _Plane<Vector> planeFromEdgeCenter(const _Edge<Vector> &edge)
{
	_Plane<Vector> res;
	res.normal=normalise(edge.end-edge.start);
	res.d=edge(0.5)&res.normal;
	return res;
}
//////////////////////////////////////////////////////////////////////////
// Axis alligned bounding box
//////////////////////////////////////////////////////////////////////////
template<class _Vector>
class _AABB
{
public:
	typedef typename _Vector::value_type value_type;
	typedef typename _Vector::size_type size_type;
	typedef _AABB<_Vector> my_type;
	enum {D=_Vector::D};
	_Vector center;
	_Vector dimensions;
public:
	_AABB()
	{}
	template<typename Vec>
	_AABB(const _AABB<Vec> &box)
		:center(box.center),dimensions(vecAbs(box.dimensions))
	{}
	template<typename tCenter,typename tDimensions>
	_AABB(const tCenter &c,const tDimensions &d)
		:center(c),dimensions(d)
	{}
	template<typename Vec> static _AABB<Vec> minmax(const Vec &min,const Vec &max)		
	{
		return _AABB<Vec>((min+max)/2,(max - min)/2);
	}
	inline bool contains(const _Vector &v)
	{
		for(int i = 0;i < D;i++)
			if(v[i]<(center[i]-dimensions[i]) || v[i]>(center[i]+dimensions[i]))
				return false;
		return true;
	}
	// does box contain this vector.
	// sides - bound test result for every dimension. -1 if is lower, 1 if is higher, 0 if is in bounds
	inline bool contains(const _Vector &v,int sides[_Vector::D])
	{
		bool flag=true;

		for(int i=0;i<Vector::D;i++)
			if(v[i]<(center[i]-dimensions[i]))
			{
				flag=false;
				sides[i]=-1;
			}
			else if(v[i]>(center[i]+dimensions[i]))
			{
				flag=false;
				sides[i]=1;
			}
			else
				sides[i]=0;
		return flag;
	}
	inline _Vector min() const
	{
		return center-dimensions;
	}
	inline _Vector max() const
	{
		return center+dimensions;
	}
	inline value_type min(size_type i) const
	{
		return center[i]-dimensions[i];
	}
	inline value_type max(size_type i) const
	{
		return center[i]+dimensions[i];
	}
	inline _Vector size() const
	{
		return dimensions+dimensions;
	}
	inline value_type size(size_type i) const
	{
		return dimensions[i]+dimensions[i];
	}
	// get face centers
	int getCenters(_Vector *vertices) const
	{
		const typename _Vector::size_type max=Vector::N;
		for(typename _Vector::size_type i=0,j=0;i<max;i++)
		{
			_Vector offs(_Vector::value_type(0));
			offs[i]=dimensions[i];
			vertices[j++]=center+offs;
			offs[i]=-dimensions[i];
			vertices[j++]=center+offs;
		}
		return max;
	}
	// get box corners
	int getCorners(_Vector *vertices) const
	{
		const typename _Vector::size_type max=2<<(Vector::D-1);
		for(typename _Vector::size_type i=0;i<max;i++)
		{
			_Vector offs(dimensions);
			for(typename _Vector::size_type j=0;j<Vector::D;j++)
				offs[j]=(i&(1<<j))?dimensions[j]:-dimensions[j];
			vertices[i]=center+offs;
		}
		return max;
	}
	// update box size
	my_type & operator&=(const _Vector &v)
	{
		_Vector lmin=min();
		_Vector lmax=max();
		for(typename _Vector::size_type j=0;j<_Vector::D;j++)
		{
			if(lmin[j]>v[j])
				lmin[j]=v[j];
			if(lmax[j]<v[j])
				lmax[j]=v[j];
		}
		center=(lmin+lmax)/2;
		dimensions=(lmax-dimensions);
		return *this;
	}
	template<class Vec> my_type & operator&=(const _AABB<Vec> &box)
	{
		_Vector lmin=min();
		_Vector lmax=max();
		Vec bmin = box.min();
		Vec bmax = box.max();
		for(typename _Vector::size_type j=0;j<_Vector::D;j++)
		{
			if(lmin[j]>bmin[j])
				lmin[j]=bmin[j];
			if(lmax[j]<bmax[j])
				lmax[j]=bmax[j];
		}
		center=(lmin+lmax)/2;
		dimensions=(lmax-dimensions);
		return *this;
	}
};
// generate minimal AABB containg group of points
template<class _Iter> _AABB<typename _Iter::value_type> make_box(const _Iter &begin,const _Iter &end)
{
	assert(begin!=end);
	typedef _Iter::value_type Vector;		
	Vector min=*begin,max=*begin;
	for(_Iter i=begin;i!=end;i++)
	{
		for(size_type j=0;j<D;j++)
		{
			value_type val=(*i)[j];
			if(val<min[j])
				min[j]=val;
			if(val>max[j])
				max[j]=val;
		}
	}
	_AABB<Vector> res;
	res.center=(max+min)*0.5;
	res.dimensions=(max-min)*0.5;
	return res;
}
template<class Vector> _AABB<Vector> make_rect(const Vector &s,const Vector &e)
{
	return _AABB<Vector>((s+e)/2,(e-s)/2);
}
////////////////////////////////////////////////////////////////////////////////
// Edge to plane intersection
////////////////////////////////////////////////////////////////////////////////
template<class Vector>
int intersection(const _Edge<Vector> &edge,const _Plane<Vector> &plane,Vector *res_v,typename Vector::value_type *res_t)
{
	// {
	//		P=S+t*(E-S)
	//		P&N-d=0;
	//		ES=E-S
	// } => S&N+t*ES&N-d=0
	// t=(d-S&N)/S&N;
	Vector es=edge.end-edge.start;
	typename Vector::value_type f=es&plane.normal;
	if(f==Vector::value_type(0))
		return 0;
	typename Vector::value_type t=(plane.d-(edge.start&plane.normal))/f;

	if(res_t)
		*res_t=t;
	if(res_v)
		*res_v=edge(t);
	return 1;
}
////////////////////////////////////////////////////////////////////////////////
// Edge to Box intersection. 
// returns hits count
// Warning: Not explicit variant. 
////////////////////////////////////////////////////////////////////////////////
template<class _Vr>
int intersection(const _Edge<_Vr> &edge,const _AABB<_Vr> &box,_Vr *res_v=NULL,typename _Vr::value_type *res_t=NULL)
{
	typedef _Plane<_Vr> Plane;
	typedef _Edge<_Vr> Edge;
	typedef typename _Vr::value_type value_type;
	Vector<char,2*_Vr::N> hit_found(char(0));
	int res=0;
	for(typename _Vr::size_type i=0,j=0;i<_Vr::N;i++)
	{
		_Vr offs(_Vr::value_type(0));
		Plane plane;
		_Vr hit_v;
		value_type hit_t;
		offs[i]=box.dimensions[i];
		// 1. create plane from box face
		plane=planeFromEdgeEnd(Edge(box.center,box.center+offs));
		// 2. get intersection between edge and plane
		if(!hit_found[i+i] && intersection(edge,plane,&hit_v,&hit_t))
		{
			// 3. check if intersection point belongs to box
			bool flag=true;
			for(typename _Vr::size_type j=0;j<_Vr::D;j++)
			{
				if(i==j)
					continue;
				value_type min=box.min(j);
				value_type max=box.max(j);
				if(hit_v[j]<min || hit_v[j]>max)
				{
					flag=false;
					break;
				}
				if(hit_v[j]==max)
					hit_found[j+j]=1;
				if(hit_v[j]==min)
					hit_found[j+j+1]=1;
			}
			if(flag)
			{
				if(res_v)
					res_v[res]=hit_v;
				if(res_t)
					res_t[res]=hit_t;
				res++;
			}
		}
		// 1. create plane from box face
		plane=planeFromEdgeEnd(Edge(box.center,box.center-offs));
		// 2. get intersection between edge and plane
		if(!hit_found[i+i+1] && intersection(edge,plane,&hit_v,&hit_t))
		{
			// 3. check if intersection point belongs to box
			bool flag=true;
			for(typename _Vr::size_type j=0;j<_Vr::D;j++)
			{
				if(i==j)
					continue;
				value_type min=box.min(j);
				value_type max=box.max(j);
				if(hit_v[j]<min || hit_v[j]>max)
				{
					flag=false;
					break;
				}
				if(hit_v[j]==max)
					hit_found[j+j]=1;
				if(hit_v[j]==min)
					hit_found[j+j+1]=1;
			}
			if(flag)
			{
				if(res_v)
					res_v[res]=hit_v;
				if(res_t)
					res_t[res]=hit_t;
				res++;
			}
		}
	}
	return res;
}
//////////////////////////////////////////////////////////////
// Edge to edge intersection
// Returns 1 if intersection is found, 0 - otherwise
//////////////////////////////////////////////////////////////
template<class Vector> int intersection(const _Edge<Vector> &a,const _Edge<Vector> &b,Vector &res_v,typename Vector::value_type res_t[2])
{
	typedef typename Vector::value_type value_type;
	static value_type tmp_t[Vector::D];

	if(!res_t)
		res_t=tmp_t;
	/*
	{
	p=a.start+t1*(a.end-a.start)
	p=b.start+t2*(b.end-b.start)
	}
	->t1 * (a.end-a.start)+t2 * (b.start-b.end)+a.start-b.start=0
	*/
	Matrix<value_type,3,Vector::D,true> equation;
	int order[Vector::D];


	equation.col(0,a.end-a.start);
	equation.col(1,b.start-b.end);
	equation.col(2,a.start-b.start);
	equation=reduceToTriangle(equation,order);
	//equation.reduceToTriangle(order);

	if(equation(1,1)==0.0f || equation(0,0)==0.0f)
		return 0;
	// get solution from triangle matrix
	res_t[1]=-equation(2,1)/equation(1,1);
	res_t[0]=(-equation(2,0)-res_t[1]*equation(1,0))/equation(0,0);
	res_v=a(res_t[0]);

	return 1;
}
template<class Real> int intersection3(const _Edge<Vector3D<Real> > &a,const _Edge<Vector3D<Real> > &b,Vector3D<Real> *res_v,float *res_t)
{
	// 1. calculate plane
	if(fabs(a.direction()&b.direction())>0.99999f)
	{
		_Plane<Vector3D<Real> > plane=planeFromEdgeStart(a);
		intersection(b,plane,res_v+1,res_t+1);
		res_v[0]=a.start;
		res_t[0]=0;
		return 1;
	}
	else
	{
		Vector3D<Real> normal=normalise(a.direction() ^ b.direction());

		float d=a.start&normal;
		_Plane<Vector3D<Real> > plane(normal,d);
		_Edge<Vector3D<Real> > c=plane.project(b);
		vec3 side_c=a.start-c.start;
		vec3 dir[]={a.direction(),c.direction()};
		float angle[]={vecAngle(a.direction(),side_c*-1.0f),vecAngle(c.direction(),side_c)};
		float sides[2];
		float sign[2]={1,1};
		for(int i=0;i<2;i++)
			if(angle[i]>M_PI_2)
			{
				angle[i]=M_PI-angle[i];
				sign[i]*=-1;
			}
		sides[0]=side_c.length()*sinf(angle[1])/sinf(angle[0]+angle[1]);
		sides[1]=side_c.length()*sinf(angle[0])/sinf(angle[0]+angle[1]);
		res_v[0]=a.direction()*sides[0]*sign[0]+a.start;
		res_v[1]=c.direction()*sides[1]*sign[1]+c.start;

		/*int r=intersection(a,c,*res_v,res_t);*/
		float distance=plane.distance(b.start);
		res_v[1]=res_v[0]+normal*distance;
	}
	return 1;
}

///////////////////////////////////////////////////////////////
// Sphere and Shhere intersection
// Returns:
// 0 - no intersection
// 1 - touch
// 2 - intersection
// v - touch point (middle point on edge)
// radius - distance from v to intersection points
///////////////////////////////////////////////////////////////
template<class Vector> int intersection(const _Sphere<Vector> &a,const _Sphere<Vector> &b,Vector *v=NULL,typename Vector::value_type *radius=NULL)
{
	typedef typename Vector::value_type value_type;
	static value_type r=0;
	static Vector tmpVec;
	if(!radius)
		radius=&r;
	if(!v)
		v=&tmpVec;
	value_type distance=vecDistance(a.center,b.center);

	if(a.radius>b.radius && a.radius>=distance)
	{
		if(distance+b.radius<a.radius)
			return 0;
		float alpha=(-b.radius*b.radius+a.radius*a.radius-distance*distance)/(2*distance);
		*v=b.center+vecNormalise(b.center-a.center)*alpha;
		*radius=sqrt(b.radius*b.radius-alpha*alpha);
		return 1;
	}
	else if(b.radius>a.radius && b.radius>=distance)
	{
		if(distance+a.radius<b.radius)
			return 0;
		float alpha=(-a.radius*a.radius+b.radius*b.radius-distance*distance)/(2*distance);
		*v=a.center+vecNormalise(a.center-b.center)*alpha;
		*radius=sqrt(a.radius*a.radius-alpha*alpha);
		return 1;
	}
	else if(distance>a.radius+b.radius)
		return 0;
	float a1=(distance*distance+a.radius*a.radius-b.radius*b.radius)/(distance+distance);
	*radius=sqrt(a.radius*a.radius-a1*a1);
	*v=a.center+vecNormalise(b.center-a.center)*a1;
	return 1;
}
///////////////////////////////////////////////////////////////
// Edge and Sphere intersection
// Returns number of intersections
///////////////////////////////////////////////////////////////
template<class Vector>
int intersection(const _Edge<Vector> &edge,const _Sphere<Vector> &sphere,Vector * res_v=NULL,typename Vector::value_type * res_t=NULL)
{
	Vector edgeDir=edge.end-edge.start;
	Vector sphereDir=edge.start-sphere.center;
	typename Vector::value_type a(0),b(0),c(-sphere.radius*sphere.radius);
	// calculate coefficients for {a*t*t+2*b*t+c=0} equation
	for(int i=0;i<Vector::D;i++)
	{
		typename Vector::value_type dir0=edgeDir[i];
		typename Vector::value_type dir1=sphereDir[i];
		a+=(dir0*dir0);
		b+=(dir0*dir1);
		c+=(dir1*dir1);
	}
	// solve this equation
	typename Vector::value_type t;
	Vector r;
	if(a!=Vector::value_type(0))
	{
		typename Vector::value_type D=b*b-c*a;
		if(D>Vector::value_type(0))
		{
			t=(-b-sqrt(D))/a;
			if(res_t)res_t[0]=t;
			if(res_v)res_v[0]=edge(t);
			t=(-b+sqrt(D))/a;
			if(res_t)res_t[1]=t;
			if(res_v)res_v[1]=edge(t);
			return 2;
		}
		else if(D==Vector::value_type(0))
		{
			t=-b/a;
			if(res_t)res_t[0]=t;
			if(res_v)res_v[0]=edge(t);
			return 1;
		}
	}
	else
	{
		t=sphere.radius/edgeDir.length();
		if(res_t)
		{
			res_t[0]=t;
			res_t[1]=-t;
		}
		if(res_v)
		{
			res_v[0]=edge(t);
			res_v[1]=edge(-t);
		}
	}
	return 0;
}
template<typename _Vector> float getPolarAngle(const _Vector &a)
{
	return atan2(a[1],a[0]);
}
template<typename _Vector> int compareVec2d_polar(const _Vector &a,const _Vector &b,const _Vector &c=_Vector(0.0f))
{
	_Vector va=a-c;
	_Vector vb=b-c;
	float angle_a=getPolarAngle(va);
	float angle_b=getPolarAngle(vb);
	if(angle_a<angle_b)
		return -1;
	if(angle_a>angle_b)
		return 1;
	float len_a=va.length();
	float len_b=vb.length();
	if(len_a<len_b)
		return-1;
	if(len_a>len_b)
		return 1;
	return 0;
}

enum EdgeRelation{
	cLEFT,
	cRIGHT,
	cBEHIND,
	cBEYOND,
	cORIGIN,
	cDESTINATION,
	cBETWEEN
};
/// determine where v lies relative to specified edge
template<class _Vector> EdgeRelation classify(const _Edge<_Vector> &edge,const _Vector &v)
{
	_Vector a = edge.end - edge.start;
	_Vector b = v - edge.start;
	float s = a[0]*b[1] - a[1]*b[0];
	if(s>0.0f)
		return cLEFT;
	if(s<0.0f)
		return cRIGHT;
	if(a[0]*b[0] < 0.0 || a[1]*b[1] < 0.0)
		return cBEHIND;
	if(a.length() < b.length())
		return cBEYOND;
	if(v==edge.start)
		return cORIGIN;
	if(v==edge.end)
		return cDESTINATION;
	return cBETWEEN;
}
/// determine where _Vector::zero() lies relative to specified edge
template<class _Vector> EdgeRelation classifyZero(const _Edge<_Vector> &edge)
{
	_Vector a = edge.end - edge.start;
	const _Vector &b = edge.start;
	float s = a[1]*b[0] - a[0]*b[1];
	if(s > 0.0f)
		return cLEFT;
	if(s < 0.0f)
		return cRIGHT;
	if(a[0]*b[0] > 0.0 || a[1]*b[1] > 0.0)
		return cBEHIND;
	if(a.length() < b.length())
		return cBEYOND;
	if(edge.start.isZero())
		return cORIGIN;
	if(edge.end.isZero())
		return cDESTINATION;
	return cBETWEEN;
}


template<class Source> void wrapHull(const Source &source,std::vector<vec2f> &target)
{
	std::vector<vec2f> tmpSource(source.begin(),source.end());
	int size=tmpSource.size();
	std::list<vec2f> tempTarget;
	int a=0;
	// 1. find leftmost point
	for(int i=1;i<size;i++)
	{
		if(tmpSource[i][0] < tmpSource[a][0])
			a=i;
	}
	tmpSource.push_back(tmpSource[a]);
	for(int i=0;i<size;i++)
	{
		vec2f b=tmpSource[a];tmpSource[a]=tmpSource[i];tmpSource[i]=b;

		tempTarget.push_back(tmpSource[i]);
		a=i+1;
		for(int j=i+2;j<=size;j++)
		{
			int c=classify(_Edge<vec2f>(tmpSource[i],tmpSource[a]),tmpSource[j]);
			if(c==cLEFT ||c==cBEYOND)
				a=j;
		}
		if(a==size)
			break;
	}
	target.assign(tempTarget.begin(),tempTarget.end());
}
//class Angle
//{
//public:
//	void fix()
//	{
//		if(angle>M_2PI)
//			angle-=M_2PI;
//		if(angle<0)
//			angle+=M_2PI;
//	}
//
//	inline Angle& operator += ( const Angle& v )
//	{
//		angle+=v.angle;
//		return *this;
//	}
//
//	inline Angle& operator -= ( const Angle& v )
//	{
//		angle-=angle;
//		return *this;
//	}
//
//	/*inline Angle& operator *= ( float f )
//	{
//		for(size_type i=0;i<_D;i++)
//			c[i]*=f;
//		return *this;
//	}
//
//	inline Angle& operator /= ( const Angle& v )
//	{
//		angle/=v.angle;
//		return *this;
//	}
//	inline Angle& operator /= ( float f )
//	{
//		angle/=angle;
//		return *this;
//	}*/
//
//	inline int operator == ( const Angle & v ) const
//	{
//		return angle==v.angle;
//	}
//	inline int operator != ( const Angle& v ) const
//	{
//		return angle!=v.angle;
//	}
//public:
//	float angle;
//};
typedef _Sphere<vec3> Sphere;
typedef _Edge<vec3> Edge;
typedef _Edge<vec3> Ray;
typedef _AABB<vec3> AABB;
typedef _Plane<vec3> Plane;
//////////////////////////////////////////////////////////////////
template<class _Vr> struct _Traectory
{
	typedef _Vr vec;
	_Vr start;
	_Vr velocity;	
	
	_Traectory(const _Traectory &tr)
	:start(tr.start),velocity(tr.velocity)
	{}
	_Traectory(const _Vr &pos,const _Vr &vel=_Vr(0.f))
		:start(pos),velocity(vel)
	{}
	_Vr getPosition(float time) const
	{
		return start+velocity*time;
	}
	_Traectory advance(float time) const
	{
		return _Traectory(getPosition(time),velocity);
	}
	const _Vr operator ()(float time) const
	{
		return advance(time).start;
	}
};

template<class _V> float getMinDistance(const _Traectory<_V> &tr0,const _Traectory<_V> &tr1,float maxTime,float &time)
{
	float res=0;	
	_V u=tr0.start-tr1.start;
	_V v=tr0.velocity-tr1.velocity;
	double c=u&u;
	double b=u&v;
	double a=v&v;
	if(a)
	{
		time=-b/a;
		if(time>=0 &&  time<=maxTime)		
		{
			res=sqrt(c-b*b/a);
		}
		else
		{
			float de=a*maxTime*maxTime+b*maxTime+b*maxTime+c;
			if(de<c)
			{
				time=maxTime;
				res=sqrt(de);
			}
			else
			{
				time=0;
				res=sqrt(c);
			}
		}
	}
	else // velocities are equal
	{
		res=sqrt(c);
		time=0;
	}
	return res;
}
typedef _Traectory<vec2f> Traectory2;
}