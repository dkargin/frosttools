#pragma once
#ifndef MATH_MATRIX
#error "include <mathMatrix.hpp> first"
#endif

template <typename Real,bool row_order=false>
class Matrix4 : public MatrixSquare<Real,4,row_order>
{
public:
	typedef MatrixSquare<Real,4,row_order> parent_type;
	typedef typename parent_type::value_type value_type;
	typedef Matrix4<Real,row_order> my_type;
	Matrix4 () {}
	// copy constructor
	Matrix4 (const Matrix<Real,4,4,row_order> &m)
	{
		this->assign((const Real*)m);
	}
	template<class R> inline Matrix4(const Vector<R,4*4> &m)
	{
		assign((const R*)m);
	}
	template<class R>Matrix4(const R *m)
	{
		assign(m);
	}

	inline void axisX(vec3 v)
	{
	    value_type *c=(value_type*)this;
		this->set(0,0,v[0]);
		this->set(0,1,v[1]);
		this->set(0,2,v[2]);
		//c[0]=v[0];
		//c[4]=v[1];
		//c[8]=v[2];
	}
	inline void axisX(Real x,Real y,Real z)
	{
		axisX(vec3(x,y,z));
	}
	inline void axisY(vec3 v)
	{
	    value_type *c=(value_type*)this;
	    this->set(1,0,v[0]);
	    this->set(1,1,v[1]);
	    this->set(1,2,v[2]);
	}
	inline void axisY(Real x,Real y,Real z)
	{
		axisY(vec3(x,y,z));
	}
	inline void axisZ(vec3 v)
	{
	    value_type *c=(value_type*)this;
	    this->set(2,0,v[0]);
		this->set(2,1,v[1]);
		this->set(2,2,v[2]);
	}
	inline void axisZ(Real x,Real y,Real z)
	{
		axisZ(vec3(x,y,z));
	}
	inline void origin(const vec3 &v)
	{
	    //value_type *c=(value_type*)this;
	    this->set(3,0,v[0]);
	    this->set(3,1,v[1]);
	    this->set(3,2,v[2]);
	}
	inline void yaw(float ang)//around y axis
	{
		(*this)*=rotateY(ang);
	}
	inline void pitch(float ang)//around Z axis
	{
		(*this)*=rotateX(ang);
	}
	inline void origin(Real x,Real y,Real z)
	{
	    //value_type *c=(value_type*)this;
		origin(vec3(x,y,z));
	}
	inline vec3 axisX() const
	{
	    //const value_type *c=(const value_type*)this;
		return vec3(this->get(0,0),this->get(0,1),this->get(0,2));
	}
	inline vec3 axisY() const
	{
	    //const value_type *c=(const value_type*)this;
		return vec3(this->get(1,0),this->get(1,1),this->get(1,2));
	}
	inline vec3 axisZ() const
	{
	    //const value_type *c=(const value_type*)this;
		return vec3(this->get(2,0),this->get(2,1),this->get(2,2));
	}
	inline vec3 origin() const
	{
	    //const value_type *c=(const value_type*)this;
		return vec3(this->get(3,0),this->get(3,1),this->get(3,2));
	}
	inline vec3 project(const vec3 &a)const//returns a coordinates in this system
	{
		return vec3(::vecProjectLen(a-origin(),axisX()),
					::vecProjectLen(a-origin(),axisY()),
					::vecProjectLen(a-origin(),axisZ()));
	}
	inline vec3 projectDir(const vec3 &a)
	{
		return vec3(::vecProjectLen(a,axisX()),
					::vecProjectLen(a,axisY()),
					::vecProjectLen(a,axisZ()));
	}
	inline vec3 vecProject(vec3 a)const
	{
		return vec3(::vecProjectLen(a,axisX()),
					::vecProjectLen(a,axisY()),
					::vecProjectLen(a,axisZ()));
	}
	static my_type	translate ( const vec3& );
	static my_type  translate (Real x,Real y,Real z)
	{
		my_type	res = Matrix4::identity();
		res.origin(x,y,z);
		return res;
	}
	static my_type	scale     ( const vec3& );
	static my_type	scale     ( float x,float y,float z );
	static my_type	rotateX   ( float );
	static my_type	rotateY   ( float );
	static my_type	rotateZ   ( float );
	static my_type	rotate    ( const vec3& v, float );
	//static my_type	mirrorX   ();
	//static my_type	mirrorY   ();
	//static my_type	mirrorZ   ();

	//	Make a rotation matrix from Euler angles. The 4th row and column are unmodified. //
	my_type &setRotationRadians( const float *angles );

	//	Make a rotation matrix from Euler angles. The 4th row and column are unmodified.
	my_type &setRotationDegrees( const float *angles );

	//	Make an inverted rotation matrix from Euler angles. The 4th row and column are unmodified.
	my_type &setInverseRotationRadians( const float *angles );

	//	Make an inverted rotation matrix from Euler angles. The 4th row and column are unmodified.
	my_type &setInverseRotationDegrees( const float *angles );
	my_type &operator=(const parent_type &m)
	{
		for(int i=0;i< this->cols()*this->rows();i++)
			(*this)[i]=m[i];
		return *this;
	}
	operator parent_type ()
	{
		return *this; 
	}
};

template<typename Real>
Matrix4<Real,false> & attachMt4c(Real source[16])
{
	return *new (source) Matrix4<Real>;
}
template<typename Real>
Matrix4<Real,true> & attachMt4r(Real source[16])
{
	return *new (source) Matrix4<Real>;
}


/*
template<typename Real>
Matrix3<Real,true> getRotation(const Matrix4<Real> mt4)
{
	Matrix3<Real,true> res(1.0);
	vec3 t;
	for(int i=0;i<3;i++)
	{
		Vector<Real,4> col=mt4.col(i);
		res.col(i,col);
	}
	return res;
}

template<typename Real>
Matrix4<Real> setRotation(Matrix4<Real,false> mt4,Matrix3<Real,true> rot)
{
	for(int i=0;i<3;i++)
	{
		Vector<Real,4> col4(Real(0));
		// use placement new to treat col4 as Vector<Real,3>. Pretty nice
		Vector<Real,3> *col3=new (&col4[0]) Vector<Real,3>(rot.col(i));
		mt4.col(i,col4);
	}
	return mt4;
}
*/
template<typename Real,bool order> Matrix4<Real,order> Matrix4<Real,order>::translate ( const vec3& loc )
{
	Matrix4<Real,order> res = my_type::identity();

	res.origin(loc);
	return res;
}
template<typename Real,bool order> Matrix4<Real,order>	Matrix4<Real,order>::scale ( float x, float y, float z)
{
	Matrix4<Real,order> res = my_type::identity();

	res(0,0) = x;
	res(1,1) = y;
	res(2,2) = z;

	return res;
}

template<typename Real,bool order> Matrix4<Real,order>	Matrix4<Real,order>::scale ( const vec3& v )
{
	Matrix4<Real,order> res = my_type::identity();

	res(0,0) = v[0];
	res(1,1) = v[1];
	res(2,2) = v[2];
	return res;
}

template<typename Real,bool order>
Matrix4<Real,order>	Matrix4<Real,order>::rotateX ( float angle )
{
	my_type res = my_type::identity();
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );

	res(1,1) = cosine;
	res(2,1) = -sine;
	res(1,2) = sine;
	res(2,2) = cosine;

	return res;
}
template<typename Real,bool order>
Matrix4<Real,order>	Matrix4<Real,order>::rotateY ( float angle )
{
	my_type res=my_type::identity();
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );

	res(0,0) = cosine;
	res(2,0) = -sine;
	res(0,2) = sine;
	res(2,2) = cosine;

	return res;
}
template<typename Real,bool order>
Matrix4<Real,order>	Matrix4<Real,order>::rotateZ ( float angle )
{
	my_type res=my_type::identity();
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
Matrix4<Real,order>	Matrix4<Real,order>::rotate ( const vec3& axis, float angle )
{
	my_type res=my_type::identity();
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );
	
	//axis X
	res(0,0) = axis[0] * axis[0] + ( 1 - axis[0] * axis[0] ) * cosine;
	res(0,1) = axis[0] * axis[1] * ( 1 - cosine ) + axis[2] * sine;
	res(0,2) = axis[0] * axis[2] * ( 1 - cosine ) - axis[1] * sine;
	res(0,3) = 0;
	//axis Y
	res(1,0) = axis[0] * axis[1] * ( 1 - cosine ) - axis[2] * sine;
	res(1,1) = axis[1] * axis[1] + ( 1 - axis[1] * axis[1] ) * cosine;
	res(1,2) = axis[1] * axis[2] * ( 1 - cosine ) + axis[0] * sine;
	res(1,3) = 0;
	//axis Z
	res(2,0) = axis[0] * axis[2] * ( 1 - cosine ) + axis[1] * sine;
	res(2,1) = axis[1] * axis[2] * ( 1 - cosine ) - axis[0] * sine;
	res(2,2) = axis[2] * axis[2] + ( 1 - axis[2] * axis[2] ) * cosine;
	res(2,3) = 0;

	res(3,0)=0;res(3,1)=0;res(3,2)=0;res(3,3)=1;
	return res;
}
//
//template<typename Real,bool order> Matrix4<Real,order>	Matrix4<Real,order>::mirrorX ()
//{
//	Matrix4<Real,order>	res ( 1 );
//
//	res[0] = -1;
//
//	return res;
//}
//
//template<typename Real,bool order>
//Matrix4<Real,order>	Matrix4<Real,order>::mirrorY ()
//{
//	Matrix4<Real,order>	res ( 1 );
//
//	res[5] = -1;
//
//	return res;
//}
//
//template<typename Real,bool order>
//Matrix4<Real,order>	Matrix4<Real,order>::mirrorZ ()
//{
//	Matrix4<Real,order>	res ( 1 );
//
//	res[10] = -1;
//
//	return res;
//}

template<typename Real,bool order>
Vector3D<Real> operator * ( const Matrix4<Real,order>& m, const Vector3D<Real>& v )
{
	//Vector3D<Real> res;

	//res[0] = m.c [0] * v[0] + m.c [1] * v[1] + m.c [ 2] * v[2] + m.c [ 3];
	//res[1] = m.c [4] * v[0] + m.c [5] * v[1] + m.c [ 6] * v[2] + m.c [ 7];
	//res[2] = m.c [8] * v[0] + m.c [9] * v[1] + m.c [10] * v[2] + m.c [11];

	//float	denom = m.c [12] * v[0] + m.c [13] * v[1] +  m.c [14] * v[1] + m.c [15];
	//if ( denom != 1.0 )
	//	res /= denom;
	//return res;
	return transform(m,v);
}

template<class Real,bool order>
inline Matrix4<Real,order> & Matrix4<Real,order>::setRotationDegrees( const float *angles )
{
	const float m_180_pi=180.0/M_PI;
	float vec[3];
	vec[0] = ( float )( angles[0]*m_180_pi );
	vec[1] = ( float )( angles[1]*m_180_pi );
	vec[2] = ( float )( angles[2]*m_180_pi );
	return setRotationRadians( vec );
}

template<class Real,bool order>
inline Matrix4<Real,order> &Matrix4<Real,order>::setInverseRotationDegrees( const float *angles )
{
	const float m_180_pi=180.0/M_PI;
	float vec[3];
	vec[0] = ( float )( angles[0]*m_180_pi );
	vec[1] = ( float )( angles[1]*m_180_pi );
	vec[2] = ( float )( angles[2]*m_180_pi );
	return setInverseRotationRadians( vec );
}

template<class Real,bool order>
Matrix4<Real,order> &Matrix4<Real,order>::setRotationRadians( const float *angles )
{
    value_type *c=(value_type*)this;
	//Matrix4<Real> m_matrix;
	float cr = cos( angles[0] );
	float sr = sin( angles[0] );
	float cp = cos( angles[1] );
	float sp = sin( angles[1] );
	float cy = cos( angles[2] );
	float sy = sin( angles[2] );
	float srsp = sr*sp;
	float crsp = cr*sp;
	//    0  1  2  3   0  1  2  3
	// 0| 0  1  2  3 | 0  4  8 12|0
	// 1| 4  5  6  7 | 1  5  9 13|1
	// 2| 8  9 10 11 | 2  6 10 14|2
	// 3|12 13 14 15 | 3  7 11 15|3
	/*
	//column order
	c[0] = Real( cp*cy );
	c[1] = Real( cp*sy );
	c[2] = Real( -sp );
	c[4] = Real( srsp*cy-cr*sy );
	c[5] = Real( srsp*sy+cr*cy );
	c[6] = Real( sr*cp );
	c[8] = Real( crsp*cy+sr*sy );
	c[9] = Real( crsp*sy-sr*cy );
	c[10] = Real( cr*cp );
	*/
	// row order
	set(0,0,Real( cp*cy ));
	set(0,1,Real( cp*sy ));
	set(0,2,Real( -sp ));

	set(1,0,Real( srsp*cy-cr*sy ));
	set(1,1,Real( srsp*sy+cr*cy ));
	set(1,2,Real( sr*cp ));

	set(2,0,Real( crsp*cy+sr*sy ));
	set(2,1,Real( crsp*sy-sr*cy ));
	set(2,2,Real( cr*cp ));

	return *this;
}

template<class Real,bool order>
Matrix4<Real,order>& Matrix4<Real,order>::setInverseRotationRadians( const float *angles )
{
	assert(false);// no column order
	//_CrtDbgBreak();
	//Matrix4<Real> m_matrix;
	value_type *c=(value_type*)this;
	float cr = cos( angles[0] );
	float sr = sin( angles[0] );
	float cp = cos( angles[1] );
	float sp = sin( angles[1] );
	float cy = cos( angles[2] );
	float sy = sin( angles[2] );
	float srsp = sr*sp;
	float crsp = cr*sp;
	/*
	//column order
	c[0] = Real( cp*cy );
	c[1] = Real( cp*sy );
	c[2] = Real( -sp );
	c[4] = Real( srsp*cy-cr*sy );
	c[5] = Real( srsp*sy+cr*cy );
	c[6] = Real( sr*cp );
	c[8] = Real( crsp*cy+sr*sy );
	c[9] = Real( crsp*sy-sr*cy );
	c[10] = Real( cr*cp );*/
	//row order
	c[0] = Real( cp*cy );
	c[4] = Real( cp*sy );
	c[8] = Real( -sp );
	c[1] = Real( srsp*cy-cr*sy );
	c[5] = Real( srsp*sy+cr*cy );
	c[9] = Real( sr*cp );
	c[2] = Real( crsp*cy+sr*sy );
	c[6] = Real( crsp*sy-sr*cy );
	c[10] = Real( cr*cp );
	return *this;
	//Matrix4<Real> m_matrix;
}
