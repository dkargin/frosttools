#include "stdafx.h"
//#include "at"

#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif

TStringC T2C(TString T)
{
	if(sizeof(CHAR)==sizeof(TCHAR) )
		return *((TStringC *)&T);
	char Buf[1024];
	char *Str=Buf;
	TStringC R;
/*	bool Resize=(T.size()>=1023);
	if(Resize)
	{
		Str=new char[T.size()+1];
	}
	int Q;
	Q=wcstombs(Str,T.c_str(),T.size());
	R=Str;
	if(Resize)
		delete Str;*/
//	USES_CONVERSION;
//	Str=W2A(T.c_str());
	
	return Str;
}

TString C2T(TStringC T)
{
	if(sizeof(CHAR)==sizeof(TCHAR) )
		return *((TString *)&T);
	TCHAR Buf[1024];
	TCHAR *Str=Buf;
	TStringC R;
/*	bool Resize=(T.size()>=1023);
	if(Resize)
	{
		Str=new TCHAR[T.size()+1];
	}
	int Q;
	Q=mbstowcs(Str,T.c_str(),T.size());
	R=Str;
	if(Resize)
		delete Str;*/
//	USES_CONVERSION;        !!!
//	Str=A2W(T.c_str());		!!!

	return Str;
}