
template<class Type>
struct CompareLess: public std::binary_function<Type,Type,bool>
{
	inline bool operator()(const Type& left, const Type& right) const
	{	// apply operator< to operands
		return (left) < (right);
	}
};
template<class Type>
struct CompareGreater: public std::binary_function<Type,Type,bool>
{
	inline bool operator()(const Type& left, const Type& right) const
	{	// apply operator< to operands
		return (left) > (right);
	}
};
template<class Type>
struct CompareLessEqual: public std::binary_function<Type,Type,bool>
{
	inline bool operator()(const Type& left, const Type& right) const
	{	// apply operator< to operands
		return (left) <= (right);
	}
};
template<class Type>
struct CompareGreaterEqual: public std::binary_function<Type,Type,bool>
{
	inline bool operator()(const Type& left, const Type& right) const
	{	// apply operator< to operands
		return (left) >= (right);
	}
};

//////////////////////////////////////////////////////////////////////
// Distance table helper
//////////////////////////////////////////////////////////////////////
template<class Real,int N,class Coord=int>
class DistanceTable
{
	static const int size=N;
	Real distanceTable[size][size];
public:
	DistanceTable()
	{
		for(Coord y=0;y<size;y++)
			for(Coord x=0;x<size;x++)
				distanceTable[x][y]=sqrtf((x)*(x)+(y)*(y));
	}
	static inline Coord abs(Coord v)
	{
		return (v>0)?(v):(-v);
	}
	// slow version, but safe
	inline const Real &mod(const Coord &x,const Coord &y)
	{
		assert(x<size && y<size);
		return distanceTable[abs(x)][abs(y)];
	}
	// fast version, but no bounds check
	inline const Real &operator()(Coord x,Coord y)
	{
		return distanceTable[x][y];
	}
};
////////////////////////////////////////////////////////////////
// Generic reference counter. Not tested. Not used
////////////////////////////////////////////////////////////////
//
//class RefCounted
//{
//public:
//#ifndef InterlockedIncrement
//	static inline long InterlockedIncrement(long *val)
//	{
//		*val=*val+1;
//		return *val;
//	}
//#endif
//#ifndef InterlockedDecrement
//	static inline long InterlockedDecrement(long *val)
//	{
//		*val=*val-1;
//		return *val;
//	}
//#endif
//	inline RefCounted()
//	: _cRef(0)
//	{}
//
//	inline ~RefCounted()
//	{ assert(_cRef == 0); }
//
//	inline long zddRef()
//	{ return InterlockedIncrement(&_cRef) ; }
//
//	inline long release()
//	{
//		InterlockedDecrement(&_cRef) ;
//		if (_cRef == 0)
//		{
//			delete this;
//			return 0;
//		}
//		return _cRef ;
//	}
//	inline long  counter() const
//	{
//		return _cRef;
//	}
//private:
//	long _cRef;
//};


template <typename T> class Singleton
{
protected:

	static T* ms_Singleton;

public:
	inline Singleton( void )
	{
		assert( !ms_Singleton );
		ms_Singleton = static_cast< T* >( this );
	}
	virtual ~Singleton( void )
	{
		assert( ms_Singleton );
		ms_Singleton = 0;
	}
	inline static T& getSingleton( void )
	{
		assert( ms_Singleton );
		return ( *ms_Singleton );
	}

	inline static T* getSingletonPtr( void )
	{
		return ms_Singleton;
	}
};

/// K-means clusteriser
class Clusteriser
{
	struct ItemDef
	{
		float value;
		int groupFlag;
	};

	int iMax;
	int iMin;

	int count;
	std::vector<ItemDef> matrix;
public:
	Clusteriser(int _count)
	{
		count=_count;
		matrix.resize(_count*_count);
	}
	ItemDef & operator()(int x,int y)
	{
		return matrix[x+y*count];
	};
	void reset()
	{
		for(int i=0;i<matrix.size();i++)
		{
			matrix[i].value=0;
			matrix[i].groupFlag=0;
		}
	}
	void groups(std::vector<int> &group)
	{
		int newGroup=1;
		// 1st pass - check if all pairs are inside one group
		for(int i=0;i<count;i++)
		{
			group[i]=0;
			(*this)(i,i).groupFlag=1;
			int j=findNearest(i);
			(*this)(i,j).groupFlag=1;
			(*this)(j,i).groupFlag=1;
		}
		// 2nd pass - assemble in groups
		for(int y=0;y<count;y++)
			for(int x=0;x<count;x++)
			{
				if(x==y)
					continue;
				if((*this)(x,y).groupFlag)
				{
					if(group[y]==0)
						group[y]=newGroup++;	// create new group
					if(!group[x])
						group[x]=group[y];
				}
			}
	}
	int findNearest(int x)
	{
		float min=1.0e+16;
		int imin=-1;
		for(int i=0;i<count;i++)
		{
			if(x==i)
				continue;
			float &val=matrix[i+x*count].value;
			if(val<min)
			{
				min=val;
				imin=i;
			}
		}
		return imin;
	}
	void getMinMax()
	{
		float valMax=matrix[0].value;
		float valMin=matrix[0].value;
		iMax=0;
		iMin=0;
		for(int i=1;i<matrix.size();i++)
		{
			if(valMax<matrix[i].value)
			{
				iMax=i;
				valMax=matrix[i].value;
			}
			if(valMin<matrix[i].value)
			{
				iMin=i;
				valMin=matrix[i].value;
			}
		}
	}
	void randomise()
	{
		for(int y=0;y<count;y++)
			for(int x=0;x<count;x++)
			{
				float val=rand();
				(*this)(x,y).value=val;
				//(*this)(y,x).value=val;
			}
	}
	float relativeEquality(int i,int k,int l)
	{
		return 1.0f-fabs(equality(i,k)-equality(i,l));
	}
	float equality(int i,int j)
	{
		return 1.0f-(*this)(i,j).value/matrix[iMax].value;
	}
};



//////////////////////////////////////////////////////////////////////////////
// Binary heap implementation
//////////////////////////////////////////////////////////////////////////////
template <class NodeType,class Comparator = CompareLessEqual<NodeType> >
class BinaryHeap
{
	NodeType **array;
	typedef NodeType *PNode;
	typedef unsigned int WORD;
	int arraySize;
	int arraySizeMax;
	Comparator compare;
public:
	BinaryHeap()
	{
		arraySize=0;
		arraySizeMax=0;
		array=NULL;
	}
	BinaryHeap(int size)
	{
		arraySizeMax=size;
		array=new PNode[arraySizeMax];
		arraySize=0;
	}

	virtual NodeType * bestNode()
	{
		if(array)return *array;
	}
	virtual void addNode(NodeType *node)
	{
		if(node==NULL)return;
	if(arraySize==arraySizeMax)return;

	unsigned int v=arraySize;
	unsigned int u=v;
	NodeType *tmp;
	bool flag=false;

	/*if(node->containerID<size && array[node->containerID]==node)//node is already in container
	{
		v=node->containerID;
		u=v;
		flag=true;
	}
	else
		node->containerID=v;*/
	array[v]=node;
	while(v)
	{
		u=v/2;
		if(compare(*array[v],*array[u]))
		{
			tmp=array[u];
			array[u]=array[v];
			array[v]=tmp;
			//fix ID's
			//array[u]->containerID=u;
			//array[v]->containerID=v;
			v=u;
		}
		else
			break;
	}
	if(!flag)
		arraySize++;
	}
	virtual void removeNode(NodeType *node)
	{
		long i=-1;
		bool flag=false;// if we  have not found this node
		// find node position in container
		//if(array[node->containerID]==node)
		//{
		//	i=node->containerID;
		//	flag=true;
		//}
		// if it failed - search through all nodes
		//else //!!! O(n) search !!! Hash to be implemented
			for(i=0;i<arraySize;i++)
				if(node==array[i])
				{
					flag=true;
					break;
				}
		if(!flag || !arraySize)
			return;
		// remove from binary tree
		unsigned int chLeft=i;
		unsigned int chRight=i;
		unsigned int u=i;
		unsigned int v=i;
		NodeType* tmp;
		arraySize--;
		array[i]=array[arraySize];
		//array[i]->containerID=i;
		array[arraySize]=NULL;

		while(true)
		{
			u=v;
			chLeft=u*2+1;//+1;
			chRight=u*2+2;//+2;
			if(2*u+2<arraySize)
			{
				if(compare(*array[chLeft],*array[u]))v=chLeft;
				if(compare(*array[chRight],*array[v]))v=chRight;
				//if(array[u]->totalCost>=array[chLeft]->totalCost)v=chLeft;
				//if(array[v]->totalCost>=array[chRight]->totalCost)v=chRight;
			}
			else if(chLeft<arraySize)
			{
				if(compare(*array[chLeft],*array[u]))v=chLeft;
				//if(array[u]->totalCost>=array[chLeft]->totalCost)v=chLeft;
			}
			if(u!=v)
			{
				tmp=array[u];
				array[u]=array[v];
				array[v]=tmp;

				//fix ID's
				//array[u]->containerID=u;
				//array[v]->containerID=v;
			}
			else
				break;
		}
	}
	virtual NodeType* operator[](int i)
	{
		return array[i];
	}
	virtual void flush()
	{
		arraySize=0;
	}
	~BinaryHeap()
	{
		if(array)
			delete array;
		array=NULL;
		arraySizeMax=0;
		arraySize=0;
	}
};
