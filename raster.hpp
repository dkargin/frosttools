#ifndef RASTER_HPP
#define RASTER_HPP
//////////////////////////////////////////////////////////////////
// Класс для рисования на растре любого вида. Требуется поддержка
// функции setPixel(x,y)
// size_x(),size_y()
//////////////////////////////////////////////////////////////////
template< class Raster>
class RasterDraw
{
	Raster &target;	// растр, в котороый рисуем
	typedef typename Raster::value_type value_type;
	value_type color;

	inline void hor_line(int x1,int x2,int y)
	{
		if(x1>x2)
		{
			int tmp=x1;x1=x2;x2=tmp;
		}
		x1=std::max(int(0),x1);
		x2=std::min(target.size_x()-1,x2);
		if(y<0 || y>=target.size_y())
			return;
		for(int x=x1;x<=x2;x++)
			target.setPixel(x,y,color);
	}
	void ver_line(int x,int y1,int y2)
	{
		if(y1>y2)
		{
			int tmp=y1;y1=y2;y2=tmp;
		}
		for(int y=y1;y<=y2;y++)
			target.setPixel(x,y,color);
	}
public:
	RasterDraw(Raster &nTarget)
		:target(nTarget)
	{}
	void setColor(value_type c)
	{
		color=c;
	}
	value_type getColor()
	{
		return color;
	}
	void line(int x1,int y1,int x2,int y2);
	void boxSolid(int x0,int y0,int x1,int y1)
	{
		x0=std::max(x0,0);
		x1=std::min(x1,target.size_x());
		y0=std::max(y0,0);
		y1=std::min(y1,target.size_y());

		for(int y=y0;y<y1;y++)
			hor_line(x0,x1,y);
	}
	void circleSolid(int xc,int yc,int size)
	{
		int xCenter=xc;
		int yCenter=yc;
		for(int r=0;r<size;r++)
		{
			int	x      = 0;
			int	y      = r;
			int	d      = 1 - r;
			int	delta1 = 3;
			int	delta2 = -2*r + 5;

			hor_line(xCenter-x,xCenter+x,yCenter-y);
			hor_line(xCenter-x,xCenter+x,yCenter+y);
			hor_line(xCenter-y,xCenter+y,yCenter-x);
			hor_line(xCenter-y,xCenter+y,yCenter+x);

			while ( y > x )
			{
				if ( d < 0 )
				{
					d      += delta1;
					delta1 += 2;
					delta2 += 2;
					x++;
				}
				else
				{
					d      += delta2;
					delta1 += 2;
					delta2 += 4;
					x++;
					y--;
				}

				hor_line(xCenter-x,xCenter+x,yCenter-y);
				hor_line(xCenter-x,xCenter+x,yCenter+y);
				hor_line(xCenter-y,xCenter+y,yCenter-x);
				hor_line(xCenter-y,xCenter+y,yCenter+x);
			}
		}
	}
};

template<class Pixel>
class Raster2D
{
protected:
	Pixel *data;
	int pixelSize;
	int width;
	int height;
	bool iExtern;
	int stride;
public:
	typedef Pixel value_type;
	typedef Raster2D<Pixel> my_type;

	Raster2D(Pixel *nData,int nWidth,int nHeight)
 		:iExtern(true),data(nData),width(nWidth),height(nHeight),pixelSize(sizeof(Pixel)),zero(0)
	{}
	Raster2D(int nWidth,int nHeight)
		:iExtern(false),width(nWidth),height(nHeight),pixelSize(sizeof(Pixel))
	{
		data = new Pixel[width*height];
	}
	explicit Raster2D(const my_type &raster)
	{
		width=raster.width;
		height=raster.height;
		data=new Pixel[width*height];
	}
	~Raster2D()
	{
		if(!iExtern && data)
			delete[] data;
	}
	virtual inline void setPixel(int x,int y,Pixel c)
	{
		if(x<width && x>=0 && y<height && y>=0)
			data[x+y*width]=c;
	}
	void clear(const Pixel &c)
	{
		Pixel *ptr=data;
		Pixel *end=data+width*height;

		for(;ptr!=end;ptr++)
			*ptr=c;
	}

	bool valid(int x, int y) const
	{
		return x<width && x>=0 && y<height && y>=0;
	}

	const Pixel & operator () (int x,int y) const
	{
		if(!valid(x,y))
			_xbounds(x,y);
		return data[x+y*width];
	}
	
	Pixel & operator () (int x,int y)
	{
		if(!valid(x,y))
			_xbounds(x,y);
		return data[x+y*width];
	}
	inline int size_x()
	{
		return width;
	}
	inline int size_y()
	{
		return height;
	}
	my_type & operator=(const my_type &raster)
	{
		assert(width==raster.width);
		assert(height==raster.height);
		for(int i=0;i<width*height;i++)
			data[i]=raster.data[i];
		return *this;
	}
public:
	Pixel * getData() const
	{
		return data;
	}
protected:
	void _xbounds(int x, int y) const
	{
		_CrtDbgBreak();
		throw std::exception("Raster2D::point is out of bounds");
	}
};

/////////////////////////////////////////////////////////////////////
// 2d array
/////////////////////////////////////////////////////////////////////
template<class Type>
class Raster
{
public:
	typedef Raster<Type> my_type;
	typedef Type value_type;
	typedef size_t size_type;

	Raster()
		:sx(0),sy(0)
	{}
	Raster(int w,int h)
		:sx(w),sy(h),data(w*h)
	{}
	~Raster()
	{}
	bool validPos(size_t x,size_t y) const
	{
		return x>=0 && x<sx && y>=0 && y<sy;
	}
	const Type &operator()(size_t x,size_t y)const
	{
		return get(x,y);
	}
	Type &operator()(size_t x,size_t y)
	{
		return get(x,y);
	}
	virtual const Type &get(size_t x,size_t y)const
	{
		assert(validPos(x,y));
		return data[x+sx*y];
	}
	virtual Type &get(size_t x,size_t y)
	{
		assert(validPos(x,y));
		return data[x+sx*y];
	}
	void resize(size_t w,size_t h)
	{
		sx=w;
		sy=h;
		data.resize(w*h);
	}
	Type & operator[](size_t i)
	{
		return data[i];
	}
	const Type & operator[](size_t i) const
	{
		return data[i];
	}
	size_t size_x() const
	{
		return sx;
	}
	size_t size_y() const
	{
		return sy;
	}
protected:
	std::vector<Type> data;
	size_t sx;
	size_t sy;
};

template<class Type>
class Raster3D
{
public:
	typedef Raster3D<Type> my_type;
	typedef Type value_type;
	typedef std::vector<Type> Container;
	typedef int size_type;

	Raster3D()
		:sx(0),sy(0),sz(0),size_xy(0),data(NULL)
	{}
	Raster3D(int x,int y,int z)
		:data(NULL)
	{
		resize(x,y,z);
	}
	~Raster3D()
	{
		clear();
	}
	void clear()
	{
		sx=0;
		sy=0;
		sz=0;
		size_xy=0;
		if(data)
		{
			delete []data;
			data=NULL;
		}
	}
	bool validPos(int x,int y,int z) const
	{
		assert(data);
		return x>=0 && x<(int)sx && y>=0 && y<(int)sy && z>=0 && z<(int)sz;
	}
	const Type &operator()(int x,int y,int z)const
	{
		assert(data);
		assert(validPos(x,y,z));
		return data[x+sx*y+size_xy*z];
	}
	Type &operator()(int x,int y,int z)
	{
		assert(data);
		assert(validPos(x,y,z));
		return data[x+sx*y+size_xy*z];
	}
	const Type &get(int x,int y,int z)const
	{
		assert(validPos(x,y,z));
		return data[x+sx*y+size_xy*z];
	}
	Type &get(int x,int y,int z)
	{
		assert(validPos(x,y,z));
		return data[x+sx*y+size_xy*z];
	}
	void resize(int x,int y,int z)
	{
		clear();
		sx=x;
		sy=y;
		sz=z;
		size_xy=x*y;
		data=new Type[x*y*z];
	}
	inline size_type size_x()
	{
		return sx;
	}
	inline size_type size_y()
	{
		return sy;
	}
	inline size_type size_z()
	{
		return sz;
	}
	inline size_type index(int x,int y,int z)
	{
		return x+sx*y+size_xy*z;
	}
	inline void coord(size_type index,size_type &x,size_type &y,size_type &z)
	{
		z=index/size_xy;
		size_type d=index%size_xy;
		y=d/sx;
		x=d%sx;
	}
protected:
	Type* data;
	size_type sx,sy,sz,size_xy;
};
#endif