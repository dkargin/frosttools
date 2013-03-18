#ifndef FROSTTOOLS_UNIQUE
#define FROSTTOOLS_UNIQUE
//////////////////////////////////////////////////////////////////////
// Unique ID generator
//////////////////////////////////////////////////////////////////////
#include <set>
#include <map>
//using namespace std;
template <class IDType=unsigned int>
class IDGenerator
{
	int idCounter;
	int idLast;
	typedef std::set<IDType> FreeIDSet;
	FreeIDSet freeIDset;
public:
	IDGenerator(void)
		:idCounter(0),idLast(0)
	{}
	~IDGenerator(void)
	{}

	IDType genID()
	{
		IDType res;
		if(freeIDset.empty())
		{
			idCounter++;
			res=idLast++;
		}
		else
		{
			typename FreeIDSet::iterator it=freeIDset.begin();
			res=(*it);
			freeIDset.erase(it);
		}
		return res;
	}
	void reset()
	{
		idCounter=0;
		idLast=0;
		freeIDset.clear();
	}
	/// mark id as already used
	/// tell that this id is already used (we got this ID externally, i.e through network synchronisation)
	void useID(IDType id)
	{

	}
	bool used(IDType id)
	{
		return freeIDset.find(id)==freeIDset.end() && id<idLast;
	}
	void freeID(IDType id)
	{
		//don't check, if id is valid
		if(id!=idLast-1)
			freeIDset.insert(id);
		else
			idLast--;
		idCounter--;
	}

	inline void write(std::ostream& _stream);
	inline void read(std::istream& stream);
};

template<class IDType>inline void IDGenerator<IDType>::write(std::ostream& stream)
{
    typename FreeIDSet::iterator it=freeIDset.begin();
    typename FreeIDSet::size_type size=freeIDset.size();
    stream<<size;
    for(;it!=freeIDset.end();it++)
    {
        IDType id=*it;
        stream<<id;
    }
    stream<<idLast;
    stream<<idCounter;
}

template<class IDType>inline void IDGenerator<IDType>::read(std::istream& stream)
{
    freeIDset.clear();
    typename FreeIDSet::size_type size;
    stream>>size;
    for(;size;size--)
    {
        IDType id;
        stream>>id;
        freeIDset.insert(id);
    }
    stream>>idLast;
    stream>>idCounter;
}
//////////////////////////////////////////////////////////////////////
// Unique id manager
// Convert unique object to unique id and vice versa
//////////////////////////////////////////////////////////////////////
template <	class InputType,
			class OutputType=unsigned int>
class IDManager
{
protected:
	std::map<InputType,OutputType> forward;
	IDGenerator<OutputType> idGenerator;
public:
	inline const OutputType & operator [] (const InputType &obj)
	{
		return getID(obj);
	}
	virtual const OutputType & getID(const InputType &obj)
	{
		typename std::map<InputType,OutputType>::iterator it=forward.find(obj);
		// 1. check if object exists
		if(it==forward.end())
		{
			// 2a. add new object
			OutputType res=idGenerator.genID();
			forward[obj]=res;
			return forward[obj];
		}
		return it->second;
	}
	bool containsObject(const InputType &obj)
	{
		return forward.find(obj)==forward.end();
	}
	virtual void erase(const InputType &obj)
	{
		typename std::map<InputType,OutputType>::iterator it=forward.find(obj);
		// 1. check if object exists
		assert(it!=forward.end());
		idGenerator.freeID(it->second);
		forward.erase(it);
	}
	inline void write(std::ostream &stream)
	{
		idGenerator.write(stream);
		typename std::map<InputType,OutputType>::iterator it=forward.begin();
		stream<<forward.size();
		for(;it!=forward.end();it++)
		{
			stream<<it->first<<std::endl<<it->second<<std::endl;
		}
		//		FILE *stream;
		/*
		   // Open output file for writing. Using _fsopen allows us to
		   // ensure that no one else writes to the file while we are
		   // writing to it.
			//
		   if( (stream = _fsopen( "outfile", "wt", _SH_DENYWR )) != NULL )
		   {
			  fprintf( stream, "No one else in the network can write "
							   "to this file until we are done.\n" );
			  fclose( stream );
		   }
		   // Now others can write to the file while we read it.
		   system( "type outfile" );
		*/
	}
	inline void read(std::istream &stream)
	{
		forward.clear();
		idGenerator.read(stream);
		typename std::map<InputType,OutputType>::size_type size;
		typename std::map<InputType,OutputType>::value_type val;

		stream>>size;

		for(;size;size--)
		{
			InputType obj;
			OutputType id;
			stream>>obj>>id;
			forward[obj]=id;
		}
	}
};
template <	class InputType,
			class OutputType=unsigned int>
class ReverseIDManager
{
protected:
	IDGenerator<OutputType> idGenerator;
	std::map<InputType,OutputType> forward;
	std::map<OutputType,InputType> backward;
public:
	inline const OutputType & operator [] (const InputType &obj)
	{
		return getID(obj);
	}
	inline const InputType & operator [] (const OutputType &obj)
	{
		return backward[obj];
	}
	virtual const OutputType & getID(const InputType &obj)
	{
		typename std::map<InputType,OutputType>::iterator it=forward.find(obj);
		// 1. check if object exists
		if(it==forward.end())
		{
			// 2a. add new object
			OutputType res=idGenerator.genID();
			forward[obj]=res;
			backward[res]=obj;
			return forward[obj];
		}
		return it->second;
	}
	virtual void erase(const InputType &obj)
	{
		typename std::map<InputType,OutputType>::iterator it=forward.find(obj);
		assert(it!=forward.end());
		// 1. check if object exists
		if(it!=forward.end())
		{
			OutputType out=it->second;
			InputType in=it->first;
			idGenerator.freeID(out);
			forward.erase(in);
			backward.erase(out);
		}
	}
	bool containsObject(const InputType &obj)
	{
		return forward.find(obj)!=forward.end();
	}
};
//////////////////////////////////////////////////////////////////////
// Unique name manager
//////////////////////////////////////////////////////////////////////
//template <class CharType=char>
class NameManager
{
	typedef char CharType;
	typedef std::basic_string<CharType, std::char_traits<CharType>, std::allocator<CharType> > StringType;
	typedef std::basic_stringstream<CharType, std::char_traits<CharType>, std::allocator<CharType> > StringStreamType;
	typedef std::map<StringType,int> MapType;
	MapType stringTable;
	CharType temp[255];
public:
	NameManager(void)
	{}
	~NameManager(void)
	{}
	StringType getName(const StringType &name)
	{
		StringStreamType stringStream;	//utility stream
		MapType::iterator it=stringTable.find(name);
		if(it==stringTable.end())
		{
			stringTable[name]=0;
			return name;
		}
		sprintf(temp,"%s%04d",name.c_str(),++stringTable[name]);
			//stringStream<<name<<++stringTable[name];
		return getName(temp);
	}
	int getNameCounter(const StringType &str)
	{
		MapType::iterator it=stringTable.find(str);
		if(it==stringTable.end())
			return 0;
		return it->second;
	}
	void reset()
	{
		stringTable.clear();
	}
};


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
////////////////////////////////////////////////////////////////////
/** 
	This object works in pair with IDStore<Target>
	Target - class name, derrived from IDStored<Target>
	this object has unique ID and iterator to its position in external container IDStore(std::map)
	automaticly removes its record in destructor and adds record in constructor
	also new ID is automacicly assigned in constructor
**/
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
#endif
