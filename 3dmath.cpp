

#include "stdafx.h"
#include "3dmath.h"

//////////////////////// Derived functions /////////////////////////////


//////////////////////// Derived functions /////////////////////////////

Mt4x4	translate ( const vec3& loc )
{
	Mt4x4	res ( 1 );

	//res.x [0][3] = loc[0];
	//res.x [1][3] = loc[1];
	//res.x [2][3] = loc[2];
	res.c [ 3] = loc[0];
	res.c [ 7] = loc[1];
	res.c [11] = loc[2];

	return res;
}

Mt4x4	scale ( const vec3& v )
{
	Mt4x4	res ( 1 );

	//res.x [0][0] = v[0];
	//res.x [1][1] = v[1];
	//res.x [2][2] = v[2];
	res.c [ 0] = v[0];
	res.c [ 5] = v[1];
	res.c [10] = v[2];

	return res;
}

Mt4x4	rotateX ( float angle )
{
	Mt4x4 res ( 1 );
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );

	//res.x [1][1] = cosine;
	//res.x [1][2] = -sine;
	//res.x [2][1] = sine;
	//res.x [2][2] = cosine;
	res.c [ 5] = cosine;
	res.c [ 6] = -sine;
	res.c [ 9] = sine;
	res.c [10] = cosine;


	return res;
}

Mt4x4	rotateY ( float angle )
{
	Mt4x4 res ( 1 );
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );

	//res.x [0][0] = cosine;
	//res.x [0][2] = -sine;
	//res.x [2][0] = sine;
	//res.x [2][2] = cosine;
	res.c [ 0] = cosine;
	res.c [ 2] = -sine;
	res.c [ 8] = sine;
	res.c [10] = cosine;

	return res;
}

Mt4x4	rotateZ ( float angle )
{
	Mt4x4 res ( 1 );
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );

	//res.x [0][0] = cosine;
	//res.x [0][1] = -sine;
	//res.x [1][0] = sine;
	//res.x [1][1] = cosine;

	res.c [0] = cosine;
	res.c [1] = -sine;
	res.c [4] = sine;
	res.c [5] = cosine;

	return res;
}

Mt4x4	rotation ( const vec3& axis, float angle )
{
	Mt4x4 res ( 1 );
	float  cosine = cos ( angle );
	float  sine   = sin ( angle );

	res.c [ 0] = axis[0] * axis[0] + ( 1 - axis[0] * axis[0] ) * cosine;
	res.c [ 4] = axis[0] * axis[1] * ( 1 - cosine ) + axis[2] * sine;
	res.c [ 8] = axis[0] * axis[2] * ( 1 - cosine ) - axis[1] * sine;
	res.c [12] = 0;

	res.c [1] = axis[0] * axis[1] * ( 1 - cosine ) - axis[2] * sine;
	res.c [5] = axis[1] * axis[1] + ( 1 - axis[1] * axis[1] ) * cosine;
	res.c [9] = axis[1] * axis[2] * ( 1 - cosine ) + axis[0] * sine;
	res.c [13] = 0;

	res.c [2] = axis[0] * axis[2] * ( 1 - cosine ) + axis[1] * sine;
	res.c [6] = axis[1] * axis[2] * ( 1 - cosine ) - axis[0] * sine;
	res.c [10] = axis[2] * axis[2] + ( 1 - axis[2] * axis[2] ) * cosine;
	res.c [14] = 0;

	res.c [3] = 0;
	res.c [7] = 0;
	res.c [10] = 0;
	res.c [15] = 1;

	return res;
}

Mt4x4	mirrorX ()
{
	Mt4x4	res ( 1 );

	res.c [0] = -1;

	return res;
}

Mt4x4	mirrorY ()
{
	Mt4x4	res ( 1 );

	res.c [5] = -1;

	return res;
}

Mt4x4	mirrorZ ()
{
	Mt4x4	res ( 1 );

	res.c [10] = -1;

	return res;
}

Pose operator * ( const Mt4x4& m,const Pose& v )
{
	Pose res;
	res.p=m*v.p;	
	res.dir[0] = m.c [0] * v.dir[0] + m.c [1] * v.dir[1] + m.c [2] * v.dir[2];
	res.dir[1] = m.c [4] * v.dir[0] + m.c [5] * v.dir[1] + m.c [6] * v.dir[2];
	res.dir[2] = m.c [8] * v.dir[0] + m.c [9] * v.dir[1] + m.c [10] * v.dir[2];
	return res;

}