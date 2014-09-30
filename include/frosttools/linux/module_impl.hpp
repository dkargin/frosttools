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
		char *c = dlerror();
		if (c != NULL) {
			printf("linx/module_impl.h:SysModule Dlopen error = %s \n", c);
		} else {
			printf("linx/module_impl.h:SysModule Dlopen error = NULL \n");
		}

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

	struct Exception
	{
		std::string message;
		Exception(const std::string &msg):message(msg){}
		Exception(const Exception &ex):message(ex.message){}
		const std::string::value_type * getMessage()
		{
			return message.c_str();
		}
	};

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
