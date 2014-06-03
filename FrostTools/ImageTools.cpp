<<<<<<< HEAD
#include "StdAfx.h"

/////////////////////////////////////////////////////////////
// Correlation.cpp
// defines all image algorythms
// Image definition, Matrix filter, Correlation
/////////////////////////////////////////////////////////////
//Программная
//модель искусственной нейронной сети на основе нейроподобных
//элементов с временной суммацией сигналов с двумерным
//дендритом
/////////////////////////////////////////////////////////////

#include "Document.h"
#include "imageTools.h"
//#include "imageView.h"

#include <math.h>

#pragma comment(lib,"DevIL.lib")
#pragma comment(lib,"ilu.lib")
#pragma comment(lib,"ilut.lib")
bool ilRun=false;

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

ILenum convertPixelTypeL2IL(int pixelType)
{
	switch(pixelType)
	{
	case pixelTypeRGB:
		return IL_RGB;
	case pixelTypeRGBA:
		return IL_RGBA;
	case pixelTypeLuminance:
		return IL_LUMINANCE;
	}
	return 0;
}
int convertPixelTypeIL2L(ILenum pixelType)
{
	switch(pixelType)
	{
	case IL_RGB:
		return pixelTypeRGB;
	case IL_RGBA:
		return pixelTypeRGBA;
	case IL_LUMINANCE:
		return pixelTypeLuminance;
	}
	return pixelTypeUnsupported;
}


////////////////////////////////////////////////////////////////
// Image Class. IL image wrapper & auxillary controls
////////////////////////////////////////////////////////////////
Image::Image(Workspace *ws)
:BaseDocument("generic image",ws,documentTypeImage)
{
	if(!imageCounter++ && !ilRun)
	{
		ilInit();
		iluInit();
		ilutInit();
		ilRun=true;
	}
	data=NULL;
	//surface=NULL;
	fileName=NULL;
	valid=false;
	source=imageSourceInvalid;
}

Image::~Image(void)
{
	if(imageName)
		ilDeleteImages(1,(ILuint*)&imageName);	
	/*if(surface)
		surface->release();*/
	if(fileName)
		delete fileName;
	if(!--imageCounter)
		ilShutDown();	
}

Image::Image(const char *file,Workspace *ws,const char *Name)
:BaseDocument((Name==NULL)?"generic image":Name,ws,documentTypeImage)
{
	if(!imageCounter++ && !ilRun)
	{
		ilInit();
		iluInit();
		ilutInit();
		ilRun=true;
	}
	data=NULL;
	fileName=NULL;
	//surface=NULL;
	source=imageSourceInvalid;
	pixelFormat=pixelTypeLuminance;
	loadImage(file);
}
Image::Image(int w, int h, int imageType,Workspace *ws)
:BaseDocument("generic image",ws,documentTypeImage)
{
	if(!imageCounter++ && !ilRun)
	{
		ilInit();
		iluInit();
		ilutInit();
		ilRun=true;
	}
	data=NULL;
	fileName=NULL;
	//surface=NULL;
	imageCreate(w,h,imageType);
}

bool Image::isValid()
{
	return valid;
}
int Image::getWidth()
{
	return width;
}
int Image::getHeight()
{
	return height;
}

template <class Pixel>
PixelIterator<Pixel> Image::getPixel(int x, int y)
{
	int8 *d=NULL;	
	ilBindImage(imageName);	
	data=ilGetData();
	d=data;	
	PixelIterator<Pixel> res;

	res.current=data+width*y+x;
	res.areaStart=data;
	res.areaEnd=data+width*height;
	res.lineStart=NULL;
	res.lineEnd=NULL;
	res.rectMode=rectModeClose;	
	res.rectMode=rectModeOff;
	return res;
}

template <class Pixel>
PixelIterator<Pixel> Image::getRect(int left, int top, int right, int bottom)
{
	right=MIN(right,width);
	bottom=MIN(bottom,height);
	top=MAX(top,0);
	left=MAX(left,0);

	ilBindImage(imageName);	
	data=ilGetData();

	PixelIterator<Pixel> res;	
	res.areaStart=data+left+top*width;
	res.areaEnd=data+right+width*bottom;
	res.lineStart=data+left+top*width;
	res.lineEnd=data+right+width*top;
	res.rectMode=rectModeClose;
	res.current=data+left+top*width;
	res.stride=width;
	SetRect( &res.area,left,top,right,bottom);
	return res;
}
void Image::resize(int newWidth, int newHeight)
{
	if(this->isValid())
	{		
		ilBindImage(imageName);
		iluImageParameter(ILU_FILTER,ILU_BILINEAR);
		iluScale(newWidth,newHeight,1);
		width=ilGetInteger(IL_IMAGE_WIDTH);
		height=ilGetInteger(IL_IMAGE_HEIGHT);
		data=ilGetData();	
	}
}
template <class Pixel>
void Image::setPixel(int x,int y,const Pixel &c)
{
	
}
//void Image::createSurface(IDisplay *display)
//{	
//	if(isValid()&&!surface)
//		display->createSurface(&surface,width,height);
//	surfaceUpdated=false;
//}
//void Image::updateDDrawSurface()
//{
//	if(isValid() && surface && !surfaceUpdated)
//	{
//		PixelIterator<int8> p(getPixel<int8>(0,0));		
//		COLORREF clr;
//		char v;
//		surface->lockSurface();		
//		for(int y=0;y<height;y++)
//			for(int x=0;x<width;x++,p++)
//			{
//				v=*p;
//				clr=RGB(v,v,v);
//				surface->putPixel(x,y,clr);
//			}
//		surface->unLockSurface();
//		surfaceUpdated=true;
//	}
//}
//ISurface * Image::getSurface()
//{
//	return surface;
//}
int Image::getPixelFormat()
{
	return pixelFormat;
}

const char* Image::getName()
{
	return fileName;
}
int Image::imageCreate(int w,int h,int imageType)
{
	source=imageSourceMemory;
	ilGenImages(1,(ILuint*)&imageName);

	ilBindImage(imageName);
	bool res= (0!=ilTexImage(w,h,1,1,convertPixelTypeL2IL(pixelFormat),IL_UNSIGNED_BYTE,NULL) );	
	
	if(res)
	{		
		width=w;
		height=h;
		pixelFormat=imageType;	
		width=ilGetInteger(IL_IMAGE_WIDTH);
		height=ilGetInteger(IL_IMAGE_HEIGHT);
		
		valid=true;
	}
	else
		return 0;
	return 1;
}
int Image::loadImage(const char *file)
{	
	source=imageSourceFile;
	ilGenImages(1,(ILuint*)&imageName);
	ilBindImage(imageName);
	bool res= (0!=ilLoadImage(file));
	if(res)
	{		
//		surface=NULL;
		pixelFormat=convertPixelTypeIL2L(ilGetInteger(IL_IMAGE_FORMAT));
		ilConvertImage(convertPixelTypeL2IL(pixelFormat),IL_UNSIGNED_BYTE);
		fileName=new char[strlen((char*)file)+1];
		strcpy(fileName,file);
		width=ilGetInteger(IL_IMAGE_WIDTH);
		height=ilGetInteger(IL_IMAGE_HEIGHT);	
		valid=true;
	}
	else
		return 0;
	return 1;	
}
int Image::saveImage(const char *path)
{
	//USES_CONVERSION;
	//char *file=W2A(path);
	ilBindImage(imageName);
	if(ilSaveImage(path))			
		return 1;	
	ILenum err=ilGetError();
	return 0;
}

int copyImage(Image *dst,Image * src)
{
	int w=src->getWidth();
	int h=src->getHeight();
	dst->imageCreate(w,h,src->getType());
	//PixelIterator it_src<in=src->getPixel(0,0),it_dst=dst->getPixel(0,0);
	return 0;
}

int Image::save(std::ofstream &out)
{
	out<<documentPath;	
	return 0;
}
int Image::load(std::ifstream &in)
{
	char tmpPath[MAX_PATH];
	in>>tmpPath;
	loadImage(tmpPath);
	return 0;
}


int Image::imageCounter=0;


Filter::Filter(Workspace *ws)
:BaseDocument("generic filter",ws,documentTypeFilter)
{}

Filter::~Filter(void)
{}


void Filter::init()
{}


void Filter::applyTo(Image &source,Image &target)
{
	DWORD i=0;
	lastSource=&source;
	lastTarget=&target;
	

}

MatrixFilter::MatrixFilter(int filterSize,Workspace *ws)
:Filter(ws)
{
	size=filterSize;
	matrix=new float[(2*size+1)*(2*size+1)];
	lastSource=NULL;
	lastTarget=NULL;
}

MatrixFilter::~MatrixFilter()
{
	if(matrix)
		delete matrix;
}

//template<class Pixel>
void MatrixFilter::applyTo(Image &target,Image &source)
{
	int x1,y1;
	float r;
	PixelIterator<int8> trg=target.getPixel<int8>(0,0);
	PixelIteratorConst<int8> src;
	for(int y=0;y<source.getHeight();y++)
		for(int x=0;x<source.getWidth();x++,trg++)
		{
			src=source.getRect<int8>(x-size,y-size,x+size,y+size);
			RECT rc;
			src.getRect(rc);
			r=0;
			for(y1=rc.top-y;y1<rc.bottom-y;y1++)
				for(x1=rc.left-x;x1<rc.right-x;x1++,src++)
					r+=matrix[x1+size+(y1+size)*(size+size+1)]*(int8)(*src);
			*trg=(int8)r;
		}
}

void MatrixFilter::init()
{}
void MatrixFilter::setElementFn(FunctionXY &fn)
{
	//ASSERT(fn);
	for(int y=-size;y<size;y++)
		for(int x=-size;x<size;x++)
			setElement(x,y,fn((float)x,(float)y));
}
void MatrixFilter::setElement(signed int x,signed int y,float e)
{
	if(	x<-size || 
		x> size ||
		y<-size ||
		y> size)
		return;
	x+=size;
	y+=size;
	matrix[x+y*(size+size+1)]=e;
}
float & MatrixFilter::operator () (int x,int y)
{
	return matrix[x+size+(y+size)*(size+size+1)];
}
float MatrixFilter::getElement(signed int x,signed int y)
{
	if(	x<-size || 
		x> size ||
		y<-size ||
		y> size)
		return 0;
	x+=size;
	y+=size;
	return matrix[x+size+(y+size)*(size+size+1)];
}
int MatrixFilter::save(std::ofstream &out)
{
	out<<size;
	out.write((char *)matrix,sizeof(float)*(2*size+1)*(2*size+1));
	return 0;
}
int MatrixFilter::load(std::ifstream &in)
{
	in>>size;
	if(matrix)
		delete matrix;
	matrix=new float[(2*size+1)*(2*size+1)];
	in.read((char *)matrix,sizeof(float)*(2*size+1)*(2*size+1));
	return 0;
}

Operator::Operator(const char *name,Workspace *ws)
:BaseDocument(name,ws,documentTypeOperator)
{}
void Operator::init()
{
}
void Operator::setSource(Image *a, Image *b)
{
	lastSource[0]=a;
	lastSource[1]=b;
}
void Operator::setSource(Image *a[])
{
	lastSource[0]=a[0];
	lastSource[1]=a[1];
}
void Operator::setTarget(Image *target)
{
	lastTarget=target;
}

Correlation::Correlation(int s,float t,Workspace *ws)
:Operator("Correlation",ws)
{
	size=s;
	threshold=t;
	matrix=new float[(2*size+1)*(2*size+1)];
	for(int i=0;i<(2*size+1)*(2*size+1);i++)
		matrix[i]=0;
}
Correlation::~Correlation()
{
	if(matrix)
		delete matrix;
}
void  Correlation::setElement(signed int x,signed int y,float e)
{
	if(	x<-size || 
		x> size ||
		y<-size ||
		y> size)
		return;
	x+=size;
	y+=size;
	matrix[x+y*(size+size+1)]=e;
}
float  Correlation::getEletemt(signed int x,signed int y)
{
	if(	x<-size || 
		x> size ||
		y<-size ||
		y> size)
		return 0;
	x+=size;
	y+=size;
	return matrix[x+y*(size+size+1)];
}
void Correlation::set(float c)
{
	for(int i=0;i<(2*size+1)*(2*size+1);i++)
		matrix[i]=c;
}
float Correlation::sum()
{
	float res=0;
	for(int i=0;i<(2*size+1)*(2*size+1);i++)
		res+=matrix[i];
	return res;
}
void Correlation::init(int s,float t)
{
	size=s;
	threshold=t;
	if(matrix)
		delete matrix;
	matrix=new float[(2*size+1)*(2*size+1)];
	for(int i=0;i<(2*size+1)*(2*size+1);i++)
		matrix[i]=0;
}
int Correlation::save(std::ofstream &out)
{
	out<<size;
	out<<threshold;
	return 1;
}
int Correlation::load(std::ifstream &in)
{
	in>>size;
	in>>threshold;
	init(size,threshold);
	return 1;
}

void Correlation::apply()
{
	if(!lastSource[0])
		return;
	if(!lastSource[1])
		return;
	if(!lastTarget)
		return;
	if(	lastTarget->getWidth()!=lastSource[0]->getWidth()||
		lastTarget->getWidth()!=lastSource[1]->getWidth()||
		lastTarget->getHeight()!=lastSource[0]->getHeight()||
		lastTarget->getHeight()!=lastSource[1]->getHeight())
		return;
	int width=lastSource[0]->getWidth();
	int height=lastSource[0]->getHeight();
	PixelIterator<int8> t=lastTarget->getPixel<int8>(0,0);


	signed int x,y;
	signed int x1,y1;
	signed int dx,dy;
	signed int ye;
	int length=0;
	float val=0;

	for(y=0;y<height;y++)
	{		
		ye=MIN(y+size,height-1);		
		for(x=0;x<width;x++,t++)
		{
			y1=MAX(y-size,0);
			x1=MAX(x-size,0);
			int xe=MIN(x+size,width-1);
			
			val=0;
			length=0;

			PixelIterator<int8> s0=lastSource[0]->getRect<int8>(x1,y1,xe,ye);
			PixelIterator<int8> s1=lastSource[1]->getRect<int8>(x1,y1,xe,ye);

			for(;y1<=ye;y1++)
			{
				for(;x1<=xe;x1++)	
				{
					length++;										
					float diff= *s1- *s0;
					val+=fabs(diff);								
				}			
			}	
			float k=(val/length);
			if(threshold>0)
				k=(k>threshold)?255:0;						
			*t=(int8)k;
		}
	}
=======
#include "StdAfx.h"

/////////////////////////////////////////////////////////////
// Correlation.cpp
// defines all image algorythms
// Image definition, Matrix filter, Correlation
/////////////////////////////////////////////////////////////
//Программная
//модель искусственной нейронной сети на основе нейроподобных
//элементов с временной суммацией сигналов с двумерным
//дендритом
/////////////////////////////////////////////////////////////

#include "Document.h"
#include "imageTools.h"
//#include "imageView.h"

#include <math.h>

#pragma comment(lib,"DevIL.lib")
#pragma comment(lib,"ilu.lib")
#pragma comment(lib,"ilut.lib")
bool ilRun=false;

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

ILenum convertPixelTypeL2IL(int pixelType)
{
	switch(pixelType)
	{
	case pixelTypeRGB:
		return IL_RGB;
	case pixelTypeRGBA:
		return IL_RGBA;
	case pixelTypeLuminance:
		return IL_LUMINANCE;
	}
	return 0;
}
int convertPixelTypeIL2L(ILenum pixelType)
{
	switch(pixelType)
	{
	case IL_RGB:
		return pixelTypeRGB;
	case IL_RGBA:
		return pixelTypeRGBA;
	case IL_LUMINANCE:
		return pixelTypeLuminance;
	}
	return pixelTypeUnsupported;
}


////////////////////////////////////////////////////////////////
// Image Class. IL image wrapper & auxillary controls
////////////////////////////////////////////////////////////////
Image::Image(Workspace *ws)
:BaseDocument("generic image",ws,documentTypeImage)
{
	if(!imageCounter++ && !ilRun)
	{
		ilInit();
		iluInit();
		ilutInit();
		ilRun=true;
	}
	data=NULL;
	//surface=NULL;
	fileName=NULL;
	valid=false;
	source=imageSourceInvalid;
}

Image::~Image(void)
{
	if(imageName)
		ilDeleteImages(1,(ILuint*)&imageName);	
	/*if(surface)
		surface->release();*/
	if(fileName)
		delete fileName;
	if(!--imageCounter)
		ilShutDown();	
}

Image::Image(const char *file,Workspace *ws,const char *Name)
:BaseDocument((Name==NULL)?"generic image":Name,ws,documentTypeImage)
{
	if(!imageCounter++ && !ilRun)
	{
		ilInit();
		iluInit();
		ilutInit();
		ilRun=true;
	}
	data=NULL;
	fileName=NULL;
	//surface=NULL;
	source=imageSourceInvalid;
	pixelFormat=pixelTypeLuminance;
	loadImage(file);
}
Image::Image(int w, int h, int imageType,Workspace *ws)
:BaseDocument("generic image",ws,documentTypeImage)
{
	if(!imageCounter++ && !ilRun)
	{
		ilInit();
		iluInit();
		ilutInit();
		ilRun=true;
	}
	data=NULL;
	fileName=NULL;
	//surface=NULL;
	imageCreate(w,h,imageType);
}

bool Image::isValid()
{
	return valid;
}
int Image::getWidth()
{
	return width;
}
int Image::getHeight()
{
	return height;
}

template <class Pixel>
PixelIterator<Pixel> Image::getPixel(int x, int y)
{
	int8 *d=NULL;	
	ilBindImage(imageName);	
	data=ilGetData();
	d=data;	
	PixelIterator<Pixel> res;

	res.current=data+width*y+x;
	res.areaStart=data;
	res.areaEnd=data+width*height;
	res.lineStart=NULL;
	res.lineEnd=NULL;
	res.rectMode=rectModeClose;	
	res.rectMode=rectModeOff;
	return res;
}

template <class Pixel>
PixelIterator<Pixel> Image::getRect(int left, int top, int right, int bottom)
{
	right=MIN(right,width);
	bottom=MIN(bottom,height);
	top=MAX(top,0);
	left=MAX(left,0);

	ilBindImage(imageName);	
	data=ilGetData();

	PixelIterator<Pixel> res;	
	res.areaStart=data+left+top*width;
	res.areaEnd=data+right+width*bottom;
	res.lineStart=data+left+top*width;
	res.lineEnd=data+right+width*top;
	res.rectMode=rectModeClose;
	res.current=data+left+top*width;
	res.stride=width;
	SetRect( &res.area,left,top,right,bottom);
	return res;
}
void Image::resize(int newWidth, int newHeight)
{
	if(this->isValid())
	{		
		ilBindImage(imageName);
		iluImageParameter(ILU_FILTER,ILU_BILINEAR);
		iluScale(newWidth,newHeight,1);
		width=ilGetInteger(IL_IMAGE_WIDTH);
		height=ilGetInteger(IL_IMAGE_HEIGHT);
		data=ilGetData();	
	}
}
template <class Pixel>
void Image::setPixel(int x,int y,const Pixel &c)
{
	
}
//void Image::createSurface(IDisplay *display)
//{	
//	if(isValid()&&!surface)
//		display->createSurface(&surface,width,height);
//	surfaceUpdated=false;
//}
//void Image::updateDDrawSurface()
//{
//	if(isValid() && surface && !surfaceUpdated)
//	{
//		PixelIterator<int8> p(getPixel<int8>(0,0));		
//		COLORREF clr;
//		char v;
//		surface->lockSurface();		
//		for(int y=0;y<height;y++)
//			for(int x=0;x<width;x++,p++)
//			{
//				v=*p;
//				clr=RGB(v,v,v);
//				surface->putPixel(x,y,clr);
//			}
//		surface->unLockSurface();
//		surfaceUpdated=true;
//	}
//}
//ISurface * Image::getSurface()
//{
//	return surface;
//}
int Image::getPixelFormat()
{
	return pixelFormat;
}

const char* Image::getName()
{
	return fileName;
}
int Image::imageCreate(int w,int h,int imageType)
{
	source=imageSourceMemory;
	ilGenImages(1,(ILuint*)&imageName);

	ilBindImage(imageName);
	bool res= (0!=ilTexImage(w,h,1,1,convertPixelTypeL2IL(pixelFormat),IL_UNSIGNED_BYTE,NULL) );	
	
	if(res)
	{		
		width=w;
		height=h;
		pixelFormat=imageType;	
		width=ilGetInteger(IL_IMAGE_WIDTH);
		height=ilGetInteger(IL_IMAGE_HEIGHT);
		
		valid=true;
	}
	else
		return 0;
	return 1;
}
int Image::loadImage(const char *file)
{	
	source=imageSourceFile;
	ilGenImages(1,(ILuint*)&imageName);
	ilBindImage(imageName);
	bool res= (0!=ilLoadImage(file));
	if(res)
	{		
//		surface=NULL;
		pixelFormat=convertPixelTypeIL2L(ilGetInteger(IL_IMAGE_FORMAT));
		ilConvertImage(convertPixelTypeL2IL(pixelFormat),IL_UNSIGNED_BYTE);
		fileName=new char[strlen((char*)file)+1];
		strcpy(fileName,file);
		width=ilGetInteger(IL_IMAGE_WIDTH);
		height=ilGetInteger(IL_IMAGE_HEIGHT);	
		valid=true;
	}
	else
		return 0;
	return 1;	
}
int Image::saveImage(const char *path)
{
	//USES_CONVERSION;
	//char *file=W2A(path);
	ilBindImage(imageName);
	if(ilSaveImage(path))			
		return 1;	
	ILenum err=ilGetError();
	return 0;
}

int copyImage(Image *dst,Image * src)
{
	int w=src->getWidth();
	int h=src->getHeight();
	dst->imageCreate(w,h,src->getType());
	//PixelIterator it_src<in=src->getPixel(0,0),it_dst=dst->getPixel(0,0);
	return 0;
}

int Image::save(std::ofstream &out)
{
	out<<documentPath;	
	return 0;
}
int Image::load(std::ifstream &in)
{
	char tmpPath[MAX_PATH];
	in>>tmpPath;
	loadImage(tmpPath);
	return 0;
}


int Image::imageCounter=0;


Filter::Filter(Workspace *ws)
:BaseDocument("generic filter",ws,documentTypeFilter)
{}

Filter::~Filter(void)
{}


void Filter::init()
{}


void Filter::applyTo(Image &source,Image &target)
{
	DWORD i=0;
	lastSource=&source;
	lastTarget=&target;
	

}

MatrixFilter::MatrixFilter(int filterSize,Workspace *ws)
:Filter(ws)
{
	size=filterSize;
	matrix=new float[(2*size+1)*(2*size+1)];
	lastSource=NULL;
	lastTarget=NULL;
}

MatrixFilter::~MatrixFilter()
{
	if(matrix)
		delete matrix;
}

//template<class Pixel>
void MatrixFilter::applyTo(Image &target,Image &source)
{
	int x1,y1;
	float r;
	PixelIterator<int8> trg=target.getPixel<int8>(0,0);
	PixelIteratorConst<int8> src;
	for(int y=0;y<source.getHeight();y++)
		for(int x=0;x<source.getWidth();x++,trg++)
		{
			src=source.getRect<int8>(x-size,y-size,x+size,y+size);
			RECT rc;
			src.getRect(rc);
			r=0;
			for(y1=rc.top-y;y1<rc.bottom-y;y1++)
				for(x1=rc.left-x;x1<rc.right-x;x1++,src++)
					r+=matrix[x1+size+(y1+size)*(size+size+1)]*(int8)(*src);
			*trg=(int8)r;
		}
}

void MatrixFilter::init()
{}
void MatrixFilter::setElementFn(FunctionXY &fn)
{
	//ASSERT(fn);
	for(int y=-size;y<size;y++)
		for(int x=-size;x<size;x++)
			setElement(x,y,fn((float)x,(float)y));
}
void MatrixFilter::setElement(signed int x,signed int y,float e)
{
	if(	x<-size || 
		x> size ||
		y<-size ||
		y> size)
		return;
	x+=size;
	y+=size;
	matrix[x+y*(size+size+1)]=e;
}
float & MatrixFilter::operator () (int x,int y)
{
	return matrix[x+size+(y+size)*(size+size+1)];
}
float MatrixFilter::getElement(signed int x,signed int y)
{
	if(	x<-size || 
		x> size ||
		y<-size ||
		y> size)
		return 0;
	x+=size;
	y+=size;
	return matrix[x+size+(y+size)*(size+size+1)];
}
int MatrixFilter::save(std::ofstream &out)
{
	out<<size;
	out.write((char *)matrix,sizeof(float)*(2*size+1)*(2*size+1));
	return 0;
}
int MatrixFilter::load(std::ifstream &in)
{
	in>>size;
	if(matrix)
		delete matrix;
	matrix=new float[(2*size+1)*(2*size+1)];
	in.read((char *)matrix,sizeof(float)*(2*size+1)*(2*size+1));
	return 0;
}

Operator::Operator(const char *name,Workspace *ws)
:BaseDocument(name,ws,documentTypeOperator)
{}
void Operator::init()
{
}
void Operator::setSource(Image *a, Image *b)
{
	lastSource[0]=a;
	lastSource[1]=b;
}
void Operator::setSource(Image *a[])
{
	lastSource[0]=a[0];
	lastSource[1]=a[1];
}
void Operator::setTarget(Image *target)
{
	lastTarget=target;
}

Correlation::Correlation(int s,float t,Workspace *ws)
:Operator("Correlation",ws)
{
	size=s;
	threshold=t;
	matrix=new float[(2*size+1)*(2*size+1)];
	for(int i=0;i<(2*size+1)*(2*size+1);i++)
		matrix[i]=0;
}
Correlation::~Correlation()
{
	if(matrix)
		delete matrix;
}
void  Correlation::setElement(signed int x,signed int y,float e)
{
	if(	x<-size || 
		x> size ||
		y<-size ||
		y> size)
		return;
	x+=size;
	y+=size;
	matrix[x+y*(size+size+1)]=e;
}
float  Correlation::getEletemt(signed int x,signed int y)
{
	if(	x<-size || 
		x> size ||
		y<-size ||
		y> size)
		return 0;
	x+=size;
	y+=size;
	return matrix[x+y*(size+size+1)];
}
void Correlation::set(float c)
{
	for(int i=0;i<(2*size+1)*(2*size+1);i++)
		matrix[i]=c;
}
float Correlation::sum()
{
	float res=0;
	for(int i=0;i<(2*size+1)*(2*size+1);i++)
		res+=matrix[i];
	return res;
}
void Correlation::init(int s,float t)
{
	size=s;
	threshold=t;
	if(matrix)
		delete matrix;
	matrix=new float[(2*size+1)*(2*size+1)];
	for(int i=0;i<(2*size+1)*(2*size+1);i++)
		matrix[i]=0;
}
int Correlation::save(std::ofstream &out)
{
	out<<size;
	out<<threshold;
	return 1;
}
int Correlation::load(std::ifstream &in)
{
	in>>size;
	in>>threshold;
	init(size,threshold);
	return 1;
}

void Correlation::apply()
{
	if(!lastSource[0])
		return;
	if(!lastSource[1])
		return;
	if(!lastTarget)
		return;
	if(	lastTarget->getWidth()!=lastSource[0]->getWidth()||
		lastTarget->getWidth()!=lastSource[1]->getWidth()||
		lastTarget->getHeight()!=lastSource[0]->getHeight()||
		lastTarget->getHeight()!=lastSource[1]->getHeight())
		return;
	int width=lastSource[0]->getWidth();
	int height=lastSource[0]->getHeight();
	PixelIterator<int8> t=lastTarget->getPixel<int8>(0,0);


	signed int x,y;
	signed int x1,y1;
	signed int dx,dy;
	signed int ye;
	int length=0;
	float val=0;

	for(y=0;y<height;y++)
	{		
		ye=MIN(y+size,height-1);		
		for(x=0;x<width;x++,t++)
		{
			y1=MAX(y-size,0);
			x1=MAX(x-size,0);
			int xe=MIN(x+size,width-1);
			
			val=0;
			length=0;

			PixelIterator<int8> s0=lastSource[0]->getRect<int8>(x1,y1,xe,ye);
			PixelIterator<int8> s1=lastSource[1]->getRect<int8>(x1,y1,xe,ye);

			for(;y1<=ye;y1++)
			{
				for(;x1<=xe;x1++)	
				{
					length++;										
					float diff= *s1- *s0;
					val+=fabs(diff);								
				}			
			}	
			float k=(val/length);
			if(threshold>0)
				k=(k>threshold)?255:0;						
			*t=(int8)k;
		}
	}
>>>>>>> bd17e7de82f5e0c1302ffcdcf53da1a9448d3f2d
}