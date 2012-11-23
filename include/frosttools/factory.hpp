#ifndef _FACTORY_HPP_
#define _FACTORY_HPP_

template<class Type>
class _Factory
{
public:
	typedef Type Object;
	virtual ~_Factory(){}
	virtual const TCHAR * getType()const=0;		// get type name
	virtual Object * create()=0;			// create new objects
	virtual void release()=0;					// factory and its objects
};

template<class _F>
class _FactoryManager
{
protected:
	typedef _F Factory;
	typedef typename Factory::Object Object;
	typedef std::basic_string<TCHAR> tstring;
	typedef std::map<tstring,Factory*> Factories;
	Factories factories;
public:
	virtual ~_FactoryManager()
	{
		for(typename Factories::iterator it=factories.begin();it!=factories.end();it++)
			it->second->release();
		factories.clear();
	}
	virtual void release()
	{
		delete this;
	}
	virtual int registerFactory(Factory *factory)
	{
		if(!factory)
			return 0;
		tstring name=factory->getType();
		typename Factories::iterator it=_find(factory);
		if(it!=factories.end())
			return 0;
		factories[name]=factory;
		return 1;
	}
	virtual int unregisterFactory(Factory *factory)
	{
		typename Factories::iterator it=_find(factory);
		if(it!=factories.end())
		{
			factories.erase(it);
			return 1;
		}
		return 0;
	}
	Object * create(const TCHAR *name)
	{
		Object *res=NULL;
		typename Factories::iterator it=_find(name);
		if(it!=factories.end())
			res=it->second->create();
		return res;
	}
protected:
	typename Factories::iterator _find(Factory *factory)
	{
		return factories.find(factory->getType());
	}
	typename Factories::iterator _find(const TCHAR *name)
	{
		return factories.find(name);
	}
};

#endif