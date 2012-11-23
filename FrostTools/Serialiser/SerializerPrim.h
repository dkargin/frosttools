//----------------------------------------------------------)
//
//	SerializerPrim.h
//
//	Designed by Andrey Martynov
//	mailto:rek@.rsdn.ru
//
//----------------------------------------------------------)

#ifndef __SerializerPrim_h__
#define __SerializerPrim_h__

#ifndef __Serializer_h__
#	include "Serializer.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Rsdn       {
namespace Serializer {
namespace Primitives {

using namespace Utils;
//----------------------------------------------------------)

template <>
struct Primitive<bool>
{
	static tstring ToString(const bool& f)
		{ return f ? _T("true") : _T("false");}

	static bool Parse(const tstring& s)
	{ 
		if (EqualCI(s, _T("true")))
		   return true;
		else if (EqualCI(s, _T("false")))
		   return false;
		else
			throw ParseValueException(_T("Can't parse to bool: ") + s);
	}
};
//----------------------------------------------------------)

template <> 
struct Primitive<char>
{
	static tstring ToString(const char& f)
	{ tchar sz[8]; return _itot(f, sz, 10); }

	static char Parse(const tstring& s)
	{ 
		__int64 n = _tstoi64(s.c_str());
		if (n < SCHAR_MIN || n > SCHAR_MAX)
			throw ParseValueException(_T("Can't parse to char: ") + s);
		return static_cast<char>(n); 
	}
};
//----------------------------------------------------------)

template <> 
struct Primitive<byte>
{
	static tstring ToString(const byte& f)
		{ return ItoT(f); }

	static byte Parse(const tstring& s)
	{ 
		__int64 n = _tstoi64(s.c_str());
		if (n > UCHAR_MAX)
			throw ParseValueException(_T("Can't parse to byte: ") + s);
		return static_cast<byte>(n); 
	}
};
//----------------------------------------------------------)

template <>
struct Primitive<int>
{
	static tstring ToString(const int& f)
		{ return ItoT(f); }

	static int Parse(const tstring& s)
	{ 
		__int64 n = _tstoi64(s.c_str());
		if (n < INT_MIN || n > INT_MAX)
			throw ParseValueException(_T("Can't parse to int: ") + s);
		return static_cast<int>(n); 
	}
};
//----------------------------------------------------------)

template <>
struct Primitive<uint>
{
	static tstring ToString(const uint& f)
		{ return ItoT(f); }

	static uint Parse(const tstring& s)
	{ 
		__int64 n = _tstoi64(s.c_str());
		if (n > UINT_MAX)
			throw ParseValueException(_T("Can't parse to uint: ") + s);
		return static_cast<uint>(n); 
	}
};
//----------------------------------------------------------)

template <>
struct Primitive<short>
{
	static tstring ToString(const short& f)
		{ return ItoT(f); }

	static short Parse(const tstring& s)
	{ 
		__int64 n = _tstoi64(s.c_str());
		if (n < SHRT_MIN || n > SHRT_MAX)
			throw ParseValueException(_T("Can't parse to short: ") + s);
		return static_cast<short>(n); 
	}
};
//----------------------------------------------------------)

template <>
struct Primitive<ushort>
{
	static tstring ToString(const ushort& f)
		{ return ItoT(f); }

	static ushort Parse(const tstring& s)
	{ 
		__int64 n = _tstoi64(s.c_str());
		if (n > USHRT_MAX)
			throw ParseValueException(_T("Can't parse to ushort: ") + s);
		return static_cast<ushort>(n); 
	}
};
//----------------------------------------------------------)

template <> 
struct Primitive<long>
{
	static tstring ToString(const long& f)
		{ return ItoT(f); }

	static long Parse(const tstring& s)
	{ 
		__int64 n = _tstoi64(s.c_str());
		if (n < LONG_MIN || n > LONG_MAX)
			throw ParseValueException(_T("Can't parse to long: ") + s);
		return static_cast<long>(n); 
	}
};
//----------------------------------------------------------)

template <>
struct Primitive<ulong>
{
	static tstring ToString(const ulong& f)
		{ return ItoT(f); }

	static ulong Parse(const tstring& s)
	{ 
		__int64 n = _tstoi64(s.c_str());
		if (n > ULONG_MAX)
			throw ParseValueException(_T("Can't parse to ulong: ") + s);
		return static_cast<ulong>(n); 
	}
};
//----------------------------------------------------------)

template <>
struct Primitive<float>
{
	static tstring ToString(const float& f)
		{ return FtoT(f); }

	static float Parse(const tstring& s)
	{ 
		double d = _tstof(s.c_str()); 
		if (d > FLT_MAX)
			throw ParseValueException(_T("Can't parse to float: ") + s);
		return static_cast<float>(d); 
	}
};
//----------------------------------------------------------)

template <>
struct Primitive<double>
{
	static tstring ToString(const double& f)
		{ return FtoT(f); }

	static double Parse(const tstring& s)
		{ return _tstof(s.c_str()); }
};
//----------------------------------------------------------)

template <> 
struct Primitive<tstring>
{
	static tstring ToString(const tstring& f)
		{ return f; }

	static tstring Parse(const tstring& s)
		{ return s; 	}
};

//----------------------------------------------------------)

template <typename EnumType>
struct EnumMetaData 
{
protected:
	typedef std::map<EnumType, tstring> NameMap;
	typedef std::map<tstring, EnumType> ValueMap;

public:
	tstring ToString(EnumType e) const
	{
		NameMap::const_iterator it = names.find(e);
		ASSERT(it != names.end());
		return it->second;
	}

	EnumType Parse(const tstring& name) const
	{
		ValueMap::const_iterator it = values.find(Upper(name));
		if (it == values.end())
			throw ParseValueException(name + _T(" is not member of enum"));
		return it->second;
	}
protected:
	void Enumerator(const tstring& name, EnumType e)
	{
		tstring stdName = Upper(name);
		values[stdName] = e;
		names [e] = stdName;
	}

	ValueMap values;
	NameMap  names;
};

} // namespace Primitives {
//----------------------------------------------------------)

#define DEFAULT_PRIM_METADATA(prim) \
template <> inline const MetaClass<prim>& DefaultMetaClass            <prim>  () \
{ \
	static PrimClassMeta  <prim> _; \
	return _; \
} \
template <> inline const MetaClass<std::vector<prim> >& DefaultMetaClass<std::vector<prim> >() \
{ \
	static VectorClassMeta<prim> _(_T(#prim)); \
	return _; \
} \

DEFAULT_PRIM_METADATA(bool);
DEFAULT_PRIM_METADATA(byte);
DEFAULT_PRIM_METADATA(char);
DEFAULT_PRIM_METADATA(short);
DEFAULT_PRIM_METADATA(ushort);
DEFAULT_PRIM_METADATA(int);
DEFAULT_PRIM_METADATA(uint);
DEFAULT_PRIM_METADATA(long);
DEFAULT_PRIM_METADATA(ulong);
DEFAULT_PRIM_METADATA(float);
DEFAULT_PRIM_METADATA(double);
DEFAULT_PRIM_METADATA(tstring);

} // namespace Serializer {
} // namespace Rsdn       {

#define ENUM_METADATA(EnumType, EnumMetaData) \
 \
static const EnumMetaData& EnumData_Instance(void) \
{ \
	static EnumMetaData _; \
	return _; \
} \
 \
template <> \
struct Primitive<EnumType> \
{ \
	static tstring ToString(const EnumType& e) \
	{ return EnumData_Instance().ToString(e); } \
 \
	static EnumType Parse(const tstring& s) \
	{  return EnumData_Instance().Parse(s);  } \
}; \
 \
template <> \
const MetaClass<EnumType>& Serializer::DefaultMetaClass<EnumType>() \
	{ static PrimClassMeta<EnumType> _; return _; }\

#endif
//==========================================================}