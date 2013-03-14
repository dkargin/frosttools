//----------------------------------------------------------)
//
//	Util.h
//
//	Designed by Andrey Martynov
//	mailto:rek@.rsdn.ru
//
//----------------------------------------------------------)

#ifndef __Util_h__
#define __Util_h__

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//----------------------------------------------------------}

typedef size_t         index;
typedef unsigned char  byte;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

#ifdef _UNICODE
	typedef wchar_t             tchar;
	typedef std::wstring        tstring;
	#define tcout               std::wcout
#else
	typedef char                tchar;
	typedef std::string         tstring;
	#define tcout               std::cout
#endif

//----------------------------------------------------------}

namespace Rsdn  {
namespace Utils {

tstring         ErrorInfo();
tstring         ErrorMsg(uint errorCode);

inline bool	    EqualCI(const tstring&, const tstring&);
inline tstring	Upper(const tstring& str);
inline tstring  ItoT(int n);
inline tstring	FtoT(double);

//----------------------------------------------------------}

template <typename RefcountedType>
class CPtrShared
{
public:
	typedef CPtrShared<RefcountedType> my_t;

	CPtrShared(const CPtrShared&);
	CPtrShared(RefcountedType* = NULL);
	~CPtrShared();

	CPtrShared<RefcountedType>& operator=(CPtrShared<RefcountedType>& _Right);
	CPtrShared<RefcountedType>& operator=(RefcountedType*);

	bool operator == (const my_t&	 ) const;
	bool operator == (RefcountedType*) const;
	bool operator != (const my_t&	 ) const;
	bool operator != (RefcountedType*) const;

	operator RefcountedType* ()   const { return m_p; }
	RefcountedType* operator ->() const { return m_p; }

	RefcountedType** operator&(); //

	void CopyTo(RefcountedType** pp)
		{ *pp = m_p; m_p->AddRef(); }
private:
	RefcountedType* m_p;
};
//----------------------------------------------------------}

class CRefcounted
{
public:
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	CRefcounted();
	virtual ~CRefcounted() ;
#ifdef _DEBUG
	long  Counter() const;
#endif _DEBUG

private:
	long m_cRef;
};
//----------------------------------------------------------)

struct Exception : public std::exception
{
	Exception(const tstring& a_msg) : msg(a_msg) {}
	const char *what() const
		{ return CT2A(msg.c_str());	}
protected:
	const tstring& msg;
};
//----------------------------------------------------------}
//----------------------------------------------------------}
//						CPtrShared
//----------------------------------------------------------}
//----------------------------------------------------------}

template <class T> inline
CPtrShared<T>::CPtrShared(T* p)
	: m_p( p )
{
	if (m_p != NULL)
		m_p->AddRef();
}
//----------------------------------------------------------}

template <class T> inline
CPtrShared<T>::CPtrShared(const CPtrShared<T>& p)
	: m_p(p.m_p)
{
	if (m_p != NULL)
		m_p->AddRef();
}
//----------------------------------------------------------}

template <class Type> inline
CPtrShared<Type>::~CPtrShared()
{
	if (m_p != NULL)
		m_p->Release();
}
//----------------------------------------------------------}

template <class T> inline
CPtrShared<T>& CPtrShared<T>::operator=(CPtrShared<T>& a)
{
	if (*this != a)
	{	
		if (m_p != NULL)
			m_p->Release();

		m_p = a.m_p; 

		if (m_p != NULL)
			m_p->AddRef();
	}
	return *this; 
}
//----------------------------------------------------------}

template <class T> inline
CPtrShared<T>& CPtrShared<T>::operator=(T* p)
{
	if (m_p != p)
	{	
		if (m_p != NULL)
			m_p->Release();

		m_p = p; 

		if (m_p != NULL)
			m_p->AddRef();
	}
	return *this; 
}
//----------------------------------------------------------}

template <class T> inline
T** CPtrShared<T>::operator&()
{
	if (m_p)
	{
		m_p->Release();
		m_p = NULL;
	}
	return &m_p;
}
//----------------------------------------------------------}

template <class T> inline
bool CPtrShared<T>::operator == (const CPtrShared<T>& p) const
	{ return m_p == p.m_p; }

template <class T> inline
bool CPtrShared<T>::operator == (T* p) const
	{ return m_p == p; }

template <class T> inline
bool CPtrShared<T>::operator != (const CPtrShared<T>& p) const
	{ return m_p != p.m_p; }

template <class T> inline
bool CPtrShared<T>::operator != (T* p) const
	{ return m_p != p; }

//----------------------------------------------------------}
//----------------------------------------------------------}
//						CRefcounted
//----------------------------------------------------------}
//----------------------------------------------------------}

inline CRefcounted::CRefcounted()
	: m_cRef(0)
	{}

inline CRefcounted::~CRefcounted()
	{ assert(m_cRef == 0); }

inline ULONG CRefcounted::AddRef()
	{ return InterlockedIncrement(&m_cRef) ; }

inline ULONG CRefcounted::Release()
{
	InterlockedDecrement(&m_cRef) ;
	if (m_cRef == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef ;
}
#ifdef _DEBUG
inline long  CRefcounted::Counter() const
	{ 
		return m_cRef; 
	}
#endif _DEBUG

//----------------------------------------------------------}
//----------------------------------------------------------}
//						strings
//----------------------------------------------------------}
//----------------------------------------------------------}

inline tstring ItoT(int n)
	{ tchar sz[12]; return _itot(n, sz, 10); }

inline tstring FtoT(double d)
	{ char buffer[50]; _gcvt(d, 7, buffer); return tstring(CA2T(&buffer[0]));}

inline bool EqualCI(const tstring& s1, const tstring& s2) 
	{ return ! _tcsicmp(s1.c_str(), s2.c_str()); }

inline tstring	Upper(const tstring& str)
	{	
		tchar* up = _tcsupr(_tcsdup(str.c_str()));
		tstring s(up);
		::free(up);
		return s; 
	}
}
} // namespace Rsdn

#endif  // __Util_h__
//==========================================================)