#ifndef _MATH_VECTOR_HPP_
#define _MATH_VECTOR_HPP_

#include "mathBase.hpp"

namespace frosttools
{

template <typename Real,int _D>
class Vector
{
public:
	typedef Real value_type;				///< defines scalar value type
	typedef unsigned int size_type;		///< defines index type
	static const size_type D = _D;			///< dimension size
	typedef Vector<Real,_D> vector_type;	///< defines vector type

	value_type c[_D];						///< actual data
public:
	/// Get zero vector
	inline static Vector zero()
	{
		Vector result;
		for(size_type i = 0; i < _D; i++)
			result[i] = 0;
		return result;
	}

	/// Get orth vector for specified direction
	inline static Vector orth(size_type dir,const Real &def=1)
	{
		Vector result;
		for(size_type i = 0; i < _D; i++)
			i==dir ? result[i] = def:result[i]=0;
		return result;
	}

	inline Vector()	{}

	/// Constructor
	inline explicit Vector(Real p)
	{
		for(size_type i = 0; i < _D; i++)
			c[i] = p;
	}

	/// Constructor
	template<typename tReal> inline Vector(const Vector<tReal,_D>& vec)
	{
		for(size_type i = 0; i < _D; i++)
			c[i] = vec.c[i];
	}

	/// Constructor
	template<typename tReal> explicit inline Vector(const tReal *v)
	{
		for(size_type i = 0; i < _D; i++)
			c[i] = v[i];
	}
	/// check if vector is zero length
	inline bool isZero() const
	{
		for(size_type i = 0; i < _D; ++i)
			if( c[i] != value_type(0) )
				return false;
		return true;
	} 
	/// pointer conversion
	inline operator value_type * ()
	{
		return ((Real*)this);
	}
	/// const pointer conversion
	inline operator const value_type * () const
	{
		return ((Real*)this);
	}
	/// assignment operator
	inline Vector/*<Real,_D>*/ & operator = ( const Vector/*<Real,_D>*/& v )
	{
		for(size_type i = 0;i < _D; i++)
			c[i] = v.c[i];
		return *this;
	}

	/// addition operator
	inline Vector<Real,_D> operator + () const
	{
		return *this;
	}

	/// substraction operator
	inline Vector<Real,_D> operator - () const
	{
		Vector res;
		for(size_type i = 0; i < _D; i++)
			res[i] = -c[i];
		return res;
	}
	/// get squared length
	inline Real length_squared () const
	{
		return (*this) & (*this);
	}
	/// c0 + c1 + c2 + ...
	inline Real sum()const
	{
		Real result = c[0];
		for(size_type i = 1; i < _D; i++)
			result += c[i];
		return result;
	}
	/// calculate vector length
	inline Real	length () const
	{
		return (Real) sqrtf ( (*this) & (*this) );
	}

	/// normalize vector
	Vector<Real,_D>&	normalize ()
	{
		return (*this) /= length ();
	}

	/// addition
	inline Vector& operator += ( const Vector& v )
	{
		for(size_type i = 0; i < _D; i++)
			c[i] += v[i];
		return *this;
	}

	/// substraction
	inline Vector& operator -= ( const Vector& v )
	{
		for(size_type i = 0; i < _D; i++)
			c[i] -= v[i];
		return *this;
	}

	/// strange one?
	inline Vector& operator *= ( const Vector& v )
	{
		for(size_type i = 0;i < _D; i++)
			c[i] *= v[i];
		return *this;
	}

	/// multiplication
	inline Vector& operator *= ( float f )
	{
		for(size_type i = 0; i < _D; i++)
			c[i] *= f;
		return *this;
	}

	/// another strange one
	inline Vector& operator /= ( const Vector& v )
	{
		for(size_type i = 0; i < _D; i++)
			c[i] /= v[i];
		return *this;
	}

	/// scalar division operator
	inline Vector<Real,_D>& operator /= ( Real f )
	{
		for(size_type i = 0; i < _D; i++)
			c[i] /= f;

		return *this;
	}

	/// check if vectors are equal
	inline int operator == ( const Vector<Real,_D>& v ) const
	{
		for(size_type i = 0; i < _D; i++)
			if(!(this->c[i] == v.c[i]))
				return false;
		return true;
	}

	/// Check if vectors are not equal
	inline int operator != ( const Vector<Real,_D>& v ) const
	{
		for(size_type i = 0; i < _D;i++)
			if(this->c[i] == v[i])
				return false;
		return true;
	}

	/// return dimensions number
	inline int size() const
	{
		return _D;
	}
};

template <class _Vr>
inline typename _Vr::vector_type operator+(const _Vr& v1,const typename _Vr::vector_type& v2)
{
    typedef typename _Vr::size_type size_type;
	_Vr res(v1);
	for(size_type i = 0; i < _Vr::D; i++)
		res[i] += v2[i];
	return res;
}

template <typename _Vr>
inline _Vr operator-(const _Vr& v1,const typename _Vr::vector_type& v2)
{
	_Vr res(v1);
	for(typename _Vr::size_type i = 0; i < _Vr::D; i++)
		res[i] -= v2[i];
	return res;
}

template <typename _Vr>
inline _Vr operator*(const _Vr& v,const typename _Vr::value_type &r)
{
	_Vr res(v);
	for(typename _Vr::size_type i = 0;i < _Vr::D; i++)
		res[i] *= r;
	return res;
}

template <typename _Vr>
inline _Vr operator* (const typename _Vr::value_type &r,const _Vr& v)
{
	_Vr res(v);
	for(typename _Vr::size_type i = 0; i < _Vr::D; i++)
		res[i] *= r;
	return res;
}


template <typename _Vr>
inline _Vr operator/(const _Vr& v,const typename _Vr::value_type &r)
{
	_Vr res(v);
	for(typename _Vr::size_type i = 0 ; i < _Vr::D; i++)
		res[i] /= r;
	return res;
}

template <typename _Vr>
inline typename _Vr::value_type operator&(const _Vr& v1,const typename _Vr::vector_type& v2)
{
	typename _Vr::value_type res(0);
	for(typename _Vr::size_type i = 0;i < _Vr::D; i++)
		res += v1[i] * v2[i];
	return res;
}

template <typename _Vr>
inline _Vr vecAbs(const _Vr &v)
{
	_Vr res(v);
	for(typename _Vr::size_type i = 0 ; i < _Vr::D; i++)
		if(res[i] < 0)
			res[i] = -res[i];
	return res;
}

/// Vector Specialization for 2D
template <class Real>
class	Vector2D: public Vector<Real,2>
{
public:
    typedef Vector<Real,2> parent_type;
	typedef typename parent_type::value_type value_type;
	typedef typename parent_type::size_type size_type;
	static const size_type size = 2;
	typedef Vector2D<Real> vector_type;
	inline Vector2D()
	{}

	inline Vector2D ( Real px, Real py)
	{
		parent_type::c[0] = px;
		parent_type::c[1] = py;
	}
	template<class tReal>
	inline Vector2D(const tReal *p)
		:parent_type(p)
	{}
	explicit inline Vector2D(Real p)
		:parent_type(p)
	{}
	template<class tReal>
	inline Vector2D(const Vector<tReal,2>&v)
		:parent_type(v)
	{}
	//rotates vector around Z-axis
	const vector_type& rotate(float angle)
	{
	    float *p = (float*)this;
		float cs = cosf(angle);
		float sn = sinf(angle);

		float x1 = p[0] * cs - p[1] * sn;
		float y1 = p[0] * sn + p[1] * cs;

		p[0] = x1;
		p[1] = y1;
		return *this;
	}

	float angle() const
	{
		return atan2f(this->c[1], this->c[0]);
	}
};

/// Vector Specialization for 3D
template <class Real>
class	Vector3D: public Vector<Real,3>
{
public:
    typedef Vector<Real,3> parent_type;
	typedef Vector3D<Real> vector_type;

	typedef typename parent_type::value_type value_type;
	typedef typename parent_type::size_type size_type;
	static const size_type size = 3;

	Vector3D ():parent_type() {/*c[0]=c[1]=c[2]=0;*/}
#ifdef lua_h
	inline Vector3D ( lua_State *L ,int idx)
	{
		lua_pushinteger(L,1);
		lua_gettable(L,idx);
		c[0] = (value_type)lua_tonumber(L,-1);
		lua_pushinteger(L,2);
		lua_gettable(L,idx);
		c[1] = (value_type)lua_tonumber(L,-1);
		lua_pushinteger(L,3);
		lua_gettable(L,idx);
		c[2] = (value_type)lua_tonumber(L,-1);
		lua_pop(L,3);
	}
#endif

	inline Vector3D ( Real px, Real py, Real pz )
	{
		this->c[0] = px;
		this->c[1] = py;
		this->c[2] = pz;
	}
	template<class tReal>
	inline Vector3D(const tReal *p):parent_type(p){}
	explicit inline Vector3D(Real p):parent_type(p){}

	template<class tReal>inline Vector3D(const Vector<tReal,3> &v):parent_type(v){}

    float   maxLength () const
    {
        const float *p=(float*)this;
        return max3 ( fabs (p[0]), fabs (p[1]), fabs (p[2]) );
    }
	void set(Real x,Real y,Real z)
	{
        float *p=(float*)this;
		p[0] = x;
		p[1] = y;
		p[2] = z;
	}

	vector_type& dir(float angle)
	{
	    float *p = (float*)this;
		p[0] = cosf(angle);
		p[1] = sinf(angle);
		p[2] = Real(0);
		return *this;
	}
	//rotates vector around Z-axis
	const vector_type& rotateZ(float angle)
	{
	    float *p = (float*)this;
		float cs = cosf(angle);
		float sn = sinf(angle);

		float x1 = p[0] * cs - p[1] * sn;
		float y1 = p[0] * sn + p[1] * cs;

		p[0] = x1;
		p[1] = y1;
		return *this;
	}
	//rotates vector around Z-axis by 90 degrees
	const vector_type& rotatePi2()
	{
		float *p = (float*)this;

		float x1 = - p[1];
		float y1 = p[0];

		p[0] = x1;
		p[1] = y1;
		return *this;
	}

	const vector_type& rotatePi2n()
	{
		float *p = (float*)this;

		float x1 = p[1];
		float y1 =-p[0];

		p[0] = x1;
		p[1] = y1;
		return *this;
	}

	void rotateY(float angle)
	{
	    float *p = (float*)this;
		float cs = cosf(angle);
		float sn = sinf(angle);

		float x1 = p[0] * cs - p[2] * sn;
		float z1 = p[0] * sn + p[2] * cs;

		p[0] = x1;
		p[2] = z1;
	}
	// rotate around given vector
	void rotateV(float angle,const Vector3D &V)
	{
		vector_type z = V*(*this&V);
		vector_type x = *this-z;
		vector_type y = x^V;
		*this = cosf(angle)*x + sinf(angle)*y + z;
	}
	// rotate a vector in the (X,Y) coordinate plane. (non-orthogonal X,Y possible)
	void rotateV(float angle,const Vector3D &X,const Vector3D &Y)
	{
		float cs=cosf(angle);
		float sn=sinf(angle);

		float x0 = (X&(*this));//(X,*this);
		float y0 = (Y&(*this));//project_vec(Y,*this);

		float x1 = x0 * cs - y0 * sn;
		float y1 = x0 * sn + y0 * cs;
		*this = x1*X + y1*Y;
	}

	float angleTo(const Vector3D &vec)
	{
		float val = (*this)&vec/vec.length()/vec.length();
		return val;
	}
private:
    Real   max3 ( Real a, Real b, Real c ) const
    {
        return a > b ? (a > c ? a : (b > c ? b : c)) :
                       (b > c ? b : (a > c ? a : c));
    }
};

template <typename Real>
inline Vector<Real,3> crossProduct ( const Vector<Real,3>& u, const Vector<Real,3>& v )
{
	return Vector3D<Real> (u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0]);
}

/// Vector Specialization for 4D
template <class Real>
class	Vector4D: public Vector<Real,4>
{
public:
    typedef Vector<Real,4> parent_type;
	typedef typename parent_type::value_type value_type;
	typedef typename parent_type::size_type size_type;
	static const size_type size=4;
	typedef Vector4D<Real> vector_type;

	inline Vector4D()
	{}
	inline Vector4D ( Real px, Real py, Real pz, Real pw)
	{
	    float *p=(float*)this;
		p[0] = px;
		p[1] = py;
		p[2] = pz;
		p[3] = pw;
	}
	inline Vector4D(const Real *p)
		:parent_type(p)
	{}
	explicit inline Vector4D(Real p)
		:parent_type(p)
	{}
	inline Vector4D(const Vector3D<Real>&v,Real w)
	{
        float *p=(float*)this;
		p[0] = v[0];
		p[1] = v[1];
		p[2] = v[2];
		p[3] = w;
	}

	inline Vector4D ( const Vector4D<Real>& v )
		:parent_type(v)
	{}
	inline Vector4D(const Vector<Real,4>&v)
		:parent_type(v)
	{}

	operator Vector<Real,4> ()
	{
		return *this;
	}
	inline Vector4D<Real> operator = (parent_type &v)
	{
        float *p = (float*)this;
		p[0] = v[0];
		p[1] = v[1];
		p[2] = v[2];
		p[3] = v[3];
		return *this;
	}
};

typedef Vector4D<int> vec4i;
typedef const vec4i & crvec4i;
typedef Vector4D<float> vec4f;
typedef const vec4f & crvec4f;

typedef Vector3D<float> vec3f;
typedef const vec3f & crvec3f;
typedef vec3f vec3;				// Most polular type. Just for faster typing.
typedef crvec3f crvec3;
typedef Vector3D<signed int> vec3i;
typedef const vec3i & crvec3i;
typedef Vector3D<unsigned char> vec3ub;
typedef const vec3ub & crvec3ub;

typedef Vector2D<short signed int> vec2i;
typedef const vec2i & crvec2i;
typedef Vector2D<float> vec2f;
typedef const vec2f & crvec2f;

template <typename _Vr> inline _Vr vecNormalize(const _Vr &v)
{
	_Vr res = v;
	return res.normalize();
}

template <typename _Vr> inline _Vr vecNormalize_s(const _Vr &v)
{
	typename _Vr::value_type len = v.length();
	return len>0.f?v/len:_Vr::zero();
}

template <class _Vr>
inline typename _Vr::value_type vecProjectLen(const _Vr &a, const _Vr &b)
{
	return (a&b)/b.length();
}

template <class _Vr>
inline _Vr vecProject(const _Vr &a,const _Vr &b)
{
	return (b*(a&b))/(b&b);
}

template <class _Vr> inline typename _Vr::value_type vecSqrDistance(const _Vr &a, const _Vr &b)
{
	typename _Vr::value_type result = 0.f;
	for(typename _Vr::size_type i = 0 ; i < _Vr::D; i++)
	{
		typename _Vr::value_type delta = a[i] - b[i];
		result+= (delta*delta);
	}
	return result;
}

template <class _Vr> inline typename _Vr::value_type vecDistance(const _Vr &a, const _Vr &b)
{
	typename _Vr::value_type result = 0.f;
	for(typename _Vr::size_type i = 0 ; i < _Vr::D; i++)
	{
		typename _Vr::value_type delta = a[i] - b[i];
		result+= (delta*delta);
	}
	return sqrt(result);
}

template <class _Vr> inline typename _Vr::value_type vecDotProduct(const _Vr &a, const _Vr &b)
{	
	return a&b;
}

template <class _Vr> inline typename _Vr::value_type vecLength(_Vr v)
{
	return sqrtf(v & v);
}

template <class _Vr> inline float vecAngle2d(const _Vr &a, const _Vr &b)
{
	_Vr ta = vecNormalise(a);
	_Vr tb = vecNormalise(b);
	float sn = ta[0]*tb[1] - ta[1]*tb[0];
	float cs = (a&b) > 0.0f;

	if(fZero(cs)) // either 90, or -90
		return sn> 0 ? M_PI_2 : -M_PI_2;

	return cs>0? asin(sn):M_PI-asin(sn);
}

template <class _Vr> inline float vecAngle2d_CCW(const _Vr &a, const _Vr &b)
{
	_Vr ta = vecNormalise(a);
	_Vr tb = vecNormalise(b);
	float cs = ta&tb;
	float sn = ta[0]*tb[1] - ta[1]*tb[0];
	// sn<0 if CW order
	float res = acos(cs);
	if(sn<0.0f)
		return M_2PI - res;
	return res;
}

template <class _Vr> inline float vecAngle2d_CW(const _Vr &a, const _Vr &b)
{
	_Vr ta = vecNormalise(a);
	_Vr tb = vecNormalise(b);
	float cs = ta&tb;
	float sn = ta[0]*tb[1] - ta[1]*tb[0];
	// sn<0 if CW order
	float res=acos(cs);
	if(sn>0.0f)
		return M_2PI-res;
	return res;
}

template <class _Vr> inline float vecAngle2d_positive(const _Vr &a, const _Vr &b)
{
	_Vr ta = vecNormalise(a);
	_Vr tb = vecNormalise(b);
	float sn = ta[0]*tb[1] - ta[1]*tb[0];
	float cs = (a&b);

	if(fZero(cs)) // either 90, or -90
		return sn>0?M_PI_2:-M_PI_2;
	float res = cs>0.0f? asin(sn):M_PI-asin(sn);
	if(res < 0)
		res = res+M_2PI;
	return res;
}

template <class _Vr> inline float vecAngle(const _Vr &a, const _Vr &b)
{
	assert(a.length()>0.0f);
	assert(b.length()>0.0f);
	float f = (a&b)/(vecLength(a)*vecLength(b));
	if(f > 1.0f)
		f = 1.0f;
	return acos(f);
}

template<class _Vr>
bool collinear(const _Vr &a,const _Vr &b)
{
	float f = (a&b)/(vecLength(a)*vecLength(b));
	return fabs(f)>0.9999f;
}
template <class _Vr> inline float vecAngle(const _Vr &a, const _Vr &b, const _Vr &up)
{
	vec3 va = a - vecProject(a,up);
	vec3 vb = b - vecProject(b,up);
	vec3 left = crossProduct(up , va);

	float y = vecAngle(va,vb);
	if((left&vb) >= 0.0f)
		return y;
	else
		return -y;
}

template <class _Vr> inline float vecAngle_n(const _Vr &a, const _Vr &b, const _Vr &up)
{
	static vec3 c;
	static float y;
	c = (a^b);
	y = acosf(a&b);
	float u = c&up;
	if((c&up)>= 0)
		return y;
	else
		return 2*M_PI - y;
}

template<class _Vr> signed int pointInEdge(const _Vr &start, const _Vr &end, const _Vr &point)
{
	for(typename _Vr::size_type i = 0; i < _Vr::D; i++)
	{
		//float mi=min(start[i],end[i]);

		if(point[i] < start[i] && start[i] < end[i])
			return -1;
		if(point[i] > start[i] && start[i] > end[i])
			return -1;
		if(point[i] > end[i] && start[i] < end[i])
			return 1;
		if(point[i] < end[i] && start[i] > end[i])
			return 1;
	}
	return 0;
}

template <> inline float vecDistance<vec2f>(const vec2f &a, const vec2f &b)
{	
	return sqrt((a[0]-b[0])*(a[0]-b[0]) + (a[1] - b[1])*(a[1] - b[1]));
}

template <> inline float vecSqrDistance<vec2f>(const vec2f &a, const vec2f &b)
{	
	return ((a[0]-b[0])*(a[0]-b[0]) + (a[1] - b[1])*(a[1] - b[1]));
}

template <> inline float vecDotProduct<vec2f>(const vec2f &a, const vec2f &b)
{	
	return a[0]*b[0] + a[1]*b[1];
}

template <> inline float vecDistance<vec3>(const vec3 &a, const vec3 &b)
{	
	return sqrt((a[0]-b[0])*(a[0]-b[0]) + (a[1] - b[1])*(a[1] - b[1])+(a[2] - b[2])*(a[2] - b[2]));
}

template <> inline float vecSqrDistance<vec3>(const vec3 &a, const vec3 &b)
{	
	return (a[0]-b[0])*(a[0]-b[0]) + (a[1] - b[1])*(a[1] - b[1])+(a[2] - b[2])*(a[2] - b[2]);
}


template <> inline float vecDotProduct<vec3>(const vec3 &a, const vec3 &b)
{	
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

}

#endif
