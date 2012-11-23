#ifndef _MODULE_IMPL_HPP_
#define _MODULE_IMPL_HPP_

#include <string>
#include <dlfcn.h>

class SysModule
{
	std::string fileName;
	void * hLib;
	
	int lastErrorCode;
	
	//bool useExceptions;

	inline void processError()
	{
		/*
		lastErrorCode = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,lastErrorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			errorMessage,255,NULL);*/
	/*
		if(useExceptions)
			throw Exception(errorMessage);*/
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
	};
	*/
	SysModule(const char * path)
		:hLib(NULL), lastErrorCode(0)
	{
		loadModule(path);
	}

	SysModule()
		:hLib(NULL), lastErrorCode(0)
	{}

	~SysModule()
	{
		if(hLib)
			freeModule();
	}

	template<typename ProcType> bool getProcAddress(const char * name,ProcType &proc)
	{
		void * ptr=getProcAddress(name);
		proc = (ProcType)ptr;
		if(!ptr)
		{
			processError();
			return false;
		}
		return true;
	}

	int loadModule(const char * file)
	{
		fileName = file;
		
		hLib = dlopen(file, RTLD_NOW);

		if(!hLib)
		{
			processError();
			return -1;
		}
		return 0;
	}

	int freeModule()
	{
		if(dlclose(hLib) != 0)
		{
			processError();
			return -1;
		}		
		return 0;
	}

	bool loaded()
	{
		return hLib!=NULL;
	}
private:
	void * getProcAddress(const char * name)
	{
		void * res = dlsym(hLib,name);
		if(!res)
			processError();
		return res;
	}
};

#endif