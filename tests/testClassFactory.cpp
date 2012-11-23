#include "stdafx.h"
//#include "classfactory.h"
#include "serialiser.hpp"

class DataStreamBinary: public DataStream
{
public:
	std::stringstream stream;
	virtual bool read(void *target,int size,const char *name)
	{
		stream.read((char*)target,size);
		return true;
	}
	virtual void write(const void *target,int size,const char *name)
	{
		stream.write((const char*)target,size);
	}
	void beginObject(const char *type,const char *name)
	{
	}
	void endObject()
	{
	}
	void write(const int &value,const char *name)
	{
		//printf("writing %s=%d",name,value);
		stream.write((char*)&value,sizeof(int));
	}
	bool read(int &value,const char *name)
	{
		stream.read((char*)&value,sizeof(int));
		return true;
	}
	void write(const float &value,const char *name)
	{
		//printf("writing %s=%f",name,value);
		stream.write((char*)&value,sizeof(float));
	}
	bool read(float &value,const char *name)
	{
		stream.read((char*)&value,sizeof(float));
		return true;
	}

};
//class DataStreamConsole: public DataStream
//{
//public:
//	std::stringstream stream;
//
//	void beginObject(const char *type,const char *name)
//	{
//	}
//	void endObject()
//	{
//	}
//	void write(const int &value,const char *name)
//	{
//		printf("writing %s=%d",name,value);
//		stream<<name<<" "<<value<<std::endl;
//	}
//	bool read(int &value,const char *name)
//	{
//		static char tmp[255];
//		stream>>tmp>>value;
//		printf("reading %s=%d as %s",tmp,value,name);
//		return true;
//	}
//	void write(const float &value,const char *name)
//	{
//		printf("writing %s=%f",name,value);
//		stream<<name<<" "<<value<<std::endl;
//	}
//	bool read(float &value,const char *name)
//	{
//		static char tmp[255];
//		stream>>tmp>>value;
//		printf("reading %s=%f as %s",tmp,value,name);
//		return true;
//	}
//	//template<class Type>
//	//void writeValue(const Type &value,const char *name)
//	//{
//	//	stream<<name<<" "<<value<<std::endl;
//	//}
//
//	//template<class Type>
//	//void readValue(Type &value,const char *name)
//	//{
//	//	static char tmp[255];
//	//	stream>>tmp>>value;
//	//	//stream.read(&valie,sizeof(Type));
//	//}
//};
//ClassFactory classFactory;
template<class Type>
struct ListSerialiser:public Serialiser::Meta<std::list<Type> >
{
	typedef std::list<Type> ListType;
	ListSerialiser():Serialiser("List")
	{
		container("list",(ListType ListType::*)NULL);
	}
};

struct Point
{
	float x,y;
	Point():x(0),y(0){}
	Point(float _x,float _y):x(_x),y(_y){}
	Point(const Point &p):x(p.x),y(p.y){}
	void print()
	{
		printf("(%f,%f)",x,y);
	}
};

template <> class SerialiserBase<Point>
{
public:
	typedef Point Target;
	static bool sread(SerialiserBase<Target> *serialiser,DataStream *stream,Target *object,const char* name)
	{
		printf("reading %s\n",name);
		return stream->read(object,sizeof(Target),name);		
	}
	static bool swrite(SerialiserBase<Target> *serialiser,DataStream *stream,Target *object,const char* name)
	{
		printf("writing %s\n",name);
		stream->write(object,sizeof(Target),name);
		return true;
	}
};
template <> class SerialiserBase<vec3>
{
public:
	typedef vec3 Target;
	static bool sread(SerialiserBase<Target> *serialiser,DataStream *stream,Target *object,const char* name)
	{
		printf("reading %s\n",name);
		return stream->read(object,sizeof(Target),name);		
	}
	static bool swrite(SerialiserBase<Target> *serialiser,DataStream *stream,Target *object,const char* name)
	{
		printf("writing %s\n",name);
		stream->write(object,sizeof(Target),name);
		return true;
	}
};
bool operator == (const Point &a,const Point &b)
{
	return a.x==b.x && a.y==b.y;
}
struct Rect
{
	vec3 a,b;
	int depth;
	std::list<float> floats;
	Rect()
		:a(1,2,3),b(4,5,9),depth(0)
	{
		floats.push_back(10);
		floats.push_back(40);
		floats.push_back(20);
		floats.push_back(60);
	}
	Rect(const vec3 &na,const vec3 &nb)
		:a(na),b(nb)
	{
		floats.push_back(1);
		floats.push_back(4);
		floats.push_back(2);
		floats.push_back(6);
	}
	void print()
	{
		//a.print();
		printf("-");
		//b.print();
		printf("-%d\n",depth);
	}
	void setDepth(const int &d)
	{
		depth=d;
	}
	int getDepth() const
	{
		return depth;
	}
	vec3 getC() const
	{
		return a;
	}
	void setC(const vec3 &p)
	{
		a=p;
	}
};

template<class Container>
struct ContainerHolder
{
	Container container;
};


Serialiser::Meta<Rect> rs("Rect");


void initSerialisers()
{
//	ps.value("x",&Point::x);
//	ps.value("y",&Point::y);

	rs.value<vec3>("a",&Rect::getC,&Rect::setC);
	rs.value("b",&Rect::b);
	rs.value("floats",&Rect::floats);
	rs.value<int>("depth",&Rect::getDepth,&Rect::setDepth);
}

//bool testSerialiserSimple()
//{
//	const int count=10;
//	std::vector<Point> source(count),target(count);
//	// generate random points
//	for(int i=0;i<source.size();i++)
//	{
//		source[i].x=(float)rand()/(float)RAND_MAX;
//		source[i].y=(float)rand()/(float)RAND_MAX;
//	}
//	DataStreamBinary stream;
//	// write to stream
//	for(int i=0;i<source.size();i++)
//	{
//		ps.write(&stream,&source[i],"point");
//	}	
//	// read from stream
//	for(int i=0;i<target.size();i++)
//	{
//		ps.read(&stream,&target[i]);
//	}	
//	// test
//	for(int i=0;i<source.size();i++)
//	{
//		if(!(source[i]==target[i]))
//			return false;
//	}	
//	return true;
//}
template<class Container>
bool testContainer()
{
	typedef ContainerHolder<Container> Base;
	Base source,target;
	// preparing serialiser
	Serialiser::Meta<Base> local;
	local.container("container",&Base::container);
	DataStreamBinary stream;
	const int count=10;
	// fill container
	for(int i=0;i<10;i++)
		source.container.push_back(rand());
	// write
	local.write(&stream,&source,"data");
	// read
	local.read(&stream,&target);
	if(source.container.size()!=target.container.size())
		return false;
	for(Container::iterator it1=source.container.begin(),it2=target.container.begin();
		it1!=source.container.end()&& it2!=target.container.end();it1++,it2++)
	{
		if(*it1!=*it2)
			return false;
	}
	return true;
}

void testClassFactory()
{	
	initSerialisers();
	//printf("Test simple fields:%s\n",testSerialiserSimple()?"-success":"-fail");
	printf("Test list container with int:%s\n",testContainer<std::list<int> >()?"-success":"-fail");
	printf("Test list container with float:%s\n",testContainer<std::list<float> >()?"-success":"-fail");
	printf("Test vector container with int:%s\n",testContainer<std::vector<int> >()?"-success":"-fail");
	printf("Test vector container with float:%s\n",testContainer<std::vector<float> >()?"-success":"-fail");

	
	Point point1;
	Rect rect1(vec3(0,1,1),vec3(10,14,3)),rect2;
	
	DataStreamBinary stream;	
	//ps.write(&stream,&point1,"point1");
	rs.write(&stream,&rect1,"rect1");
	DataStreamBinary stream2;
	stream2.stream.str(stream.stream.str());
	rs.read(&stream2,&rect2);
	rect2.print();
	//std::cout<<stream.stream.str();
	system("pause");
}


/*
class Engine;
class EngineDef;
class Turret;
class TurretDef;
class Unit;
class UnitDef;

class Engine: public FactoryObject
{
public:
	Engine(EngineDef *def)
		:FactoryObject((IObjectCreator*)def)
	{}
	void release()
	{
		delete this;
	}
};
class EngineDef: public IObjectCreator
{
public:
	EngineDef(ClassFactory *fact)		
	{
		ClassFactory *f=(ClassFactory*)fact;
		f->propertyFloat("Engine","maxPower",		OFFSET(maxPower));
		f->propertyFloat("Engine","maxAcseleration",OFFSET(maxAcseleration));
		f->propertyFloat("Engine","maxSpeed",		OFFSET(maxSpeed));
	}
	FactoryObject	*create()
	{
		return new Engine(this);
	}
	void parseParameters(ClassFactory *owner)
	{}

	float maxPower;
	float maxAcseleration;
	float maxSpeed;
	class EngineSerialiser: public Serialiser<EngineDef>
	{
	public:
		EngineSerialiser()
		{
			value("maxPower"		,&EngineDef::maxPower);
			value("maxAcseleration"	,&EngineDef::maxAcseleration);
			value("maxSpeed"		,&EngineDef::maxSpeed);
		}
	};
};
class Turret: public FactoryObject
{
public:
	Turret(TurretDef *def)
		:FactoryObject(def)
	{}
	void release()
	{
		delete this;
	}
};
class TurretDef: public IObjectCreator
{
public:
	TurretDef(ClassFactory *fact)
		//:ObjectDef(fact,"Turret")
	{		
		fact->propertyFloat("Turret","turnRateHor",OFFSET(turnRateHor));
		fact->propertyFloat("Turret","turnRateVer",OFFSET(turnRateVer));
	}
	FactoryObject	*create()
	{
		return new Turret(this);
	}
	void parseParameters(ClassFactory *owner)
	{}

	float turnRateHor;
	float turnRateVer;
};
class Unit: public FactoryObject
{
public:
	Unit(UnitDef *def)
		:FactoryObject((IObjectCreator*)def)
	{}
	void release()
	{
		delete this;
	}
};

class UnitDef: public IObjectCreator
{
public:
	UnitDef(ClassFactory *fact)
		//:ObjectDef(fact,"Unit")
	{
		//ClassFactory *f=(ClassFactory*)fact;
		fact->propertyObject("Unit","engine","Engine",OFFSET(engineDef));
		fact->propertyObject("Unit","turret","Turret",OFFSET(turretDef));
	}
	FactoryObject	*create()
	{
		return new Unit(this);
	}
	void parseParameters(ClassFactory *owner)
	{}
	
	EngineDef *engineDef;
	TurretDef *turretDef;	
};
*/

/*
class MyDataStream: public DataStream,public std::fstream
{
public:
	virtual	char		readByte()
	{
		char res;
		read(&res,1);
		return res;
	}
	virtual	short		readWord()
	{
		short res;
		read((char*)&res,sizeof(short));
		return res;
	}
	virtual	int			readDword()
	{
		int res;
		read((char*)&res,sizeof(int));
		return res;
	}
	virtual	float		readFloat()
	{
		float res;
		read((char*)&res,sizeof(float));
		return res;
	}
	virtual	double		readDouble()
	{
		double res;
		read((char*)&res,sizeof(double));
		return res;
	}
	virtual	void readBuffer(void* buffer, int size)
	{		
		read((char*)buffer,size);		
	}
	virtual	DataStream&	storeByte(char b)
	{
		write(&b,sizeof(char));
	}
	virtual	DataStream&	storeWord(short w)
	{
		write((char*)&w,sizeof(short));
	}
	virtual	DataStream&	storeDword(int d)
	{
		write((char*)&d,sizeof(int));
	}
	virtual	DataStream&	storeFloat(float f)
	{
		write((char*)&f,sizeof(float));
	}
	virtual	DataStream&	storeDouble(double f)
	{
		write((char*)&f,sizeof(double));
	}
	virtual	DataStream&	storeBuffer(const void* buffer, int size)
	{
		write((char*)buffer,size);
	}
};*/