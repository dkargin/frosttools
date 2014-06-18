/*
 * mathBase.hpp
 *
 *  Created on: Jun 4, 2014
 *      Author: vrobot
 */

#ifndef MATHBASE_HPP_
#define MATHBASE_HPP_

#define _USE_MATH_DEFINES
#include	<math.h>
//!\defgroup Math Mathematics
/// Mathematics library. Contains classes for linear algebra and set of simple geomety primitives

//!\defgroup Geometry Geometry
///	Classes for simple geometry objects

///\brief frosttools
namespace frosttools
{
///\addtogroup Math
///	Provides matrix and vector types for linear algebra and operations with them
//@{

/// Clamp value by specified maximum
inline float clampf(float value,float max)
{
	signed int n=(int)floor(value/max);
	return value-max*n;
}

/// 180/Pi
const float PI_180=57.295779513082320876798154814105f;
/// 2*Pi
const float M_2PI=(float)2.0f*M_PI;

/// Degrees to Radians
inline float DEG2RAD(float a)
{
	return a/PI_180;
}

/// Radians to degrees
inline float RAD2DEG(float a)
{
	return a*PI_180;
}

/// Clamp angle to [-180;180] interval
inline float ClampAngle(float angle)
{
	while(angle>180.0f)
		angle-=360.0;
	while(angle<-180.0f)
		angle+=360.0;
	return angle;
}

/// Clamp value to [min, max] range
template<class Real> inline Real clamp(const Real &val,const Real &min,const Real &max)
{
	if(val<min)
		return min;
	else if(val>max)
		return max;
	return val;
}

/// Dead zone filter
/// returns 0 if value lies in [min;max] range, val otherwise
template<class Real> inline Real deadZone(const Real &val,const Real &min,const Real &max)
{
	if(val>min && val<max)
		return 0;
	return val;
}

/// check if value is small enough
inline bool fZero(float f)
{
	const float low=0.000005f;
	if(f<low && f>-low)
		return true;
	return false;
}

/// Get value sign
/*
 * returns -1.0 if negative, 1.0 if positive, 0.0 if zero
 */
inline float fSign(float f)
{
	if(fZero(f))
		return 0.0f;
	return f>0.0f?1.0f:-1.0f;
}

/// Clamp filter
struct Clamp
{
  typedef float value_type;	///< defines scalar type
  typedef Clamp my_type;		///< defines own type
  value_type min;				///< defines minimal value
  value_type max;				///< defines maximal value
  /// Constructor
  Clamp(const value_type &min,const value_type &max)
    :min(min),max(max)
  {}
  /// Copy constructor
  Clamp(const my_type & clamp)
    :min(clamp.min),max(clamp.max)
  {}
  /// Apply filter
  value_type operator*(const value_type & val) const
  {
    if(val > max)
      return max;
    if(val < min)
      return min;
    return val;
  }
  /// Assignment operator
  my_type & operator = (const my_type & val)
  {
    min = val.min;
    max = val.max;
    return *this;
  }
};

/// Space relation for two edges
enum spaceRelation
{
	srIntersect,
	srNoIntersect,
	srParallel,
	srCross,
	srEqual
};

/// Space relation for two planes
enum planeRelation
{
	prBack,
	prFront,
	prIntersect
};

/// Does anybody needs it?
template<class Real> struct MathTypes
{
	typedef int size_type;	///< Definesi index type
};

}	// namespace frosttools

#endif /* MATHBASE_HPP_ */
