#pragma once
#ifndef MATH_MATRIX
#error "include <mathMatrix.hpp> first"
#endif

namespace frosttools
{
///\addtogroup Math
//@{
/// Partial specialization for 3x3 matrix
/**
 * Defines rotation transforms for specified axis
 */
template<class Real,bool row_order=true>
class Matrix3: public MatrixSquare<Real,3,row_order>
{
public:
	static const int size=3;								///< Defines single dimension
	typedef Matrix3<Real,row_order> my_type;				///< Defines own type
	typedef MatrixSquare<Real,3,row_order> parent_type;	///< Defines parent type
	typedef typename parent_type::value_type value_type;	///< Defines scalar type

	Matrix3 () {}
	/// Copy constructor
	Matrix3 (const my_type &m)
	{
		assign((const typename my_type::value_type*)m);
	}

	/// Get rotation matrix around X axis
	static my_type rotateX ( float angle );
	/// Get rotation matrix around Y axis
	static my_type rotateY ( float angle );
	/// Get rotation matrix around Z axis
	static my_type rotateZ ( float angle );
	/// Get rotation matrix around specified axis
	static my_type rotate ( const vec3& axis, float angle );	
};

/// Cast-trick to treat existing array as Matrix3
template<typename Real>
Matrix3<Real,true> & attachMt3(Real source[9])
{
	return *new (source) Matrix3<Real,true>;
}


template<typename Real,bool order>
Matrix3<Real,order>	Matrix3<Real,order>::rotateX ( float angle )
{
	my_type res =my_type::identity ( 1 );
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );

	res(1,1) = cosine;
	res(2,1) = -sine;
	res(1,2) = sine;
	res(2,2) = cosine;

	return res;
}

template<typename Real,bool order>
Matrix3<Real,order>	Matrix3<Real,order>::rotateY ( float angle )
{
	my_type res =my_type::identity ( 1 );
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );

	res(0,0) = cosine;
	res(2,0) = -sine;
	res(0,2) = sine;
	res(2,2) = cosine;

	return res;
}

template<typename Real,bool order>
Matrix3<Real,order>	Matrix3<Real,order>::rotateZ ( float angle )
{
	my_type res =my_type::identity ( 1 );
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );
	//    0  1  2  3   0  1  2  3
	// 0| 0  1  2  3 | 0  4  8 12|0
	// 1| 4  5  6  7 | 1  5  9 13|1
	// 2| 8  9 10 11 | 2  6 10 14|2
	// 3|12 13 14 15 | 3  7 11 15|3
	res(0,0) = cosine;
	res(1,0) = -sine;
	res(0,1) = sine;
	res(1,1) = cosine;

	return res;
}

template<typename Real,bool order>
Matrix3<Real,order>	Matrix3<Real,order>::rotate ( const vec3& axis, float angle )
{
	my_type res=my_type::identity ( 1 );
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );
	
	//axis X
	res(0,0) = axis[0] * axis[0] + ( 1 - axis[0] * axis[0] ) * cosine;
	res(0,1) = axis[0] * axis[1] * ( 1 - cosine ) + axis[2] * sine;
	res(0,2) = axis[0] * axis[2] * ( 1 - cosine ) - axis[1] * sine;
	//axis Y
	res(1,0) = axis[0] * axis[1] * ( 1 - cosine ) - axis[2] * sine;
	res(1,1) = axis[1] * axis[1] + ( 1 - axis[1] * axis[1] ) * cosine;
	res(1,2) = axis[1] * axis[2] * ( 1 - cosine ) + axis[0] * sine;
	//axis Z
	res(2,0) = axis[0] * axis[2] * ( 1 - cosine ) + axis[1] * sine;
	res(2,1) = axis[1] * axis[2] * ( 1 - cosine ) - axis[0] * sine;
	res(2,2) = axis[2] * axis[2] + ( 1 - axis[2] * axis[2] ) * cosine;

	return res;
}

//@}
}
