#include "stdafx.h"
#define FrostTools_Use_Unique
#include "../frostTools.h"
namespace Test
{

void testIDManagers()
{
	NameManager nameManager;
	IDGenerator<int> idGenerator;
	IDManager<std::string,int> idManager;

	std::string str1=nameManager.getName("name A");
	std::string str2=nameManager.getName("name A");
	std::string str3=nameManager.getName("name A");
	std::string str4=nameManager.getName("name A1");

	const int n=10;
	int id[n];
	int *ptr[n];
	
	for(int i=0;i<n;i++)
	{
		ptr[i]=new int;
		id[i]=idGenerator.genID();
	}

	for(int i=0;i<n;i++)
		delete ptr[i];

	idGenerator.freeID(4);
	int id1=idGenerator.genID();
	int id2=idGenerator.genID();

	int gamma=idManager.getID("gamma");
	int alpha=idManager.getID("alpha");
	int lambda=idManager.getID("lambda");
	int omega=idManager.getID("omega");
	int psi=idManager.getID("psi");
	int epsilon=idManager.getID("epsilon");
	int theta=idManager.getID("theta");

	int tmp;
	tmp=idManager.getID("psi");

	//FILE *out=fopen("manager.txt","wb");
	std::ofstream out("manager.txt");
	idManager.write(out);
	out.close();
	//fclose(out);


	//FILE *in=fopen("manager.txt","rb");
	std::ifstream in("manager.txt");
	IDManager<std::string,int> tmpManager;
	tmpManager.read(in);
	in.close();
	//fclose(in);

	tmp=tmpManager["omega"];
}
#ifdef NOMORE
template<class Storage,class Object> struct IDPair
{
	typedef IDPair<Storage,Object> pair_type;
	typedef unsigned int ID;
	class Store;
	class Stored;	
};

template<class Base,class Target>
class IDPair<Base,Target>::Stored
{
	friend class Base;
	friend class IDPair<Base,Target>::Store;
public:
	//typedef Target Object;		
	typedef typename Store::Objects Objects;
public:
	Stored(Base *owner)
		:store(owner),localID(invalidID)
	{
		if(store)store->add((Target*)this);
	}
	virtual ~Stored()
	{
		detach();
	}
	void detach()
	{
		if(store)
		{
			store->remove((Target*)this);
			store=NULL;
		}
	}
	typename Objects::iterator back;	// back link for fast remove
	Base * getStore()
	{
		return store;
	}
	ID id() const
	{
		return localID;
	}
protected:	
	Base *store;	
	ID localID;
};

template<class Base,class Target>
class IDPair<Base,Target>::Store
{
	friend class Target;
	friend class IDPair<Base,Target>::Stored;
public:
	typedef std::map<ID,Target*> Objects;
	typedef typename Objects::iterator iterator;
	typedef typename Objects::const_iterator const_iterator;
	//typedef IDStore<Target> my_type;
	Objects objects;

	class IDGen
	{
	public:
		virtual ID generate(Target *target)=0;
		virtual void free(ID id)=0;
	};
public:
	Store(IDGen *gen=NULL)
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
		Object *object=it->second;//objects[source];
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
		return contains(id)?objects[id]:NULL;
	}
protected:
	IDGen *generator;
	void setGenerator(IDGen *gen)
	{
		generator=gen;
	}		
	// called in Stored constructor
	// assignes new unique ID and back iterator
	virtual ID add(Target * object)
	{
		assert(generator);
		if(!valid(object->id()))	
			object->localID=generator->generate(object);
		else if(contains(object->id()))
		{
			assert(false);
		}
		objects.insert(std::make_pair(object->id(),object));	
		Objects::iterator it=objects.find(object->id());
		object->back=it;
		onAdd(object);		// raise onAdd event.
		return object->id();
	}
	// called in Stored destructor
	// removes the object from list. returns iterator to next valid object.
	virtual void remove(Target * object)
	{
		// some STL versions of remove return void instead 
		// of returning iterator to next object. So we 
		// need to get it manually
		onRemove(object);	// raise onRemove event
		generator->free(object->id());
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
			Target *obj=objects.begin()->second;
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

#endif

void testManagers()
{	
	/*
	struct MyObject;
	struct MyStorage: public IDPair<MyStorage,MyObject>::Store
	{
	}storage;
	struct MyObject: public IDPair<MyStorage,MyObject>::Stored
	{
		MyObject(MyStorage *manager,const char *n)
			:Stored(manager),hello(n)
		{}
		std::string hello;
	};
	new MyObject(&storage,"object1");
	new MyObject(&storage,"object2");
	new MyObject(&storage,"object3");
	new MyObject(&storage,"object4");*/
	TimeManager manager;
	struct TestEvent : public TimeManager::Action
	{
	public:
		std::string eventMsg;
		TestEvent(const std::string &str):eventMsg(str){}
		virtual void Execute()
		{
			printf("TestEvent::Execute(%s)\n",eventMsg.c_str());
		}
	};
	manager.add(new TestEvent("Event 1"), 40);
	manager.add(new TestEvent("Event 2"), 60);
	manager.add(new TestEvent("Event 3"), 20);
	manager.add(new TestEvent("Event 4"), 50);
	for(int i = 0; i < 8; i++)
		manager.update(5);

	manager.add(new TestEvent("Event 5"), 20);
	manager.add(new TestEvent("Event 6"), 30);	
	for(int i = 0; i < 30; i++)
		manager.update(5);
	int w = 0;
}
}