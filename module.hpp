#ifndef _MODULE_HPP_
#define _MODULE_HPP_

class SysModule
{
	typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > tstring;

	//typedef std::basic_string<Char, std::char_traits<Char>, std::allocator<Char> > tstring;
	tstring fileName;
	HMODULE hLib;
	TCHAR errorMessage[256];
	DWORD lastErrorCode;
	bool useExceptions;

	inline void processError()
	{
		lastErrorCode = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,lastErrorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			errorMessage,255,NULL);
		if(useExceptions)
			throw Exception(errorMessage);
	}
public:
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

	SysModule(const TCHAR *path,bool exceptions=true)
		:hLib(NULL),lastErrorCode(0),useExceptions(exceptions)
	{
		loadModule(path);
	}
	SysModule(bool exceptions=false)
		:hLib(NULL),lastErrorCode(0),useExceptions(exceptions)
	{}
	~SysModule()
	{
		if(hLib)
			freeModule();
	}
	template<typename ProcType> bool getProcAddress(LPCTSTR name,ProcType &proc)
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

	void * getProcAddress(LPCTSTR name)
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
	int loadModule(LPCTSTR file)
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

template <class _C> class _Plugin
{
public:
	typedef _C Core;
	virtual const TCHAR *getName()=0;
	virtual void start(Core *manager)=0;
	virtual void stop()=0;
	virtual void release()=0;
};

template<class _Base=NullClass>
class _PluginManager: public _Base
{
public:
	typedef _Base Base;
	typedef _PluginManager<Base> my_type;
	typedef _Plugin<my_type> Plugin;
	typedef Plugin * (*LoadPlugin)();
	typedef std::list<std::pair<SysModule*,Plugin*> > Plugins;
	friend Plugin;

	_PluginManager()
	{}
	virtual ~_PluginManager()
	{
		unloadPlugins();
	}

	static void reportError(const char *error)
	{}
	virtual Plugin * loadLib(const TCHAR * path,const TCHAR *entry=NULL)
	{
		SysModule *module=NULL;
		Plugin *plugin=NULL;
		try
		{
			LoadPlugin loadPlugin;
			module=new SysModule(path);
			module->getProcAddress(entry?entry:TEXT("createPlugin"),loadPlugin);
			plugin=loadPlugin();
			plugin->start(this);
		}
		catch(SysModule::Exception &ex)
		{
			reportError(ex.message.c_str());
			if(plugin)
				plugin->release();
			if(module)
				delete module;
			return NULL;
		}
		plugins.push_back(std::make_pair(module,plugin));
		return plugin;
	}
	virtual int unloadPlugin(Plugin *plugin)
	{
		typename Plugins::iterator it=_find(plugin);
		if(it!=plugins.end())
		{
			Plugin *plugin=it->second;
			plugin->stop();
			plugin->release();
			delete it->first;
			plugins.erase(it);
		}
		return 0;
	}
	virtual void unloadPlugins()
	{
		while(!plugins.empty())
			unloadPlugin(plugins.front().second);
	}
protected:
	typename Plugins::iterator _find(Plugin *plugin)
	{
		typename Plugins::iterator it=plugins.begin();
		for(;it!=plugins.end();it++)
			if(it->second==plugin)
				break;
		return it;
	}
	typename Plugins::iterator _find(SysModule *module)
	{
		typename Plugins::iterator it=plugins.begin();
		for(;it!=plugins.end();it++)
			if(it->first==module)
				break;
		return it;
	}
	Plugins plugins;
};

#endif