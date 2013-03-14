//----------------------------------------------------------)
//
//	Util.cpp
//
//	Designed by Andrey Martynov
//	mailto:rek@.rsdn.ru
//
//----------------------------------------------------------)

#include "stdafx.h"
#include "SerializerUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Rsdn  {
namespace Utils {
//----------------------------------------------------------}

tstring ErrorInfo()
{
	tstring str;
	try
	{
		CComPtr<IErrorInfo> pErrorInfo;
		HRESULT hr = ::GetErrorInfo(0, &pErrorInfo);
		if (hr == S_FALSE)
			return _T("No ErrorInfo");
		if (hr == S_OK)
		{
			CComBSTR bstrDescr;
			if (FAILED(pErrorInfo->GetDescription(&bstrDescr)))
				throw std::exception();
			str += _T(" Description: ") + tstring(CW2T(bstrDescr));

			CComBSTR bstrSource;
			if (FAILED(pErrorInfo->GetSource(&bstrSource)))
				throw std::exception();
			str += _T(", Source: ") + tstring(CW2T(bstrSource));
		}
	}
	catch (...)
	{
	}
	return str;
}

//----------------------------------------------------------}

tstring ErrorMsg(uint errorCode)
{
	tchar szMsg[1024];
	DWORD dwChars = ::FormatMessage( 
						  FORMAT_MESSAGE_FROM_SYSTEM
						| FORMAT_MESSAGE_IGNORE_INSERTS
						, 0, errorCode
						, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
						, szMsg, sizeof(szMsg), NULL 
						);
	if (dwChars == 0)
		return _T("Unknown error code");

	return szMsg;
}

} // namespace Utils {
} // namespace Rsdn  {
//==========================================================)