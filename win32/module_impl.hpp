#ifndef _MODULE_W32_HPP_
#define _MODULE_W32_HPP_

class SysModule
{
	//typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > tstring;

	//typedef std::basic_string<Char, std::char_traits<Char>, std::allocator<Char> > tstring;
	std::string fileName;

	HMODULE hLib;
	
	DWORD lastErrorCode;
	/*
	bool useExceptions;*/

	inline void processError()
	{
		TCHAR errorMessage[256];
		lastErrorCode = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,lastErrorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			errorMessage,255,NULL);
		if(useExceptions)
			throw Exception(errorMessage);
	}
public:
	/*
	struct Exception
	{
		tstring message;
		Exception(const tstring &msg):message(msg){}
		Exception(const Exception &ex):message(ex.message){}
		const tstring::value_type * getMessage()
		{
			return message.c_str();
		}
	};*/

	SysModule(const char *path)
		:hLib(NULL),lastErrorCode(0)
	{
		loadModule(path);
	}

	SysModule()
		:hLib(NULL),lastErrorCode(0)
	{}

	~SysModule()
	{
		if(hLib)
			freeModule();
	}

	template<typename ProcType> bool getProcAddress(const char * name,ProcType &proc)
	{
		void *ptr=getProcAddress(name);
		proc=(ProcType)ptr;
		if(!ptr)
		{
			processError();
			return false;
		}
		return true;
	}

	void * getProcAddress(const char * name)
	{
#ifdef _UNICODE
		char fn[255];
		wcstombs(fn,name,wcslen(name));
		void *res=(void*)GetProcAddress(hLib,fn);
#else
		void *res=(void* )GetProcAddress(hLib,name);
#endif
		if(!res)
			processError();
		return res;
	}
	int loadModule(const char * file)
	{
		fileName=file;
		hLib=LoadLibrary(file);
		if(!hLib)
		{
			processError();
			return FALSE;
		}
		return TRUE;
	}
	int freeModule()
	{
		BOOL res=FreeLibrary(hLib);
		hLib=NULL;
		return res;
	}
	bool loaded()
	{
		return hLib!=NULL;
	}
};

#endif