#ifndef _PLUGIN_HPP_
#define _PLUGIN_HPP_

#include "module.hpp"

template <class _C> class _Plugin
{
public:
	typedef _C Core;
	virtual const char *getName()=0;
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
	typedef Plugin * (*LoadPluginFn)();
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