#ifndef MULTILAYER
#define MULTILAYER
#include <vector>

// make size_type and difference_type <long> instead if <int>
template<class Ty>
class LongAlloc: public std::allocator<Ty>
{
public:
	typedef long size_type;
	typedef long difference_type;
};

class Multilayer
{	
	class _Layer
	{
	public:
		int size;
		int offset;
		int stride;
		bool enabled;
		uint8 *data;

		_Layer()
			:size(0),offset(0),enabled(false),data(NULL)
		{}
		_Layer(const _Layer &l)
			:size(l.size),offset(l.offset),enabled(l.enabled),data(l.data)
		{}
	};

	typedef std::vector<Multilayer::_Layer,LongAlloc<Multilayer::_Layer> > Container;
	Container layers;	
public:	
	typedef Container::size_type size_type;
	//std::vector<int> test;	
	//_Layer layers[16];
	int layersCount;
	
	static const bool bAlligned=false;		
private:
	template<class Type> 
	static Type * incPtr(Type *a,size_type i)
	{
		return (Type*)((char*)(a)+i);
	}
	unsigned char *dataBuffer;
	unsigned char *nodeLast;

	size_type dataBufferSize;	
	size_type stride;			// local node size
	size_type nodeCount;	

	void updateNodeFormat()
	{
		stride=0;
		for(size_type i=0;i<layers.size();i++)
		//for(size_type i=0;i<layersCount;i++)
		{			
			if(layers[i].enabled)
			{
				layers[i].offset=stride;
				stride+=layers[i].size;
			}
		}
		if(bAlligned)
		{
			if(stride<4)
				stride=4;
			else if(stride<8)
				stride=8;
			else if(stride<16)
				stride=16;
			else if(stride<32)
				stride=32;
			else if(stride<64)
				stride=64;
			else if(stride<128)
				stride=128;
		}
	}
	void check(size_t id) const
	{
		_ASSERT(id < layers.size());		
		//_ASSERT(id<layersCount);		
	}
public:	
	Multilayer()
	{
		stride=0;
		dataBuffer=NULL;
		nodeLast=NULL;
		nodeCount=0;
		layersCount=0;
		layers.resize(0);
	}	
	~Multilayer()
	{
		releaseMultilayer();
	}
	inline size_type getNodeSize() const 
	{
		return stride;
	}
	size_type addLayer(size_type size,bool enabled=true)
	{
		_Layer layer;
		layer.size=size;
		layer.enabled=enabled;

		layers.push_back(layer);			
		//layers[++layersCount]=layer;
		//return layersCount;
		return layers.size()-1;
	}
	inline void enableLayer(int id)
	{
		layers[id].enabled=true;
	}
	inline void disableLayer(int id)
	{
		layers[id].enabled=true;
	}
	inline unsigned int getLayerOffset(int n) const
	{
		return layers[n].offset;
	}
	void releaseMultilayer()
	{
		if(dataBuffer)
		{
			delete dataBuffer;
			dataBuffer=NULL;

			stride=0;
			dataBuffer=NULL;
			nodeLast=NULL;
			nodeCount=0;
		}
	}
	void allocNodes(long n)
	{
		releaseMultilayer();			
		updateNodeFormat();
		dataBufferSize=n*stride;
		dataBuffer=new unsigned char[dataBufferSize];
		nodeLast=dataBuffer+stride*(n-1);
		nodeCount=n;
	}	
	inline int layerEnabled(int id) const
	{
		return layers[id].enabled;
	}
	///Write array to layer
	///rough variant, good with large objects, or if copy operator is undefined
	void writeLayerData(int n,void *data)
	{		
		int inc=layers[n].size;
		register char *ptr=(char*)dataBuffer+layers[n].offset;
		char *end=(char*)nodeLast+layers[n].offset;		
		
		for(;ptr!=end;ptr+=stride,data=incPtr(data,inc))
			memcpy((char*)data,ptr,inc);
	}
	///Read from layer to array
	///rough variant, good with large objects, or if copy operator is undefined
	void readLayerData(int n,void *data) const
	{		
		int inc=layers[n].size;
		char *pd=(char*)data;
		register char *ptr=(char*)dataBuffer+layers[n].offset;
		char *end=(char*)nodeLast+layers[n].offset;		
		
		for(;ptr<=end;ptr+=stride,pd+=inc)
			memcpy(ptr,pd,inc);
	}	
	///Write array to layer
	///Preffered variant, especially if we have '=' operator
	template<class Type>
	void writeLayer(int n,Type *data)
	{	
		_ASSERT(sizeof(Type)==layers[n].size);		
		register Type *ptr=(Type*)getLayerPointer(n);
		Type *end=(Type*)(nodeLast+layers[n].offset);		
		
		for(;ptr<end;ptr=incPtr(ptr,stride),data++)
			*ptr=*data;
	}
	///Read from layer to array
	///Preffered variant, especially if we have '=' operator
	template<class Type>
	void readLayer(int n,Type *data) const
	{		
		_ASSERT(sizeof(Type)==layers[n].size);
		register Type *ptr=(Type*)getLayerPointer(n);
		Type *end=(Type*)(nodeLast+layers[n].offset);		
		
		for(long i=0;i<nodeCount;ptr=incPtr(ptr,stride),data++,i++)
			*data=*ptr;
	}
	template<class Type>
	void setLayer(int n,Type data)
	{
		int sa=sizeof(Type);
		int sb=layers[n].size;
		_ASSERT(sizeof(Type)==layers[n].size);
		register Type *ptr=(Type*)getLayerPointer(n);
		Type *end=(Type*)(nodeLast+layers[n].offset);		
		
		for(;ptr<end;ptr=incPtr(ptr,stride))
			*ptr=data;
	}
	
	inline void * getNodePointer(size_type n) const
	{	
		return dataBuffer+n*stride;
	}
	inline void * getLayerPointer(int layerID,long i=0) const
	{		
		const _Layer &layer=layers[layerID];
		if(layer.data)
			return layer.data+i*layer.stride;
		else
			return dataBuffer+i*stride+layer.offset;
	}
	inline size_type size_mem() const
	{
		return dataBufferSize;
	}
	inline size_type size() const
	{
		return nodeCount;
	}
	inline size_type size_node() const
	{
		return stride;
	}
};

#endif