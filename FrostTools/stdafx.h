// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>
#define NOMINMAX

#define _USE_MATH_DEFINES
#include <math.h>

#include <list>
#include <map>
#include <string>
#include <set>
#include <vector>
#include <fstream>
#include <functional>
#include <sstream>
//#include <afx.h>
//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
//include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#include <crtdbg.h>
//#include <atlbase.h>
#include <float.h>
#ifndef TSTRING_DEF
#include <string>
typedef std::basic_string<TCHAR> TString;

typedef std::basic_string<char> TStringC;
TStringC	T2C(TString T);
TString		C2T(TStringC T);
#endif

#define FrostTools_Use_System
#include "../frostTools.h"
#include "3dmath.h"
//#include "../../Dynamics/include/dynamicssystem.h"
// TODO: reference additional headers your program requires here
