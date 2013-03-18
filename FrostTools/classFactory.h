#pragma once;
//#include "tinyXML.h"
//#include "dataStream.hpp"

#define registerEnum(c) registerEnumConstant(#c,c)

#define OFFSET(c) ((char*)(&(c))-(char*)this)

class IFactory;
class IFactoryManager;

enum ValueType
{
	valueTypeInt,
	valueTypeFloat,
	valueTypeDouble,
	valueTypeString,
	valueTypeObject
};

//class FactoryObject
//{
//protected:
//	//pointer to a definition
////	IFactory *definition;
//public:	
//	//FactoryObject(IFactory *def);//Basic constructor	
//	//~FactoryObject();
//	virtual void release()=0;	//if this object placed in extern DLL we need more accurate method to delete object	
//	IFactory * getDefinition() const;//get object's definition		
//	FactoryObject *clone() const;	//create a clone of this type
//};
//
//class IFactory
//{
//public:
//	virtual IFactoryManager *getFactory()=0;
//	virtual FactoryObject	*create()=0;
//	virtual void parseParameters(IFactoryManager *owner)=0;
//	//virtual ITemplate * createDefinition()=0;	
//
//	virtual void setType(const std::string &str)=0;
//	virtual void setClass(const std::string &str)=0;
//
//	virtual std::string getName()=0;
//	virtual std::string getType()=0;
//};
/*
template<class Type>
class Serialiser
{
public:
	template<class Type>
};*/
//class ObjectCreator: public IFactory
//{	
//	friend class ClassFactory;
//protected:
//	std::string typeName; //object name
//	std::string className;
//	IFactoryManager *factory;//factory
//	int objectsCreated;
//	int objectsDeleted;
//	//std::set<FactoryObject*> objects;
//public:	
//	ObjectCreator(IFactoryManager *fact,std::string type);
//	virtual ~ObjectCreator();
//	virtual IFactoryManager *getFactory()
//	{		
//		return factory;
//	}
//	virtual void setType(const std::string &str)
//	{
//		typeName=str;
//	}
//	virtual void setClass(const std::string &str)
//	{
//		className=str;
//	}
//	virtual std::string getName(){return className;};
//	virtual std::string getType(){return typeName;};
//	int getCreatedCount();
//	int getExistCount();
//	void notifyCreate(FactoryObject *object);	//called by FactoryObject constructor
//	void notifyRelease(FactoryObject *object);
//	virtual FactoryObject * create()=0;
//	virtual void parseParameters(ClassFactory *owner){};
//};
//struct ValueXML
//{
//	int type;
//	int offset;
//	std::string objType;
//	ValueXML(){};
//	ValueXML(const ValueXML &obj)
//		:type(obj.type),offset(obj.offset)
//	{}
//	ValueXML(int t,int o)
//		:type(t),offset(o)
//	{}
//};
//class ClassFactory;
//
//class DefinitionXML
//{
//public:
//	IFactory *def;
//	std::map<std::string,ValueXML> values;
//public:
//	DefinitionXML(IFactory *d);
//	~DefinitionXML();
//	
//	void parseParameters(ClassFactory *owner,IFactory*target);
//	void addFloat(const std::string &name,int offset);
//	void addString(const std::string &name,int offset);
//	void addInt(const std::string &name,int offset);
//	void addDouble(const std::string &name,int offset);
//	void addObject(const std::string &name,const std::string &type,int offset);
//};
//
//class IFactoryManager
//{
//public:	
//
//	virtual void registerTemplate(const std::string &name,IFactory *definition)=0;
//	virtual IFactory* registerType(const std::string &className,const char *definitionName)=0;
//	virtual void registerEnumConstant(char *name,int value)=0;
//	
//	virtual void loadDefinitions(const char *filename)=0;
//	virtual void saveDefinitions(const char *filename)=0;	
//
//	virtual FactoryObject * createObject(const std::string &name)=0;
//
//	virtual void propertyInt(const std::string &object,const std::string &name,int offset)=0;
//	virtual void propertyFloat(const std::string &object,const std::string &name,int offset)=0;
//	virtual void propertyString(const std::string &object,const std::string &name,int offset)=0;
//	virtual void propertyObject(const std::string &object,const std::string &name,const std::string &type,int offset)=0;
//
//	virtual int getLastError(){return 0;};
//	virtual const char * getErrorDesc(int err){return NULL;};
//};
//
//class ClassFactory: public IFactoryManager
//{
//protected:
//	virtual TiXmlElement * findProperty(const char *name);
//	IFactory * readElement_Object(TiXmlElement *);
//	void writeElement_Object(TiXmlElement *element,IFactory *object);
//
//
//	std::map<std::string,DefinitionXML*> templateLib;	
//
//	IFactory *currentObject;
//	std::string currentName;
//
//	std::map<std::string,IFactory*> types;
//	//std::map<std::string,ITemplate*> templates;	
//
//	std::map<std::string,int>	enumMap;
//
//	TiXmlDocument *doc;
//	TiXmlElement *currentElement;	
//public:
//	ClassFactory();
//	~ClassFactory();
//	virtual FactoryObject * createObject(const std::string &name);
//
//	virtual void registerTemplate(const std::string &name,IFactory *definition);
//	virtual IFactory* registerType(const std::string &className,const char *definitionName);
//	
//	virtual void loadDefinitions(const char *filename);
//	virtual void saveDefinitions(const char *filename);
//	IFactory*	getPropertyObject(const char *name,const std::string &type);
//
//	void propertyInt(const std::string &object,const std::string &name,int offset);
//	void propertyFloat(const std::string &object,const std::string &name,int offset);
//	void propertyString(const std::string &object,const std::string &name,int offset);
//	void propertyObject(const std::string &object,const std::string &name,const std::string &type,int offset);
//
//
//	virtual void registerEnumConstant(char *name,int value);
//	std::string getListValueString(const char *container); // <push>
//	float		getPropertyFloat(const char *name); /*ask float parameter*/
//	int			getPropertyInt(const char * name); /*ask integer parameter*/
//	std::string getPropertyString(const char *name);
//	void		getPropertyFloat3(const char *name,float *p);
//	void		getPropertyFloat2(const char *name,float *p);
//	int			getPropertyEnum(const char *name);
//};
//


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

template<class Base>
class SerialiserBase
{
public:
	virtual ~SerialiserBase(){};
	virtual bool read(DataStream *stream,Base *object)=0;
	virtual bool write(DataStream *stream,Base *object,const std::string &name)=0;

	static bool sread(SerialiserBase<Base> *serialiser,DataStream *stream,Base *object,const char* name)
	{
		if(serialiser)
			return serialiser->read(stream,object);
		else
			return false;
	}
	static bool swrite(SerialiserBase<Base> *serialiser,DataStream *stream,Base *object,const char* name)
	{
		if(serialiser)
			return serialiser->write(stream,object,name);
		return false;
	}
};


// base structure for any field pointers. Base - owner type.
template<class Base>
class FieldPtr
{
protected:
	std::string name;
public:
	FieldPtr(const char *n):name(n){}
	virtual ~FieldPtr(){};
	virtual bool read(DataStream *stream,Base *base)=0;	// reading field from stream
	virtual bool write(DataStream *stream,Base *base)=0;// storing field to stream
	void error(const char *msg,const char *from=NULL)
	{
		printf("Error in serialising: %s\n",msg);
	}
};
// 
template<class Base,class Type>
class FieldGetSetPtr: public FieldPtr<Base>
{
protected:
	typedef Type (Base::*FunctionGet)(void);
	typedef void (Base::*FunctionSet)(const Type &value);
	FunctionGet get;
	FunctionSet set;
	Type *def;
public:
	FieldGetSetPtr(const char *name,FunctionGet fnGet,FunctionSet fnSet,Type *_default)
		:get(fnGet),set(fnSet),FieldPtr<Base>(name),def(_default)
	{}
	virtual bool read(DataStream *stream,Base *base);	// reading field from stream
	virtual bool write(DataStream *stream,Base *base);	// storing field to stream
protected:
	virtual void setval(Base *base,const Type &val)
	{
		(base->*set)(val);
	}
	virtual Type getval(Base *base)
	{
		return (base->*get)();
	}
};

// base structure for typed field pointers. Base - owner type. Type - field type
template<class Base,class Type>
class TypedFieldPtr: public FieldPtr<Base>
{
protected:
	Type Base::* offset;
	Type *def;				// default value. Is it useless?	
	typedef Type value_type;
public:
	TypedFieldPtr(const char *name,Type Base::* _offset,Type *_def)
		:FieldPtr<Base>(name),offset(_offset),def(_def)
	{}
	const Type* pointer(const Base *base) const
	{
		return &(base->*offset);
	}
	Type* pointer(Base *base) const
	{
		return &(base->*offset);
	}
	const Type& reference(const Base *base) const
	{
		return base->*offset;
	}
	Type& reference(Base *base) const
	{
		return base->*offset;
	}
};

// simple field pointer. Base - owner type. Type - field type
template<class Base,class Type>
class GenericFieldPtr: public TypedFieldPtr<Base,Type>
{
public:
	GenericFieldPtr(const char *name,Type Base::* offset,Type *def)
		:TypedFieldPtr<Base,Type>(name,offset,def)
	{}
	~GenericFieldPtr()
	{
		std::cout<<"Generic field destructor"<<std::endl;
	}
	virtual bool read(DataStream *stream,Base *base)
	{		
		return stream->read(reference(base),name.c_str());
	}
	virtual bool write(DataStream *stream,Base *base)
	{
		stream->write(reference(base),name.c_str());
		return true;
	}
};
// pointer to a container field type. Base - field owner. Type - container field type (i.e std::list<int>)
// does not support complex values (i.e std::list<Point)
template<class Base,class Type>
class ContainerFieldPtr: public TypedFieldPtr<Base,Type>
{
public:
	typedef Type ContainerType;
	ContainerFieldPtr(const char *name,Type Base::* offset,Type *def)
		:TypedFieldPtr<Base,Type>(name,offset,def)
	{
		printf("Container field constructor\n");
	}
	virtual ~ContainerFieldPtr()
	{
		printf("Container field destructor\n");
	}	
	virtual bool read(DataStream *stream,Base *base);
	virtual bool write(DataStream *stream,Base *base);
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
template<class Base,class Type>
class ComplexFieldPtr: public TypedFieldPtr<Base,Type>
{
public:
	ComplexFieldPtr(const char *name,Type Base::* offset,Type *def)
		:TypedFieldPtr<Base,Type>(name,offset,def)
	{}
	virtual ~ComplexFieldPtr()
	{
		std::cout<<"Complex field destructor"<<std::endl;
	}	
	virtual bool read(DataStream *stream,Base *base);
	virtual bool write(DataStream *stream,Base *base);
};


// stores whole object. Contais field pointers
template <class Base>
class Serialiser : public SerialiserBase<Base>
{
	std::vector<FieldPtr<Base> *> fields;	
public:	
	typedef typename Base value_type;
	typedef typename std::vector<FieldPtr<Base> *>::iterator iterator;

	std::string type;		//general object type

	Serialiser()
	{
		getSerialiser<Base>((Serialiser<Base>*)this);
	}
	Serialiser(const char * _type):type(_type)
	{
		getSerialiser<Base>((Serialiser<Base>*)this);
	}
	~Serialiser()
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
	template<class Type> void value(const char *name,Type Base::* offset,Type *_default=NULL)
	{
		fields.push_back(new ComplexFieldPtr<Base,Type>(name,offset,_default));
	}
	// declare container type field
	template<class Type> void container(const char *name,Type Base::* offset,Type *_default=NULL)
	{
		fields.push_back(new ContainerFieldPtr<Base,Type>(name,offset,_default));
	}
	// declare get/set type field
	template<class Type> void value(const char *name,	typename FieldGetSetPtr<Base,Type>::FunctionGet get,
														typename FieldGetSetPtr<Base,Type>::FunctionSet set,
														Type *_default=NULL)
	{
		fields.push_back(new FieldGetSetPtr<Base,Type>(name,get,set,_default));
	}
	bool read(DataStream *stream,Base *object)
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
	bool write(DataStream *stream,Base *object,const std::string &name)
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
// GetSetFieldPtr
///////////////////////////////////////////////////////////////////////////////////////
template<class Base,class Type> bool FieldGetSetPtr<Base,Type>::read(DataStream *stream,Base *base)
{
	Type value;
	if(!set)
	{
		error("invalid set function","ComplexFieldPtr::readNode");
		return false;
	}
	SerialiserBase<Type> *serialiser=getSerialiser<Type>(NULL);
	if(!serialiser->sread(serialiser,stream,&value,name.c_str()))
	{
		error("stream reading error");
		return false;
	}
	setval(base,value);
}
template<class Base,class Type> bool FieldGetSetPtr<Base,Type>::write(DataStream *stream,Base *base)
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
template<class Base,class Type> bool ComplexFieldPtr<Base,Type>::read(DataStream *stream,Base *base)
{	
	return SerialiserBase<Type>::sread(getSerialiser<value_type>(NULL),stream,pointer(base),name.c_str());
}
template<class Base,class Type> bool ComplexFieldPtr<Base,Type>::write(DataStream *stream,Base *base)
{	
	return SerialiserBase<Type>::swrite(getSerialiser<value_type>(NULL),stream,pointer(base),name.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////
//ContainerFieldPtr
///////////////////////////////////////////////////////////////////////////////////////
template<class Base,class Type> bool ContainerFieldPtr<Base,Type>::read(DataStream *stream,Base *base)
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
template<class Base,class Type> bool ContainerFieldPtr<Base,Type>::write(DataStream *stream,Base *base)
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
///////////////////////////////////////////////////////////////////////////////////////
// primitive serialiser types

template <> class SerialiserBase<int>
{
public:
	typedef int Base;
	static bool sread(SerialiserBase<Base> *serialiser,DataStream *stream,Base *object,const char* name)
	{
		printf("reading %s\n",name);
		return stream->read(*object,name);
	}
	static bool swrite(SerialiserBase<Base> *serialiser,DataStream *stream,Base *object,const char* name)
	{
		printf("writing %s\n",name);
		stream->write(*object,name);
		return true;
	}
};

template <> class SerialiserBase<float>
{
public:
	typedef float Base;
	static bool sread(SerialiserBase<Base> *serialiser,DataStream *stream,Base *object,const char* name)
	{
		printf("reading %s\n",name);
		return stream->read(*object,name);
	}
	static bool swrite(SerialiserBase<Base> *serialiser,DataStream *stream,Base *object,const char* name)
	{
		printf("writing %s\n",name);
		stream->write(*object,name);
		return true;
	}
};