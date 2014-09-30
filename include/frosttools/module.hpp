#ifndef _MODULE_HPP_
#define _MODULE_HPP_

/// include platform-specific variants
#ifdef WIN32
#include "win32/module_impl.hpp"
#else
#include "linux/module_impl.hpp"
#endif

namespace frosttools
{

class NullPluginBase {};
typedef NullPluginBase NullClass;
/// Base class for plugin
template <class _C> class _Plugin
{
public:
	/// Defines core manager type
	typedef _C Core;

	virtual ~_Plugin() {}

	/// Get plugin name
	virtual const char *getName()=0;
	/// Start plugin
	virtual void start(Core *manager)=0;
	/// Stop plugin
	virtual void stop()=0;
	/// Release plugin
	virtual void release()=0;
};

/// Base class for plugin manager
template<class _Base=NullClass>
class _PluginManager: public _Base
{
public:
	/// Defines base manager type
	typedef _Base Base;
	/// Defines own type
	typedef _PluginManager<Base> my_type;
	/// Defines plugin type
	typedef _Plugin<my_type> Plugin;
	/// Signature for plugin entry point
	typedef Plugin * (*LoadPluginFn)();
	/// Defines plugin container type
	typedef std::list<std::pair<SysModule*,Plugin*> > Plugins;

	/// Plugin is friendly, right?
	friend Plugin;

	/// Default constructor
	_PluginManager()
	{}

	/// Destructor
	virtual ~_PluginManager()
	{
		unloadPlugins();
	}

	/// Report error
	static void reportError(const char *error)
	{}

	/// Load plugin
	/// @param path path to dynamic library
	/// @param entry specify entry name
	virtual Plugin * loadLib(const char * path,const char *entry=NULL)
	{
		SysModule *module=NULL;
		Plugin *plugin=NULL;
		try
		{
			LoadPluginFn loadPlugin;
			module=new SysModule(path);
			module->getProcAddress(entry?entry:"createPlugin",loadPlugin);
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
	/// Unloads plugin
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
	/// Unload all plugins
	virtual void unloadPlugins()
	{
		while(!plugins.empty())
			unloadPlugin(plugins.front().second);
	}
protected:
	/// Find specified plugin
	typename Plugins::iterator _find(Plugin *plugin)
	{
		typename Plugins::iterator it=plugins.begin();
		for(;it!=plugins.end();it++)
			if(it->second==plugin)
				break;
		return it;
	}
	/// Find specified module
	typename Plugins::iterator _find(SysModule *module)
	{
		typename Plugins::iterator it=plugins.begin();
		for(;it!=plugins.end();it++)
			if(it->first==module)
				break;
		return it;
	}
	/// Container for plugins
	Plugins plugins;
};

}	// namespace frosttools
#endif
