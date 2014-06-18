#ifndef FROSTTOOLS_SERIALISER
#define FROSTTOOLS_SERIALISER
#define FrostTools_Use_Serialiser

#include <assert.h>
class DataStream
{
public:
	virtual bool read(void *target,int size,const char *name)=0;
	virtual void write(const void *target,int size,const char *name)=0;
	virtual void write(const int &value,const char *name)=0;
	virtual void write(const float &value,const char *name)=0;
	virtual bool read(int &value,const char *name)=0;
	virtual bool read(float &value,const char *name)=0;
	virtual void beginObject(const char *type,const char *name)=0;
	virtual void endObject()=0;
};


template<class Target>	class SerialiserBase
{
public:
	typedef SerialiserBase<Target> my_type;
	virtual ~SerialiserBase(){};
	virtual bool read(DataStream *stream,Target *object)=0;
	virtual bool write(DataStream *stream,Target *object,const std::string &name)=0;

	static bool sread(my_type *serialiser,DataStream *stream,Target *object,const char* name)
	{
		if(serialiser)
			return serialiser->read(stream,object);
		else
			return false;
	}
	static bool swrite(my_type *serialiser,DataStream *stream,Target *object,const char* name)
	{
		if(serialiser)
			return serialiser->write(stream,object,name);
		return false;
	}
};

///////////////////////////////////////////////////////////////////////////////////////
// primitive serialiser types
template <> class SerialiserBase<int>
{
public:
	typedef int Target;
	static bool sread(SerialiserBase<Target> *serialiser,DataStream *stream,Target *object,const char* name)
	{
		printf("reading %s\n",name);
		return stream->read(*object,name);
	}
	static bool swrite(SerialiserBase<Target> *serialiser,DataStream *stream,Target *object,const char* name)
	{
		printf("writing %s\n",name);
		stream->write(*object,name);
		return true;
	}
};

template <> class SerialiserBase<float>
{
public:
	typedef float Target;
	static bool sread(SerialiserBase<Target> *serialiser,DataStream *stream,Target *object,const char* name)
	{
		printf("reading %s\n",name);
		return stream->read(*object,name);
	}
	static bool swrite(SerialiserBase<Target> *serialiser,DataStream *stream,Target *object,const char* name)
	{
		printf("writing %s\n",name);
		stream->write(*object,name);
		return true;
	}
};
namespace Serialiser
{
	
	namespace Field
	{
		// base structure for any field pointers. Base - owner type.
		template<class Target> class Base
		{
		protected:
			std::string name;
		public:
			Base(const char *n):name(n){}
			virtual ~Base(){};
			virtual bool read(DataStream *stream,Target *base)=0;	// reading field from stream
			virtual bool write(DataStream *stream,Target *base)=0;// storing field to stream
			void error(const char *msg,const char *from=NULL)
			{
				printf("Error in serialising: %s\n",msg);
			}
		};
		// 
		template<class Target,class Type> class GetSetC: public Field::Base<Target>
		{
		protected:
			typedef Type (Target::*FunctionGet)(void)const;
			typedef void (Target::*FunctionSet)(const Type &value);
			FunctionGet get;
			FunctionSet set;
			Type *def;

			virtual void setval(Target *base,const Type &val)
			{
				(base->*set)(val);
			}
			virtual Type getval(Target *base)
			{
				return (base->*get)();
			}
		public:
			GetSetC(const char *name,FunctionGet fnGet,FunctionSet fnSet,Type *_default)
				:get(fnGet),set(fnSet),Field::Base<Target>(name),def(_default)
			{}
			virtual bool read(DataStream *stream,Target *base);	// reading field from stream
			virtual bool write(DataStream *stream,Target *base);	// storing field to stream			
		};

		template<class Target,class Type> class GetSet: public Field::Base<Target>
		{
		protected:
			typedef Type (Target::*FunctionGet)(void);
			typedef void (Target::*FunctionSet)(Type &value);
			FunctionGet get;
			FunctionSet set;
			Type *def;

			virtual void setval(Target *base,Type &val)
			{
				(base->*set)(val);
			}
			virtual Type getval(Target *base)
			{
				return (base->*get)();
			}
		public:
			GetSet(const char *name,FunctionGet fnGet,FunctionSet fnSet,Type *_default)
				:get(fnGet),set(fnSet),Field::Base<Target>(name),def(_default)
			{}
			virtual bool read(DataStream *stream,Target *base);	// reading field from stream
			virtual bool write(DataStream *stream,Target *base);	// storing field to stream			
		};

		// base structure for typed field pointers. Base - owner type. Type - field type
		template<class Target,class Type>	class Typed: public Field::Base<Target>
		{
		protected:
			Type Target::* offset;
			Type *def;				// default value. Is it useless?	
			typedef Type value_type;
		public:
			Typed(const char *name,Type Target::* _offset,Type *_def)
				:Field::Base<Target>(name),offset(_offset),def(_def)
			{}
			const Type* pointer(const Target *base) const
			{
				return &(base->*offset);
			}
			Type* pointer(Target *base) const
			{
				return &(base->*offset);
			}
			const Type& reference(const Target *base) const
			{
				return base->*offset;
			}
			Type& reference(Target *base) const
			{
				return base->*offset;
			}
		};

		// simple field pointer. Base - owner type. Type - field type
		template<class Target,class Type>
		class Generic: public Typed<Target,Type>
		{
		public:
			Generic(const char *name,Type Target::* offset,Type *def)
				:Typed<Base,Type>(name,offset,def)
			{}
			~Generic()
			{
				std::cout<<"Generic field destructor"<<std::endl;
			}
			virtual bool read(DataStream *stream,Target *base)
			{		
				return stream->read(reference(base),name.c_str());
			}
			virtual bool write(DataStream *stream,Target *base)
			{
				stream->write(reference(base),name.c_str());
				return true;
			}
		};
		// pointer to a container field type. Base - field owner. Type - container field type (i.e std::list<int>)
		// does not support complex values (i.e std::list<Point)
		template<class Target,class Type>	class Container: public Typed<Target,Type>
		{
		public:
			typedef Type ContainerType;
			Container(const char *name,Type Target::* offset,Type *def)
				:Typed<Target,Type>(name,offset,def)
			{
				printf("Container field constructor\n");
			}
			virtual ~Container()
			{
				printf("Container field destructor\n");
			}	
			virtual bool read(DataStream *stream,Target *base);
			virtual bool write(DataStream *stream,Target *base);
		protected:
			// generic insertion for vector-based containers. 
			template<class T> static void addToContainer(std::vector<T> &container,const T &value)
			{
				container.push_back(value);
			}
			// generic insertion for list-based containers.
			template<class T> static void addToContainer(std::list<T> &container,const T &value)
			{
				container.push_back(value);
			}
			// generic insertion for tree-based containers. For map or multimap - insert a pair
			template<class Traits> static void addToContainer(std::_Tree<Traits> &container,const typename std::_Tree<Traits>::value_type &value)
			{
				container.insert(container.begin(),value);
			}	
		};
		// pointer to complex field type. Base - field owner. Type - field type
		template<class Target,class Type>
		class Complex: public Typed<Target,Type>
		{
		public:
			Complex(const char *name,Type Target::* offset,Type *def)
				:Typed<Target,Type>(name,offset,def)
			{}
			virtual ~Complex()
			{
				printf("Complex field destructor\n");
			}	
			virtual bool read(DataStream *stream,Target *base);
			virtual bool write(DataStream *stream,Target *base);
		};
	}

// stores whole object. Contais field pointers
template <class Target>
class Meta : public SerialiserBase<Target>
{
	typedef Field::Base<Target> FieldPtr;
	typedef std::vector<FieldPtr*> Fields;
	Fields fields;	
public:	
	typedef typename Target value_type;
	typedef typename Fields::iterator iterator;

	std::string type;		//general object type

	Meta()
	{
		getSerialiser<Target>(this);
	}
	Meta(const char * _type):type(_type)
	{
		getSerialiser<Target>(this);
	}
	~Meta()
	{
		iterator it=fields.begin();
		iterator e=fields.end();
		for(;it!=e;it++)
			delete (*it);
	}
	
	// declare simple type field
	//template<class Type> void value(const char *name,Type Base::* offset,Type *_default=NULL)
	//{
	//	fields.push_back(new GenericFieldPtr<Base,Type>(name,offset,_default));
	//}
	// declare complex type field (consists from other type fields)
	template<class Type> void value(const char *name,Type Target::* offset,Type *_default=NULL)
	{
		fields.push_back(new Field::Complex<Target,Type>(name,offset,_default));
	}
	// declare container type field
	template<class Type> void container(const char *name,Type Target::* offset,Type *_default=NULL)
	{
		fields.push_back(new Field::Container<Target,Type>(name,offset,_default));
	}
	// declare get/set type field
	template<class Type> void value(const char *name,	typename Field::GetSet<Target,Type>::FunctionGet get,
														typename Field::GetSet<Target,Type>::FunctionSet set,
														Type *_default=NULL)
	{
		fields.push_back(new Field::GetSet<Target,Type>(name,get,set,_default));
	}
	template<class Type> void value(const char *name,	typename Field::GetSetC<Target,Type>::FunctionGet get,
														typename Field::GetSetC<Target,Type>::FunctionSet set,
														Type *_default=NULL)
	{
		fields.push_back(new Field::GetSetC<Target,Type>(name,get,set,_default));
	}
	bool read(DataStream *stream,Target *object)
	{
		iterator it=fields.begin();
		iterator e=fields.end();
	//	try
		{
			for(;it!=e;it++)		
				(*it)->read(stream,object);
		}
	//	catch(...)
	//	{
	//		return false;
	//	}
		return true;
	}
	bool write(DataStream *stream,Target *object,const std::string &name)
	{
		try
		{
			stream->beginObject(type.c_str(),name.c_str());
			iterator it=fields.begin();
			iterator e=fields.end();
			for(;it!=e;it++)		
				(*it)->write(stream,object);		
			stream->endObject();
		}
		catch(...)
		{
			return false;
		}
		return true;
	}
};

// this function - is singleton holder for all serialisers. 
template< typename Type > inline SerialiserBase<Type> * getSerialiser(SerialiserBase<Type> *toRegister)	// serialiser registering method
{
	static SerialiserBase<Type> *serialiser=NULL;
	assert(serialiser==NULL || toRegister==NULL);
	if(toRegister!=NULL)
		serialiser=toRegister;
	
	return serialiser;
}
///////////////////////////////////////////////////////////////////////////////////////
// GetSetC
///////////////////////////////////////////////////////////////////////////////////////
template<class Target,class Type> bool Field::GetSetC<Target,Type>::read(DataStream *stream,Target *base)
{
	Type value;
	if(!set)
	{
		error("invalid set function","ComplexFieldPtr::readNode");
		return false;
	}
	if(!SerialiserBase<Type>::sread(getSerialiser<Type>(NULL),stream,&value,name.c_str()))
	{
		error("stream reading error");
		return false;
	}
	setval(base,value);
}
template<class Target,class Type> bool Field::GetSetC<Target,Type>::write(DataStream *stream,Target *base)
{	
	if(!get)
	{
		error("invalid get function","ComplexFieldPtr::readNode");
		return false;
	}
	Type value=getval(base);
	SerialiserBase<Type> *serialiser=getSerialiser<Type>(NULL);
	
	if(!SerialiserBase<Type>::swrite(getSerialiser<Type>(NULL),stream,&value,name.c_str()))
	{
		error("stream writing error");
		return false;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////
// GetSetFieldPtr
///////////////////////////////////////////////////////////////////////////////////////
template<class Target,class Type> bool Field::GetSet<Target,Type>::read(DataStream *stream,Target *base)
{
	Type value;
	if(!set)
	{
		error("invalid set function","ComplexFieldPtr::readNode");
		return false;
	}
	if(!SerialiserBase<Type>::sread(getSerialiser<Type>(NULL),stream,&value,name.c_str()))
	{
		error("stream reading error");
		return false;
	}
	setval(base,value);
}
template<class Target,class Type> bool Field::GetSet<Target,Type>::write(DataStream *stream,Target *base)
{	
	if(!get)
	{
		error("invalid get function","ComplexFieldPtr::readNode");
		return false;
	}
	Type value=getval(base);
	SerialiserBase<Type> *serialiser=getSerialiser<Type>(NULL);
	
	if(!SerialiserBase<Type>::swrite(getSerialiser<Type>(NULL),stream,&value,name.c_str()))
	{
		error("stream writing error");
		return false;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////
// ComplexFieldPtr
///////////////////////////////////////////////////////////////////////////////////////
template<class Target,class Type> bool Field::Complex<Target,Type>::read(DataStream *stream,Target *base)
{	
	return SerialiserBase<Type>::sread(getSerialiser<value_type>((SerialiserBase<Type>*)NULL),stream,pointer(base),name.c_str());
}
template<class Target,class Type> bool Field::Complex<Target,Type>::write(DataStream *stream,Target *base)
{	
	return SerialiserBase<Type>::swrite(getSerialiser<value_type>((SerialiserBase<Type>*)NULL),stream,pointer(base),name.c_str());
}


///////////////////////////////////////////////////////////////////////////////////////
//ContainerFieldPtr
///////////////////////////////////////////////////////////////////////////////////////
template<class Target,class Type> bool Field::Container<Target,Type>::read(DataStream *stream,Target *base)
{
	printf("Container field reading %s\n",name.c_str());
	ContainerType &container=reference(base);
	int size=0;
	container.clear();
	stream->read(size,(name+"-size").c_str());
	printf("- container size = %d\n",size);
	while(size--)
	{
		ContainerType::value_type value;
		stream->read(value,name.c_str());
		addToContainer(container,value);
	}
	return true;
}
template<class Target,class Type> bool Field::Container<Target,Type>::write(DataStream *stream,Target *base)
{
	printf("Container field writing %s\n",name.c_str());
	ContainerType &container=reference(base);
//	int o=(char*)&container-(char*)base;
	
	ContainerType::iterator it=container.begin();
	int size=(int)container.size();
	stream->write(size,(name+"-size").c_str());
	for(;it!=container.end();it++)
		stream->write((*it),name.c_str());		
	return true;
}


}

#endif