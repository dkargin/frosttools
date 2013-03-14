//typedef unsigned char int8;
class Image;
template <class _Iterator> class _ImageView;
////////////////////////////////////////////////////////////////////////////
// Итератор для работы внутри выделенной области. 
// Упрощёная версия.
// 
////////////////////////////////////////////////////////////////////////////
//template <class _Pix=int8>
typedef unsigned char _Pix;

class PixelIteratorFreeS :
	public std::_Ranit<	_Pix, 
						typename std::allocator<_Pix>::difference_type,
						typename std::allocator<_Pix>::const_pointer, 
						typename std::allocator<_Pix>::const_reference>
{
public:	
	typedef PixelIteratorFreeS _Myt;

	typedef _Pix Pixel;	
	typedef _ImageView<PixelIteratorFreeS> _Parent;

	friend class _Parent;	
	
	Pixel *dataStart;
	Pixel *areaStart,*areaEnd;
	Pixel *lineStart;
	Pixel *lineEnd;
	Pixel *current;
	int rowLength;		// distance betwen pixels, in bytes?
	int pixelStride;
	int pixelSize;	// size of pixel in bytes
public:
	inline PixelIteratorFreeS(_Parent &view,int x,int y,int cx,int cy);
	inline PixelIteratorFreeS& assign(const PixelIteratorFreeS &pi)
	{	
		memcpy(this,&pi,sizeof(*this));		
		return *this;
	}

	PixelIteratorFreeS():
		dataStart(NULL),lineStart(NULL),lineEnd(NULL),current(NULL),pixelSize(1),pixelStride(1)		
	{}
	PixelIteratorFreeS(const PixelIteratorFreeS &R)
	{
		assign(R);
	}
	
	~PixelIteratorFreeS(){};
	
	inline void nextRow()
	{			
		if(lineEnd<areaEnd)
		{
			lineStart=	reinterpret_cast<Pixel*>(reinterpret_cast<char*>(lineStart)	+rowLength);
			lineEnd=	reinterpret_cast<Pixel*>(reinterpret_cast<char*>(lineEnd)	+rowLength);
			current=lineStart;
		}
	}
	inline void prevRow()
	{
		if(lineStart>areaStart)
		{
			lineStart=	reinterpret_cast<Pixel*>(reinterpret_cast<char*>(lineStart)	-rowLength);
			lineEnd=	reinterpret_cast<Pixel*>(reinterpret_cast<char*>(lineEnd)	-rowLength);
			current=lineEnd;
		}
	}
	inline void moveTo(int x,int y);	//move to point(x,y)
	inline void shift(int x,int y);		//shift on vector(x,y)

	PixelIteratorFreeS & operator++();//shift left,
	PixelIteratorFreeS & operator--();

	inline PixelIteratorFreeS & operator++(int){return this->operator++();}
	inline PixelIteratorFreeS & operator--(int){return this->operator++();}
	
	inline PixelIteratorFreeS & operator=(const PixelIteratorFreeS &R){return assign(R);}

	Pixel & operator * ();
	
	inline Pixel * ptr() const
	{
		return current;//dataStart+x+width*y;
	}

	// STL compatibility
	bool operator<(const _Myt& _Right) const
	{	// test if this < _Right
		return (ptr() < _Right.ptr());
	}

	bool operator>(const _Myt& _Right) const
	{	// test if this > _Right
		return (_Right < *this);
	}

	bool operator<=(const _Myt& _Right) const
	{	// test if this <= _Right
		return (!(_Right < *this));
	}

	bool operator>=(const _Myt& _Right) const
	{	// test if this >= _Right
		return (!(*this < _Right));
	}
	friend bool operator==(const PixelIteratorFreeS& a,const PixelIteratorFreeS &b);
};

//template<class Pixel>
inline PixelIteratorFreeS::Pixel & PixelIteratorFreeS/*<Pixel>*/::operator * ()
{
	return *current;
}
//template<class Pixel>
inline PixelIteratorFreeS/*<Pixel>*/ & PixelIteratorFreeS/*<Pixel>*/::operator++()//shift left,
{		
	current++;
	if(current==lineEnd)		
		nextRow();
	return *this;
}
//template<class Pixel>
inline PixelIteratorFreeS/*<Pixel>*/ & PixelIteratorFreeS/*<Pixel>*/::operator--()
{		
	if(current>=lineStart)
		current--;
	else
		prevRow();
	return *this;
}

//template<class Pixel>
inline bool operator==(const PixelIteratorFreeS/*<Pixel>*/& a,const PixelIteratorFreeS/*<Pixel>*/ &b)
{
	return a.current==b.current;
}
//template<class Pixel>
inline bool operator!=(const PixelIteratorFreeS/*<Pixel>*/& a,const PixelIteratorFreeS/*<Pixel>*/ &b)
{
	return a.current!=b.current;
}

template<class Real,class Container>
inline Real scalar_product(Container &a,Container &b)
{
	assert(a.x_size()==b.x_size() && a.y_size()==b.y_size());
	Container::iterator it_a=a.begin();
	Container::iterator it_b=b.begin();
	Container::iterator end=a.end();

	Real res=0;	

	for(;it_a!=end;it_a++,it_b++)
		res+=(*a)*(*b);
	return res;
}
template<class Container>
inline int copy(Container &trg,Container &src)
{
	Container::iterator it_a=trg.begin();
	Container::iterator it_b=src.begin();
	Container::iterator end_a=trg.end();
	Container::iterator end_b=src.end();

	int i=0;
	for(; it_a != end_a && it_b != end_b ; it_a++,it_b++,i++)	
		*it_a=*it_b;	
	return i;
}

template<class Real,class Container>
inline Real difference_sum(const Container &a,const Container &b)
{
	assert(a.x_size()==b.x_size() && a.y_size()==b.y_size());

	Container::iterator it_a=a.begin();
	Container::iterator it_b=b.begin();
	Container::iterator end=a.end();

	Real res=0;	
	Real diff;

	for(;it_a!=end;it_a++,it_b++)
	{
		diff=(*it_a)-(*it_b);
		if(diff<0)diff=-diff;
		res+=diff;
	}
	return res;
}template<class Real,class Container>
inline Real weightedDifference_sum(const Container &a,float ka,const Container &b,float kb)
{
	assert(a.x_size()==b.x_size() && a.y_size()==b.y_size());

	Container::iterator it_a=a.begin();
	Container::iterator it_b=b.begin();
	Container::iterator end=a.end();

	Real res=0;	
	Real diff;

	for(;it_a!=end;it_a++,it_b++)
	{
		diff=(*it_a)*ka-(*it_b)*kb;
		if(diff<0)
			res-=diff;
		else
			res+=diff;
	}
	return res;
}
///////////////////////////////////////////////////////////////////////////////
// Область изображения. Может выходить за пределы реального изображения
// 
///////////////////////////////////////////////////////////////////////////////
//template<class Pixel=int8>
template<class _Iterator>
class _ImageView
{	
	friend class Image;
	
public:
	_ImageView()
	{
		pixelStride=1;
		pixelSize=1;
		parent=NULL;
		rowLength=0;
		data=NULL;
		memset(&areaSelected,0,sizeof(RECT));
		memset(&areaReal,0,sizeof(RECT));
	}
	typedef _Iterator/*<Pixel>*/ iterator;
	//typedef typename Pixel value_type;   
	typedef unsigned int size_type;
	typedef unsigned char Pixel;

	friend class Image;
	friend class _Iterator;
//protected:

	RECT rectIntersect(const RECT &a,const RECT &b)
	{
		RECT res;
		res.left=max(a.left,b.left);
		res.top=max(a.top,b.top);	
		return res;
	}
	Pixel *data;//pointer to color data	int stride;	//row length, in Pixels		
	Image *parent;

	int pixelStride;					// distance between pixels
	int pixelSize;
	int rowLength;				// row length, in bytes

	RECT areaSelected;
	RECT areaReal;			//real selected area
	
	_Iterator itBegin;		//top left corner
	_Iterator itEnd;			//top right corner
	
public:	
	/**/
	_ImageView(const _ImageView &view)
	{
		memcpy(this,&view,sizeof(*this));
		//assign(view);
	}
	_ImageView/*<Pixel>*/ getView(int x,int y,int cx,int cy)
	{
		return _ImageView(*parent,areaSelected.left+x,areaSelected.top+y,cx,cy);
	}
	Image * getImage()
	{
		return parent;
	}
	void assign(const _ImageView &view)
	{
		assign(*view.parent,view.areaSelected.top,
					view.areaSelected.left,
					view.areaSelected.right-view.areaSelected.left,
					view.areaSelected.bottom-view.areaSelected.top);
	}
	void assign(Image &image,int cx, int cy, int width, int height)
	{
		parent=&image;
		areaSelected.top=cy;
		areaSelected.left=cx;
		areaSelected.right=cx+width;
		areaSelected.bottom=cy+height;	

		areaReal.left=areaSelected.left;
		areaReal.right=areaSelected.right;
		areaReal.bottom=areaSelected.bottom;
		areaReal.top=areaSelected.top;
		//pixelSize=image		
		if(areaReal.left<0)
			areaReal.left=0;
		if(areaReal.top<0)
			areaReal.top=0;
		if(areaReal.bottom>=image.height())
			areaReal.bottom=image.height();
		if(areaReal.right>=image.width())
			areaReal.right=image.width();	
		rowLength=image.getCore()->widthStep;
		pixelSize=image.channels()*image.depth()>>3;
		pixelStride=image.channels()*image.depth()>>3;
		update();
	}
	_ImageView(Image &image,int cx, int cy, int width, int height)			
	{
		assign(image,cx,cy,width,height);
	}
	const Pixel * operator()(int x,int y) const
	{
		int rx=areaSelected.left+x;
		int ry=areaSelected.top+y;
		return (int8*)&parent->getCore()->imageData[rx*pixelStride+ry*rowLength];
	}
	Pixel * operator()(int x,int y)
	{
		int rx=areaSelected.left+x;
		int ry=areaSelected.top+y;
		return (Pixel*)&parent->getCore()->imageData[rx*pixelStride+ry*rowLength];
	}
	inline bool valid()
	{
		return parent!=NULL;
	}
	void resize(int w,int h)
	{
	}
	size_type x_size() const
	{
		return areaSelected.right-areaSelected.left;
	}
	size_type y_size() const
	{
		return areaSelected.bottom-areaSelected.top;
	}
	size_type size() const
	{
		return x_size()*y_size();
	}
	iterator line_begin(int y);
	iterator line_end(int y);

	void update()
	{
		int left,right;
		left=-areaSelected.left+areaReal.left;
		right=MAX(areaSelected.right-areaReal.right,0);		
		
		data=(Pixel*)parent->getCore()->imageData;

		int x=x_size();
		int y=y_size();

		itBegin=iterator(*this,areaReal.left,areaReal.top,x_size(),y_size());
		itEnd=	iterator(*this,areaReal.left,areaReal.top,x_size(),y_size());
		itEnd.current=itEnd.areaEnd;		
	}
	void print()
	{
		cout<<"ImageView debug output"<<endl;
		cout<<" start=("<<areaReal.left<<","<<areaReal.top<<")\n";
		cout<<" end=("<<areaReal.right<<","<<areaReal.bottom<<")\n";
		
		cout<<" stride="<<parent->image->widthStep<<endl;
		cout<<" address origin=";
		std::hex(cout);
		cout<<(long)(parent->image->imageDataOrigin)<<endl;
		cout<<" address start="<<(long)(parent->image->imageData)<<endl;

	}
	iterator begin() const
	{
		return itBegin;
	}
	iterator end() const
	{
		return itEnd;
	}
	inline vec2i getPos(const iterator & it)
	{
		vec2i res;
		res.c[0]=((it.current-it.areaStart)%rowLength)/pixelStride;
		res.c[1]=(it.current-it.areaStart)/rowLength;
		return res;
	}
};

typedef _ImageView<PixelIteratorFreeS> ImageView;

inline PixelIteratorFreeS::PixelIteratorFreeS(PixelIteratorFreeS::_Parent &view,int x,int y,int cx,int cy)		
{		
	dataStart=view.data;
	rowLength=view.rowLength;
	pixelSize=view.pixelSize;
	pixelStride=view.pixelStride;
	lineStart=dataStart+x*pixelStride+rowLength*y;
	lineEnd=lineStart+cx;
	areaStart=dataStart+x*pixelStride+rowLength*y;
	areaEnd=dataStart+(x+cx)*pixelStride+rowLength*(y+cy-1);
	current=lineStart;
}