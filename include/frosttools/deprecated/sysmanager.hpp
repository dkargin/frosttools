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

//////////////////////////////////////////////////////////////////////////////
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

/*

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
*/
////////////////////////////////////////////////////////////////////


//#endif //WINDOWS
#endif //once
