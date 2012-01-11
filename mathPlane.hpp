#pragma once;

template<class Vector>
class Plane
{
public:
	typedef typename Vector::value_type value_type;
	typedef Plane<Vector> my_type;
	Vector normal;
	value_type d;

public:
	Plane()				
	{
		normal=vec3(0,0,1);
		d=0;
	}
	Plane(const my_type &plane)
		:normal(plane.normal),d(plane.d)
	{}
	Plane(Vector _n,value_type _d)
		:normal(normalise(_n)),d(_d)
	{}
	float distance(Vector v) const
	{	
		return abs(((v&normal)-d));
	}
	Vector project(Vector v) const
	{
		return v-normal*((v&normal)-d);
	}

	int classify(const Vector &v) const
	{
		register float res=(v&normal)-d;
		if(res>0)
			return prFront;
		if(res<0)
			return prBack;
		return prIntersect;
	}
	int classify(const vec3 &a, const vec3 &b) const
	{
		register float resa=(a&normal)-d;
		register float resb=(b&normal)-d;
		if(resa*resb<0)
			return prIntersect;
		if(resa<0)
			return prBack;
		return prFront;
	}
	int intersectEdge(const Vector &a, const Vector &b, Vector &result) const
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
	int intersectRay(const Vector &a, const Vector &dir, Vector &result) const
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

template<class Vector>
inline Plane<Vector> planeFromMiddlePerpendicular(const Vector a,const Vector & b)
{
	Plane<Vector> plane;
	plane.normal=normalise(b-a);	
	plane.d=(a+b)&plane.normal*0.5;
	return plane;
}

template<class Vector>
inline Plane<Vector> planeFromDirection(const Vector &a,const Vector &b)
{
	Plane<Vector> plane;
	plane.normal=normalise(b);	
	plane.d=a & plane.normal;
	return plane;
}

template<class Vector>
inline Plane<Vector> planeFromEdge(const Vector &a,const Vector &b)
{
	Plane<Vector> plane;
	plane.normal=normalise(b);	
	plane.d=a & plane.normal;
	return plane;
}