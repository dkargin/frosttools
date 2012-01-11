#ifndef FROSTTOOLS_SYSMANAGER
#define FROSTTOOLS_SYSMANAGER
#define _CRT_SECURE_NO_DEPRECATE		// no CRT deprecation warnings
#define FrostTools_Use_System

#include "autoptr.hpp"
#include <stdarg.h>
#include <windows.h>
#include <map>
#include <string>
enum SystemState
{
	systemStateOff,
	systemStateInit,
	systemStatePause,
	systemStateRun,
	systemStateShutdown
};

class ISystem;
template<class Node,class Message> class System;
//class SystemNode;
template <class _C> class _Plugin
{
public:
	typedef _C Core;
	virtual const TCHAR *getName()=0;
	virtual void start(Core *manager)=0;
	virtual void stop()=0;
	virtual void release()=0;
};


class ISystem
{
	FILE *logger;
	bool externLogger;
public:
	ISystem()
		:logger(NULL),externLogger(false)
	{}
	virtual ~ISystem()
	{
		closeLogger();
	}
	virtual void closeLogger()
	{
		if(logger &&!externLogger)
			fclose(logger);
	}
	virtual void logPrint(const char *format, ...)
	{
		if(logger&&format)
		{
			va_list	ap;
			va_start(ap,format);
			vfprintf(logger,format,ap);
			va_end(ap);
		}
	}
	virtual void setLogOutput(const char *path)
	{
		closeLogger();
		externLogger=false;
		logger=fopen(path,"w");
	}
	virtual int getState()=0;
	virtual int initSystem()=0;
	virtual void resetSystem()=0;
	virtual void update(float dT)=0;
	virtual void release()=0;
};
//////////////////////////////////////////////////////////////
/// Base class for everything that can get messages
//////////////////////////////////////////////////////////////
template<class _Msg>
class MessageTarget
{
public:
	typedef _Msg message_type;
	typedef MessageTarget<_Msg> my_type;
protected:
	_Msg	*messagesQueue;		//message queue, projected in the array
	int	messagesMax;	//maximum number of messages in queue
	int	messagesCount;	//current number of messages
	int head;			//index of the head in queue
	int tail;			//index of the tail in queue
	bool messagerLocked;
public:
	MessageTarget()
	{
		head=tail=-1;
		messagesCount=0;
		messagesQueue=NULL;
		messagesMax=0;
		messagerLocked=false;
	}
	~MessageTarget()
	{
		if(messagesQueue)delete messagesQueue;
	}

	void initMessageQueue(int max)
	{
		messagesMax=max;
		messagesQueue.resize(max);
	}

	inline void inc(int &i,int max)
	{
		i++;
		if(i==max)i=0;
	}

	int sendMessage(const _Msg &message)
	{//message queue. to be implemented properly
	/*	if(head==-1)//the queue is empty
		{
			head=0;
			tail=0;
			memcpy(message_queue+tail,&message,sizeof(Message));
			numMsg++;
			return 1;

		}
		if(numMsg==MAX_MESSAGES)return 0;

		inc(tail,MAX_MESSAGES);
		memcpy(message_queue+tail,&message,sizeof(Message));
		numMsg++;

		return 1;*/ //MUST BE CORRECTLY IMPLEMENTED!!!
		if(!messagerLocked)
			return messageProc(message);
		return NULL;
	}

	bool isMessagerLocked()
	{
		return messagerLocked;
	}

	void lockMessager()
	{
		messagerLocked=true;
	}

	void unlockMessager()
	{
		messagerLocked=false;
	}

	int popMessage(_Msg &message)
	{
		UINT tmp=head;
		if(messagesCount==0)return-1;
		inc(head,this->messagesMax);
		memcpy(&message,messagesQueue+head,sizeof(_Msg));
		messagesCount--;
		return 0;
	}

	virtual int process(float fTime)
	{
		return 0;
	}

	virtual int messageProc(const _Msg &message)
	{
		return -1;
	}
};

template<class Node,class Message>
class System: public ISystem
{
public:
	typedef MessageTarget<Message> MessagerType;
protected:
	//bool bActive;
	typedef std::list<Node *> NodeContainer;
	NodeContainer nodeLib;


	MessagerType		* msgTarget;

	//IClassFactory * factory;
	int				systemState;
	int setSystemState(int state)
	{
		return systemState=state;
	}
public:
	System()
	{
		systemState=systemStateOff;
	}
	//System(IClassFactory *factory);
	//virtual void attach(SystemNode *node);
	//virtual SystemNode *createObject(const std::string &name,Object *owner)=0;
	//virtual IClassFactory * getFactory();
	//virtual SystemNode * getNode(Object *object);

	virtual void setTarget(MessagerType *t)
	{
		msgTarget=t;
	}
	virtual MessagerType * getTarget()
	{
		return msgTarget;
	}
	virtual int getState()
	{
		return systemState;
	}
	virtual int initSystem()=0;		// return 1 if sucsess, 0 otherwise.
	virtual void resetSystem()=0;

	virtual void update(float dT)=0;
	virtual void release()=0;
};
// something stored in the std::list with back links for fast remove
template<class Target> class ListStore;
template<class Target> class ListStored;
//
//////////////////////////////////////////////////////////////////////////////
#ifdef FrostTools_Use_Threads
class Lockable
{
	CRITICAL_SECTION cs; 
public:
	class Scoped
	{
		Lockable &parent;
	public:
		Scoped(Lockable &p):parent(p)
		{
			parent.lock();
		}
		~Scoped()
		{
			parent.unlock();
		}
	};
	friend class Scoped;

	void lock()
	{		
		EnterCriticalSection(&cs); 
	}

	void unlock()
	{
		LeaveCriticalSection(&cs);
	}

	Lockable()
	{
		InitializeCriticalSection(&cs);
	}

	virtual ~Lockable()
	{
		DeleteCriticalSection(&cs); 
	}

	bool locked() const
	{
		if(TryEnterCriticalSection(const_cast<CRITICAL_SECTION*>(&cs)))
		{
			LeaveCriticalSection(const_cast<CRITICAL_SECTION*>(&cs));
			return false;
		}
		return true;
	}
};

template<class Type> class SharedValue: public Lockable
{
	Type value;
public:
	explicit SharedValue(const Type & value)
	{
		lock();
		this->value = value;
		unlock();
	}
	~SharedValue()
	{
		if(locked())
			throw(std::exception("~SharedValue locked"));
		Scoped s(*this);
	}
	operator Type()
	{
		lock();
		Type result = value;
		unlock();
		return value;
	}
	SharedValue & operator = (const Type & v)
	{
		Scoped s(*this);
		value = v;
		return *this;
	}
};

#endif
//////////////////////////////////////////////////////////////////////////////
// Base class for objects, stored in external list. Holds iterator to its position in that store
//////////////////////////////////////////////////////////////////////////////
template<class Target>
class ListStored
{
public:
	typedef Target Object;
	typedef ListStore<Target> Store;
	typedef typename Store::Objects Objects;
	friend class ListStore<Target>;	
	typename Objects::iterator back;	// back link for fast remove

	ListStored(ListStore<Target> *owner);
	virtual ~ListStored();	
	Store * getStore();
private:
	Store *store;	
};

template<class Target> FrostTools_Impl ListStored<Target>::ListStored(ListStore<Target> *owner)
:store(owner)
{
	store->add(this);
}
template<class T> FrostTools_Impl ListStored<T>::~ListStored()
{
	store->remove(this);
	store=NULL;
}

template<class T> FrostTools_Impl typename ListStored<T>::Store * ListStored<T>::getStore()
{
	return store;
}

//////////////////////////////////////////////////////////////////////////////
// Base class for objects, storing ListStored<> based objects.
//////////////////////////////////////////////////////////////////////////////
template<class _T> 
class ListStore
{
public:
	typedef _T Target;
	friend class ListStored<Target>;
	typedef std::list<Target * > Objects;
	typedef typename Objects::iterator iterator;
	typedef typename Objects::const_iterator const_iterator;	

	const Objects & getObjects() const;
	const_iterator begin() const;
	const_iterator end() const;
protected:
	iterator begin();
	iterator end();	
	virtual Target * getTarget(ListStored<Target> * object);
	virtual typename Objects::iterator add(ListStored<Target> * object);
	// removes the object from list. returns iterator to next valid object.
	virtual typename Objects::iterator remove(ListStored<Target> * object);
	// remove all contained objects. 
	virtual void clear();
	virtual void onRemove(Target * object){}
	virtual void onAdd(Target *object){}

	Objects objects;
};

template<class T> const typename ListStore<T>::Objects & ListStore<T>::getObjects() const
{
	return objects;
}
template<class T> typename ListStore<T>::const_iterator ListStore<T>::begin() const
{
	return objects.begin();
}
template<class T> typename ListStore<T>::const_iterator ListStore<T>::end() const
{
	return objects.end();
}
template<class T> typename ListStore<T>::iterator ListStore<T>::begin()
{
	return objects.begin();
}
template<class T> typename ListStore<T>::iterator ListStore<T>::end()
{
	return objects.end();
}
template<class T> typename ListStore<T>::Target * ListStore<T>::getTarget(ListStored<Target> * object)
{
	//return dynamic_cast<Target*>(object);
	return (Target * )object;
}
template<class T> typename ListStore<T>::Objects::iterator ListStore<T>::add(ListStored<Target> * object)
{
	objects.push_front(getTarget(object));
	object->back=objects.begin();
	onAdd((Target*)object);		// raise onAdd event
	return object->back;
}
// removes the object from list. returns iterator to next valid object.
template<class T> typename ListStore<T>::Objects::iterator ListStore<T>::remove(ListStored<Target> * object)
{
	// some STL versions of remove return void instead 
	// of returning iterator to next object. So we 
	// need to get it manually
	Objects::iterator res=object->back;
	res++;
	onRemove((Target*)object);	// raise onRemove event
	objects.erase(object->back);	
	return res;
}
// remove all contained objects. 
template<class T> void ListStore<T>::clear()
{
	while(!objects.empty())
		delete objects.front();
}

template<template<class> class Store,class Obj,class Def>
class ManagerBase: public Store<Obj>,public Store<Def>
{
public:
	typedef Obj Object;
	typedef Def Definition;
	typedef Store<Obj> Objects;
	typedef Store<Def> Definitions;
};


template<typename  _Obj,typename _Def>
class _Manager
{
public:
	typedef _Obj object_type;
	typedef _Def definition_type;
	typedef _Manager<object_type,definition_type> my_type;
//	template<class Obj,typename Def> friend class _Manager<Obj,Def>::object_type;
//	template<class Obj,typename Def> friend class _Manager<Obj,Def>::definition_type;
	friend typename _Obj;
	friend typename _Def;

	typedef std::map<std::string,definition_type*> Definitions;
	typedef std::list<object_type* > Objects;
	Objects objects;
	Definitions definitions;
protected:
	virtual definition_type *newDefinition()=0;
	virtual object_type *newObject(definition_type *def)=0;
public:
	_Manager()
	{}
	virtual ~_Manager()
	{
		clear();
	}
	void clear()
	{
		for(typename Objects::iterator it=objects.begin();it!=objects.end();it++)
			delete (*it);
		objects.clear();
		for(typename Definitions::iterator it=definitions.begin();it!=definitions.end();it++)
			delete (it->second);
		definitions.clear();
	}
	definition_type * createDefinition(const char *name)
	{
		definition_type *res=NULL;
		if(name==NULL)
			return NULL;
		if(definitions.find(name)==definitions.end())
		{
			res=newDefinition();
			definitions.insert(std::make_pair(name,res));
			//definitions[name].attach(res);
		}
		return res;
	}
	definition_type * getDefinition(const char *name)
	{
		typename Definitions::iterator it=definitions.find(name);
		if(it!=definitions.end())
			return it->second;
		return NULL;
	}
	object_type * createObject(const char *def)
	{
		typename Definitions::iterator it=definitions.find(def);
		return it!=definitions.end()?createObject(it->second):NULL;
	}
	object_type * createObject(definition_type *def)
	{
		object_type *res=newObject(def);
		objects.push_back(res);
		//objects.push_back(AutoDeleter<object_type>());
		//objects.back().attach(res);
		return res;
	}
	virtual void release()
	{
		delete this;
	}
	virtual void destroyObject(object_type *object)
	{
		for(typename Objects::iterator it=objects.begin();it!=objects.end();it++)
		{
			if((*it)==object)
			{
				delete (*it);
				objects.erase(it);
				break;
			}
		}
	}
};
//#ifdef _WINDOWS_
#include <windows.h>
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


struct Buffer
{
	void *data;
	int length;

	Buffer():data(0),length(0){}
	Buffer(void *d,int l):data(d),length(l){}
	Buffer(const Buffer &buffer):data(buffer.data),length(buffer.length){}

	inline Buffer offset(int offs) const
	{
		return (offs>length)?Buffer((char*)data+offs,length-offs):Buffer();
	}
	inline bool valid()
	{
		return length>0 && data!=NULL;
	}

	static Buffer create(int size);
	static void release(Buffer &buffer);
	static Buffer merge(const std::list<Buffer> &container);
};

FrostTools_Impl Buffer Buffer::create(int size)
{
	return Buffer(new char[size],size);
}
FrostTools_Impl void Buffer::release(Buffer &buffer)
{
	delete []buffer.data;
}
FrostTools_Impl Buffer Buffer::merge(const std::list<Buffer> &container)
{
	typedef std::list<Buffer> Container;
	// 1. calculate total size
	int size=0;
	for(Container::const_iterator i=container.begin();i!=container.end();i++)
		size+=i->length;

	Buffer result=Buffer::create(size);
	char *ptr=(char*)result.data;
	for(Container::const_iterator i=container.begin();i!=container.end();i++)
	{
		memcpy(ptr,i->data,i->length);
		ptr+=i->length;
	}
	return result;
}
////////////////////////////////////////////////////////////////////
/** 
	This object works in pair with IDStore<Target>
	Target - class name, derrived from IDStored<Target>
	this object has unique ID and iterator to its position in external container IDStore(std::map)
	automaticly removes its record in destructor and adds record in constructor
	also new ID is automacicly assigned in constructor
**/
////////////////////////////////////////////////////////////////////
#include "unique.hpp"
template<class Target> class IDStore;
typedef unsigned int ID;
const ID invalidID=-1;
inline bool valid(const ID &id)
{
	return id!=invalidID;
}
template<class Target>
class IDStored
{
	friend class IDStore<Target>;
public:
	typedef Target Object;
	typedef IDStore<Target> Store;
	typedef typename Store::Objects Objects;
public:
	IDStored(IDStore<Target> *owner)
		:store(owner),localID(invalidID)
	{
		if(store)
			store->add(this);
	}

	virtual ~IDStored()
	{
		detach();
	}
	void detach()
	{
		if(store)
		{
			store->remove(this);
			store=NULL;
		}
	}
	typename Objects::iterator back;	// back link for fast remove
	Store * getStore()
	{
		return store;
	}
	ID id() const
	{
		return localID;
	}
protected:	
	Store *store;	
	ID localID;
};
//#include "delegate.hpp"

template<class Target> class IDStore
{	
public:	
	friend class IDStored<Target>;
	typedef IDStored<Target> Stored;
	typedef std::map<ID,Stored * > Objects;
	typedef typename Objects::iterator iterator;
	typedef typename Objects::const_iterator const_iterator;
	typedef IDStore<Target> my_type;
	Objects objects;

	class IDGen
	{
	public:
		virtual ID generate(Target *target)=0;
		virtual void release(ID id)=0;
	};
public:
	IDStore(IDGen *gen=NULL)
		:generator(gen)
	{}	
	const Objects & getObjects() const
	{
		return objects;
	}
	const_iterator begin() const
	{
		return objects.begin();
	}
	const_iterator end() const
	{
		return objects.end();
	}
	void destroy(ID id)
	{
		Objects::iterator it=objects.find(id);
		if(it!=objects.end())
			delete it->second;
	}
	bool contains(ID id)
	{
		return objects.find(id)!=objects.end();
	}
	iterator begin()
	{
		return objects.begin();
	}
	iterator end()
	{
		return objects.end();
	}
	Target * operator[](ID id)
	{
		if(contains(id))
			return getTarget(objects[id]);
		else
			return NULL;
	}
	bool remap(ID source,ID target)	// change ID from source to target
	{
		// source->target
		// target->new id
		if(!contains(source))
			return false;	// remap failed

		// find object
		Objects::iterator it=objects.find(source);		
		// get pointer
		Stored *object=it->second;//objects[source];
		// remove
		objects.erase(it);
		
		if(contains(target))	// if <target> is valid object -> remap it to new ID
			remap(target,generator->generate(getTarget(objects[target])));

		objects[target]=object;
		// set new ID
		object->localID=target;		
		object->back=objects.find(target);
		return true;
	}
	virtual Target * get(ID id)
	{
		return contains(id)?(Target * )objects[id]:NULL;
	}
protected:
	IDGen *generator;
	void setGenerator(IDGen *gen)
	{
		generator=gen;
	}
	//ID genID(Target * t)
	//{
	//	//if(!NotNullArg<Arg1>::value)
	//	//{
	//	typedef ID ( X::*Mfn)(Target *);
	//	return (base->*reinterpret_cast<Mfn>(_genID))(t);
	//	//}
	//}
	//void freeID(ID id)
	//{
	//	typedef void ( X::*Mfn)(ID);
	//	(base->*reinterpret_cast<Mfn>(_freeID))(id);
	//}
	// convert from  Stored * to Target *
	virtual Target * getTarget(Stored * object)
	{
		return (Target * )object;
	}
	
	// called in Stored constructor
	// assignes new unique ID and back iterator
	virtual ID add(Stored * object)
	{
		assert(generator);
		if(!valid(object->id()))	
			object->localID=generator->generate(getTarget(object));
		else if(contains(object->id()))
		{
			assert(false);
		}
		objects.insert(std::make_pair(object->id(),object));	
		Objects::iterator it=objects.find(object->id());
		object->back=it;
		onAdd((Target*)object);		// raise onAdd event.
		return object->id();
	}
	// called in Stored destructor
	// removes the object from list. returns iterator to next valid object.
	virtual void remove(Stored * object)
	{
		// some STL versions of remove return void instead 
		// of returning iterator to next object. So we 
		// need to get it manually
		onRemove((Target*)object);	// raise onRemove event
		generator->release(object->id());
		object->localID=-1;
		//idGenerator.freeID(object->id());
		
		objects.erase(object->back);	
	}
	// remove all contained objects. 
	virtual void clear()
	{
		while(!objects.empty())
		{
#ifdef _DEBUG
			Stored *obj=objects.begin()->second;
			delete obj;
#else
			delete objects.begin()->second;
#endif
		}
		assert(objects.empty());
	}
	// called everytime when Stored object is removerd
	virtual void onRemove(Target * object){}
	// called everytime when Stored object is added
	virtual void onAdd(Target *object){}
};

template<class Target> class BasicIDStore: public IDStore<Target>
{
	class BasicIDGen: public IDStore<Target>::IDGen
	{
		ID lastID;
	public:
		BasicIDGen():lastID(0){}
		virtual ID generate(Target *target)
		{
			return lastID++;
		}
		virtual void release(ID id){}
	}idGen;
public:
	BasicIDStore():IDStore<Target>(&idGen){}
};
//#endif //WINDOWS
#endif //once
