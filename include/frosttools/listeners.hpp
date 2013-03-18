#ifndef FROSTTOOLS_LISTENERS
#define FROSTTOOLS_LISTENERS
#pragma once;
///////////////////////////////////////////////////////////////////////
// helper object, base to all listeners. Tracks target's changes
///////////////////////////////////////////////////////////////////////
template<class TargetType>
class Listener
{	
public:	
	virtual void _update(TargetType *ws)=0;	
	virtual void _attach(TargetType *ws)=0;	
	virtual void _detach(TargetType *ws)=0;	
	virtual void onUpdate(TargetType *ws)=0;
	virtual void onAttach(TargetType *ws)=0;	//called when listener is attached externally
	virtual void onDetach(TargetType *ws)=0;
};
///////////////////////////////////////////////////////////////////////
// helper object, is listening to a single target. Tracks target's changes
///////////////////////////////////////////////////////////////////////
template<class TargetType>
class ListenerSingle: public Listener<TargetType>
{
protected:
	TargetType *listenerTarget;
//	friend class Listening<TargetType>;
public:
	ListenerSingle(TargetType *t)
		:listenerTarget(t)
	{
		if(listenerTarget)
			listenerTarget->attach((Listener<TargetType>*)this);
	}
	~ListenerSingle()
	{
		if(listenerTarget)
			listenerTarget->detach((Listener<TargetType>*)this);
	}
	virtual void _update(TargetType *ws)
	{
		if(listenerTarget==ws)
			onUpdate(ws);
	}
	virtual void _attach(TargetType *ws)
	{
		if(listenerTarget==ws)
			onAttach(ws);
	}
	virtual void _detach(TargetType *ws)
	{
		if(listenerTarget==ws)
			onDetach(ws);
	}
	virtual void onUpdate(TargetType *ws){};
	virtual void onAttach(TargetType *ws){};	//called when listener is attached externally
	virtual void onDetach(TargetType *ws){};
};

///////////////////////////////////////////////////////////////////////
// helper object, is listening to a single target. Tracks target's changes
///////////////////////////////////////////////////////////////////////
template<class TargetType>
class ListenerMultiple: public Listener<TargetType>
{
protected:
	std::set<TargetType *> listenerTargets;
public:
	ListenerMultiple()		
	{}
	~ListenerMultiple()
	{
		std::set<TargetType*>::iterator it;
		for(it=listenerTargets.begin();it!=listenerTargets.end();it++)
			(*it)->detach((Listener<TargetType>*)this);
		listenerTargets.clear();
	}	
	virtual void _update(TargetType *ws)
	{
		//if(listnerTarget==ws)
			onUpdate(ws);
	}
	virtual void _attach(TargetType *ws)
	{
		listenerTargets.insert(ws);
		onAttach(ws);
	}
	virtual void _detach(TargetType *ws)
	{		
		listenerTargets.erase(ws);
			onDetach(ws);
	}
	virtual void onUpdate(TargetType *ws){};
	virtual void onAttach(TargetType *ws){};	//called when listener is attached externally
	virtual void onDetach(TargetType *ws){};
};
////////////////////////////////////////////////////////////////////////
// helper object, is being listened. 
////////////////////////////////////////////////////////////////////////
template <class TargetType>
class Listening
{
	std::set<Listener<TargetType>*> listeners;
public:
	~Listening()
	{
		std::set<Listener<TargetType>*>::iterator it;
		for(it=listeners.begin();it!=listeners.end();it++)
			(*it)->_detach((TargetType*)this);
		listeners.clear();
	}
	void attach(Listener<TargetType>* obj)
	{
		//std::cout<<"Some listener was attached\n";
		listeners.insert(obj);
		obj->_attach((TargetType*)this);
	}
	void detach(Listener<TargetType>* obj)
	{
		listeners.erase(obj);
	}
	void updateListeners()
	{
		std::set<Listener<TargetType>*>::iterator it;
		for(it=listeners.begin();it!=listeners.end();it++)
			(*it)->_update((TargetType*)this);
	}	
};

#endif