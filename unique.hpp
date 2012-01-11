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
#endif
