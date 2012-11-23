#pragma once
#define FrostTools_Use_3DMath
#ifndef DEF_3DMATH_H
#define DEF_3DMATH_H
//#if defined WINVER && !defined NOMINMAX && defined min
//#error "define #NOMINMAX before #include \"windows.h\""
//#endif
/////////////////////////////////////////////////////////////////////
// I'm becoming template<typename maniac>
/////////////////////////////////////////////////////////////////////

#define X_AXIS	0
#define Y_AXIS	1
#define Z_AXIS	2
#define ORIGIN	3

#define _USE_MATH_DEFINES
#include	<math.h>
#include	<assert.h>

#pragma warning(disable:4244)
#pragma warning(disable:4305)
const float PI_180=57.295779513082320876798154814105f;
const float M_2PI=(float)2.0f*M_PI;

inline float DEG2RAD(float a)
{
	return a/PI_180;
}
inline float RAD2DEG(float a)
{
	return a*PI_180;
}
inline float clampf(float val,float min,float max)
{
	if(val<min)
		val=min;
	if(val>max)
		val=max;
	return val;
}

inline float ClampAngle(float angle)
{
	while(angle>180.0f)
		angle-=360.0;
	while(angle<-180.0f)
		angle+=360.0;
	return angle;
}

template<class Real> inline Real clamp(const Real &val,const Real &min,const Real &max)
{
	if(val<min)
		return min;
	else if(val>max)
		return max;
	return val;
}
template<class Real> inline Real deadZone(const Real &val,const Real &min,const Real &max)
{
	if(val>min && val<max)
		return 0;	
	return val;
}
inline int fZero(float f)
{
	const float low=0.000005f;
	if(f<low && f>-low)
		return true;
	return false;
}
inline float fSign(float f)
{
	if(fZero(f))
		return 0.0f;
	return f>0.0f?1.0f:-1.0f;
}

struct Clamp
{
  typedef float value_type;
  typedef Clamp my_type;
  value_type min,max;
  Clamp(const value_type &min,const value_type &max)
    :min(min),max(max)
  {}
  Clamp(const my_type & clamp)
    :min(clamp.min),max(clamp.max)
  {}
  value_type operator*(const value_type & val) const
  {
    if(val > max)
      return max;
    if(val < min)
      return min;
    return val;
  }
  my_type & operator = (const my_type & val)
  {
    min = val.min;
    max = val.max;
    return *this;
  }
};

inline float operator *= (float val,const Clamp & clamp)
{
  return clamp * val;
}

enum spaceRelation
{
	srIntersect,
	srNoIntersect,
	srParallel,
	srCross,
	srEqual
};
enum planeRelation
{
	prBack,
	prFront,
	prIntersect
};

template<class Real> struct MathTypes
{
	typedef int size_type;
};
#include "mathVector.hpp"
template<class Type,int X,int Y> struct StorageStatic:public Vector<Type,X*Y>
{
	static const int size_x=X;
	static const int size_y=Y;

	typedef Vector<Type,X> row_type;
	typedef Vector<Type,Y> col_type;

	inline static row_type make_row()	// constructs row type 
	{
		return row_type();
	}
	inline static col_type make_col()	// construct columt type
	{
		return col_type();
	}

	inline static int size()
	{
		return size_x*size_y;
	}
	inline static int cols()
	{
		return size_x;
	}
	inline static int rows()
	{
		return size_y;
	}
	Type * ptr()
	{
		return (Type*)this;
	}
	const Type *ptr() const
	{
		return (const Type*)this;
	}
protected:
	template<class R>void assign(const R *data)
	{
		for(int i=0;i<size();i++)
			c[i]=data[i];
	}
};

template<class Type> struct VectorDynamic
{
public:
	typedef VectorDynamic<Type> my_type;
	typedef Type value_type;
	VectorDynamic(const VectorDynamic &v)
		:c(NULL),count(0)
	{
		assign(v.c,v.count);
	}
	VectorDynamic(int N)
		:c(NULL),count(0)
	{
		resize(N);
	}
	template<class R> VectorDynamic(const R *data,int N)
		:c(NULL),count(0)
	{
		assign(data,N);
	}
	template<class R,int N> VectorDynamic(const Vector<R,N> &vec)
		:c(NULL),count(0)
	{
		assign((const R*)vec,N);
	}
	~VectorDynamic()
	{
		reset();
	}
	inline int size() const
	{
		return count;
	}
	inline Type *ptr()
	{
		return c;
	}
	inline operator Type*()
	{
		return c;
	}
	inline operator const Type *() const
	{
		return c;
	}
	void resize(int N)
	{
		if(count==N)
			return;
		if(c)reset();
		c=new Type[N];
		count=N;
	}
	//template<class Vec>	my_type & operator = (const Vec &t)
	//{
	//	typename Vec::value_type V;
	//	assign((V*)t,t.size());
	//}
	my_type & operator = (const my_type &t)
	{
		assign((const value_type*)t,t.size());
		return *this;
	}
protected:
	
	template<class R> void assign(const R *data,int num)
	{
		resize(num);
		// we can not use memcpy here
		if(data)
			for(int i=0;i<count;i++)
				c[i]=data[i];
	}
	void reset()
	{
		if(c)
		{
			delete []c;
			c=NULL;
			count=0;
		}
	}
	Type *c;
	int count;
};
template<class Type> struct StorageDynamic
{
	typedef Type value_type;
	Type *c;
	int size_x;
	int size_y;
	StorageDynamic()
		:c(NULL),size_x(0),size_y(0)
	{}
	~StorageDynamic()
	{
		reset();
	}	
	int size()const
	{
		return size_x*size_y;
	}
	int cols()const
	{
		return size_x;
	}
	int rows()const
	{
		return size_y;
	}
	Type * ptr()
	{
		return c;
	}
	const Type *ptr() const
	{
		return c;
	}
	typedef VectorDynamic<Type> row_type;
	typedef VectorDynamic<Type> col_type;

	row_type make_row()	const // constructs row type 
	{
		return row_type(cols());
	}
	col_type make_col()	const // construct columt type
	{
		return col_type(rows());
	}
protected:
	template<class R> void assign(const R *data,int w,int h)
	{	
		resize(w,h);
		if(data)
			for(int i=0;i<w*h;i++)c[i]=data[i];
	}
	void resize(int w,int h)
	{
		if(w==size_x && h==size_y)
			return;
		if(c)reset();
		if(!w*h)	// no zero-sized matrices
			return;

		c=new value_type[w*h];		
		size_x=w;
		size_y=h;
	}
	void assign_val(const value_type &val,int w,int h)
	{
		resize(w,h);
		for(int i=0;i<w*h;i++)c[i]=val;
	}
	void reset()
	{
		if(c)
		{
			delete []c;
			c=NULL;
		}
		size_x=0;
		size_y=0;
	}
};

#include "mathMatrix.hpp"
#include "mathMatrix3.hpp"
#include "mathMatrix4.hpp"
//#include "mathMatrixNM.hpp"
//#include "mathSolver.hpp"
//#include "mathQuaternion.hpp"
#include "mathExtensions.hpp"
template<typename Real,bool order>
Vector3D<Real> transform3 ( const Matrix4<Real,order>& m, const Vector3D<Real>& v )
{	
	return Vector3D<Real>((Real*)Vector<Real,4>(m.col(0)*v[0]+m.col(1)*v[1]+m.col(2)*v[2]));
}

template<typename Real,bool order>
Vector3D<Real> transform( const Matrix4<Real,order>& m, const Vector3D<Real>& v )
{
	return Vector3D<Real>((Real*)Vector<Real,4>(m.col(0)*v[0]+m.col(1)*v[1]+m.col(2)*v[2]+m.col(3)));
}
template<class Real>
Vector3D<Real> getVec3(const Vector<Real,4> &v)
{
	return vec3(v);
}
template<class Real>
Vector3D<Real> inverseRotateVect( const Matrix4<Real,true>& m, const Vector3D<Real>& v  )
{
	//   row|   col
	// 0 1 2| 0 3 6
	// 3 4 5| 1 4 7
	// 6 7 8| 2 5 8
	vec3 res;
	//float vec[3];
	//column order
	res=getVec3(res[0]*m.row(0)+res[1]*m.row(1)+res[2]*m.row(2));
	//res[0] = pVect[0]*m[0]+pVect[1]*m[1]+pVect[2]*m[2];
	//res[1] = pVect[0]*m[4]+pVect[1]*m[5]+pVect[2]*m[6];
	//res[2] = pVect[0]*m[8]+pVect[1]*m[9]+pVect[2]*m[10];
	//memcpy( pVect, vec, sizeof( float )*3 );
	return res;
}
//template <class Real> class Matrix4;
template<class R>
inline bool valueInRange(const R &a,const R &c,const R &b)// tests if value (b) lies between (a) and (b)
{
	if(a<c)
	return (b>=a) && (b<=c);
	if(a>=c)
	return (b>=c) && (b<=a);
}

inline float sideXY(const vec3 &a,const vec3 &b)
{
	return a[0]*b[1]-b[0]*a[1];
}
inline bool testvecXY(const vec3 &a,const vec3 &b,const vec3 &c)
{
	return (sideXY(a,c)>0) ^ (sideXY(b,c)>0);
}

namespace Math
{
	const bool RowOrder=true;
};

template<class Type> const Vector2D<Type> & vec3to2(const Vector3D<Type> & v)
{
	return (Vector2D<Type>&)v;
}

typedef Matrix3<float,Math::RowOrder> Mt3x3;
typedef Matrix4<float,Math::RowOrder> Mt4x4;
//typedef Matrix4<float,Math::RowOrder> COORDSYS;
/// Handles local coordinate system, in form of position+rotation+z
class Pose2z
{
public:
	typedef vec3f pos3;		/// describes "position" type
	typedef vec2f pos;		/// describes "flat position" type
	typedef vec3f vec3;		/// describes "vector" type
	typedef vec2f vec;		/// "flat" vector
	typedef vec dir;		/// describes "direction" type. Actually is syninonim to "vector"
	typedef float rot;		/// describes "rotation" type.
	typedef Mt3x3 mat;		/// describes "matrix" type

	pos3 position;			/// object global position
	rot orientation;		/// object global orientation. For 2d pose it's scalar angle(rad). For 3d - quaternion
	typedef Pose2z pose_type;

	pose_type():position(0.0f),orientation(0.0) {}
	pose_type(float x,float y,float rot = 0.f, float z = 0.f):position(x,y,z),orientation(rot) {}
	
	pose_type(const pose_type &pose):position(pose.position),orientation(pose.orientation){}
	pose_type(const pose_type::pos &p,const pose_type::rot &r):position(p[0],p[1],0),orientation(r) {}
	pose_type(const pose_type::pos3 &p,const pose_type::rot &r):position(p),orientation(r) {}

	const pos &getPosition() const
	{
		return vec3to2(position);
	}

	pos coords(float x,float y)const
	{
		float cs = CS();
		float sn = SN();
		float rx = position[0] + cs * x - sn * y;
		float ry = position[1] + sn * x + cs * y;
		return pos(rx, ry);
	}

	void setPosition(const pos &pos)
	{
		position[0] = pos[0];
		position[1] = pos[1];
	}
	/// returns object direction. Equal to X axis
	dir getDirection() const
	{
		float cs = CS();
		float sn = SN();
		return axisX();
	}

	vec3 getDirection3() const
	{
		float cs = CS();
		float sn = SN();
		return vec3(cs,sn,0);
	}
	float z() const
	{
		return position[2];
	}
	/// get cos(angle)
	float CS() const	
	{
		return cosf(orientation);
	}
	/// get sin(angle)
	float SN() const
	{
		return sinf(orientation);
	}
	/// set pose direction. Equal to setting X axis
	void setDirection(const dir &d)
	{
		orientation = atan2(d[1],d[0]);
	}
	/// convert to matrix form
	mat getMat() const
	{
		mat res=mat::identity();
		float cs = CS();
		float sn = SN();
		res(0,0)=cs;res(1,0)=-sn;res(2,0)=position[0];
		res(0,1)=sn;res(1,1)= cs;res(2,1)=position[1];	
		res(2,2) = position[2];
		return res;
	}
	/// get X axis orth in world frame
	inline vec axisX() const
	{
		return vec(CS(),SN());
	}
	/// get Y axis orth in world frame
	inline vec axisY() const
	{
		return vec(-SN(), CS());
	}
	/// convert from local direction to world direction(vector)
	inline vec transformVec(const vec &v) const
	{
		float cs = CS();
		float sn = SN();
		float x = cs * v[0] - sn * v[1];
		float y = sn * v[0] + cs * v[1];
		return vec(x,y);
	}
	/// convert from local direction to world direction(vector), flat version
	inline vec3 transformVec(const vec3 &v) const
	{
		float cs = CS();
		float sn = SN();
		float x = cs * v[0] - sn * v[1];
		float y = sn * v[0] + cs * v[1];
		return vec3(x,y,0);
	}
	/// convert from local position to world position
	inline pos transformPos(const pos &p) const
	{
		float cs = CS();
		float sn = SN();
		float x = cs * p[0] - sn * p[1] + position[0];
		float y = sn * p[0] + cs * p[1] + position[1];
		return vec(x,y);
	}
	/// convert from local position to world position
	inline pos3 transformPos(const pos3 &p) const
	{
		float cs = CS();
		float sn = SN();
		float x = cs * p[0] - sn * p[1] + position[0];
		float y = sn * p[0] + cs * p[1] + position[1];
		return vec3(x,y, position[2]);
	}
	/// convert from world position to local position (untranslate+unrotate)
	inline pos projectPos(const pos &p) const
	{
		float cs = CS();
		float sn = SN();		
		float lx = p[0] - position[0];
		float ly = p[1] - position[1];		
		float x = cs * lx + sn * ly;
		float y = -sn * lx + cs * ly;
		return pos(x,y);	// unrotate
	}
	/// convert from world position to local position (untranslate+unrotate), flat version
	inline pos3 projectPos(const pos3 &p) const
	{
		float cs = CS();
		float sn = SN();
		float lx = p[0] - position[0];
		float ly = p[1] - position[1];
		float lz = p[2] - position[2];
		float x = cs * lx + sn * ly;
		float y = -sn * lx + cs * ly;
		return pos3(x,y,lz);
	}
	/// convert from world direction to local direction (unrotate)
	inline vec projectVec(const vec &v) const
	{
		float cs = CS();
		float sn = SN();
		float x = cs * v[0] + sn * v[1];
		float y = -sn * v[0] + cs * v[1];
		return vec(x,y);
	}
	inline pose_type invert() const
	{
		return pose_type(-position, -orientation);
	}
};

inline Pose2z operator * (const Pose2z &a,const Pose2z &b)
{
	return Pose2z(a.transformPos(b.position),a.orientation + b.orientation);
}

struct FastAngle
{
	float cs;
	float sn;

	FastAngle(float cs, float sn):cs(cs), sn(sn) {}
	FastAngle(const FastAngle & angle) : cs(angle.cs), sn(angle.sn){}

	FastAngle & operator = (const FastAngle & angle)
	{
		cs = angle.cs;
		sn = angle.sn;
		return *this;
	}	
	static FastAngle FromRAD(float angle)
	{
		return FastAngle(cosf(angle), sinf(angle));
	}
	inline FastAngle operator - () const
	{
		return FastAngle(cs, -sn);
	}

	float RAD() const
	{
		return atan2(sn, cs);
	}
	friend FastAngle operator + ( const FastAngle & a, const FastAngle &b)
	{
		return FastAngle(a.cs * b.cs - a.sn * b.sn, a.cs * b.sn + a.sn * b.cs);
	}

	friend FastAngle operator - ( const FastAngle & a, const FastAngle &b)
	{
		return FastAngle(a.cs * b.cs + a.sn * b.sn, a.cs * b.sn - a.sn * b.cs);
	}
};


/// Handles local coordinate system, in form of position+rotation
class Pose2
{
public:
	typedef vec2f pos;		/// describes "position" type
	typedef vec2f vec;		/// describes "vector" type
	typedef vec dir;		/// describes "direction" type. Actually is syninonim to "vector"
	typedef FastAngle rot;		/// describes "rotation" type.
	typedef Mt3x3 mat;		/// describes "matrix" type

	rot orientation;	/// cos(orientation), sin(orientation)
	pos position;			/// object global position

	typedef Pose2 pose_type;
	pose_type():position(0.0f),orientation(1.0,0.0){}
	pose_type(float x,float y,float r = 0.f):position(x,y),orientation(FastAngle::FromRAD(r)){}
	
	pose_type(const pose_type &pose):position(pose.position),orientation(pose.orientation){}
	pose_type(const pose_type::pos &p,const pose_type::rot &r):position(p), orientation(r){}

	static pose_type zero()
	{
		return pose_type(0,0,0);
	}

	const pos &getPosition() const
	{
		return position;
	}

	pos coords(float x,float y)const
	{
		float cs = CS();
		float sn = SN();
		float rx = position[0] + cs * x - sn * y;
		float ry = position[1] + sn * x + cs * y;
		return pos(rx, ry);
	}

	void setPosition(const pos &pos)
	{
		position[0] = pos[0];
		position[1] = pos[1];
	}
	/// returns object direction. Equal to X axis
	dir getDirection() const
	{
		float cs = CS();
		float sn = SN();
		return axisX();
	}

	/// get cos(angle)
	float CS() const	
	{
		return orientation.cs;
	}
	/// get sin(angle)
	float SN() const
	{
		return orientation.sn;
	}
	/// set pose direction. Equal to setting X axis
	void setDirection(const dir &d)
	{
		float len = d.length();
		orientation.cs = d[0] / len;
		orientation.sn = d[1] / len;
		//orientation = atan2(d[1],d[0]);
	}
	/// convert to matrix form
	mat getMat() const
	{
		mat res=mat::identity();
		float cs = CS();
		float sn = SN();
		res(0,0)=cs;res(1,0)=-sn;res(2,0)=position[0];
		res(0,1)=sn;res(1,1)= cs;res(2,1)=position[1];	
		res(2,2) = position[2];
		return res;
	}
	/// get X axis orth in world frame
	inline vec axisX() const
	{
		return vec(CS(),SN());
	}
	/// get Y axis orth in world frame
	inline vec axisY() const
	{
		return vec(-SN(), CS());
	}
	/// convert from local direction to world direction(vector)
	inline vec transformVec(const vec &v) const
	{
		float cs = CS();
		float sn = SN();
		float x = cs * v[0] - sn * v[1];
		float y = sn * v[0] + cs * v[1];
		return vec(x,y);
	}
	/// convert from local direction to world direction(vector), flat version
	inline vec3 transformVec(const vec3 &v) const
	{
		float cs = CS();
		float sn = SN();
		float x = cs * v[0] - sn * v[1];
		float y = sn * v[0] + cs * v[1];
		return vec3(x,y,0);
	}
	/// convert from local position to world position
	inline pos transformPos(const pos &p) const
	{
		float cs = CS();
		float sn = SN();
		float x = cs * p[0] - sn * p[1] + position[0];
		float y = sn * p[0] + cs * p[1] + position[1];
		return vec(x,y);
	}	
	/// convert from world position to local position (untranslate+unrotate)
	inline pos projectPos(const pos &p) const
	{
		float cs = CS();
		float sn = SN();		
		float lx = p[0] - position[0];
		float ly = p[1] - position[1];		
		float x = cs * lx + sn * ly;
		float y = -sn * lx + cs * ly;
		return pos(x,y);	// unrotate
	}	
	/// convert from world direction to local direction (unrotate)
	inline vec projectVec(const vec &v) const
	{
		float cs = CS();
		float sn = SN();
		float x = cs * v[0] + sn * v[1];
		float y = -sn * v[0] + cs * v[1];
		return vec(x,y);
	}
	inline pose_type invert() const
	{
		return pose_type(-position, -orientation);
	}
};

inline Pose2 operator * (const Pose2 &a,const Pose2 &b)
{
	float cs = a.CS() * b.CS() - a.SN() * b.SN();
	float sn = a.CS() * b.SN() + a.SN() * b.CS();
	vec2f pos = a.transformPos(b.position);
	return Pose2(pos, a.orientation + b.orientation);
}
#endif
