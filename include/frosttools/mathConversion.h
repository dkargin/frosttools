#pragma once

/// Defines conversions between ORGE, PhysX and frosttools linear math

#ifdef OGRE_COMPILER
inline vec3 conv(Ogre::Vector3 v)
{
	return vec3(v.x,v.y,v.z);
}
inline Mt4x4 conv(Ogre::Matrix3 mt)
{
	Mt4x4 res(1.0);
	res.axisX(conv(mt.GetColumn(0)));
	res.axisY(conv(mt.GetColumn(1)));
	res.axisZ(conv(mt.GetColumn(2)));
	return res;
}
inline const Ogre::Vector3&	convert(const NxVec3& v) {
	static Ogre::Vector3 tmp;
	tmp.x = v.x;
	tmp.y = v.y;
	tmp.z = v.z;
	return	tmp;
}
inline Ogre::Vector3 conv(const vec3 &v)
{
	return Ogre::Vector3(v[0],v[1],v[2]);
}
#endif

#ifdef NX_FOUNDATION_NX

inline vec3 conv(const NxVec3 &v)
{
	return vec3(v.x,v.y,v.z);
}

inline NxVec3 nxConv(const vec3 &v)
{
	return NxVec3(&v[0]);
}

inline Mt4x4 conv(const NxMat34 &mt)
{   
	Mt4x4 res;	
	if(res.rowOrder())
		mt.getRowMajor44(res.c);
	else
		mt.getColumnMajor44(res.c);
	return res;
}
inline NxMat34 nxConv(const Mt4x4 &m)
{
	NxMat34 mt;
	if(Mt4x4::rowOrder())
		mt.setRowMajor44(m.c);	
	else
		mt.setColumnMajor44(m.c);
	return mt;
}

inline const NxVec3&	convert(float x, float y, float z) {
	static NxVec3 tmp;
	tmp.set(x,y,z);
	return	tmp;
}
#endif

#ifdef NX_FOUNDATION_NX 
#ifdef OGRE_COMPILER

/*inline const NxExtendedVec3& toNxExtendedVec3(const Ogre::Vector3& v) {
	static NxExtendedVec3 tmp;
	tmp.set(v.x, v.y, v.z);
	return	tmp;
}*/

inline const NxVec3&	convert(const Ogre::Vector3& v) {
	static NxVec3 tmp;
	tmp.set(v.x,v.y,v.z);
	return	tmp;
}

inline const Ogre::Quaternion& convert(const NxQuat& q) {	
	static Ogre::Quaternion tmp;
	tmp.w = q.w;
	tmp.x = q.x;
	tmp.y = q.y;
	tmp.z = q.z;
	return tmp;
}

inline const NxQuat & convert(const Ogre::Quaternion& q, bool _normalise = true) {

	Ogre::Quaternion q2 = q;
	if (_normalise)
		q2.normalise();
	
	static NxQuat a;
	a.x = q2.x;
	a.y = q2.y;
	a.z = q2.z;
	a.w = q2.w;
	return a;
}

inline const NxRay&	convert(const Ogre::Ray& r) {
	static NxRay tmp;
	tmp.orig = convert(r.getOrigin());
	tmp.dir = convert(r.getDirection());
	return	tmp;
}
/*
inline const Ogre::Vector3&	convert(const NxExtendedVec3& v) {
	static Ogre::Vector3 tmp;
	tmp.x = v.x;
	tmp.y = v.y;
	tmp.z = v.z;
	return	tmp;
}*/
#endif
#endif
