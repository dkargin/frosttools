#pragma once

#ifdef _UNICODE
#undef _UNICODE
#include "il\il.h"
#include "il\ilu.h"
#include "il\ilut.h"
#define _UNICODE
#else
#include "il\il.h"
#include "il\ilu.h"
#include "il\ilut.h"
#endif
//#include "..\\fastDraw\\fastdraw.h"
#include "mathTools.h"
//#include "dataRenderer.h"

typedef unsigned char int8;

enum PixelType
{	
	pixelTypeRGB,		//3 Red\Green\Blue bytes
	pixelTypeRGBA,		//4 Reg\Greeb\Blue bytes
	pixelTypeLuminance,	//1 grayscale byte
	pixelTypeUnsupported
};

enum PixelSize
{
	pixelSizeRGB=3,		
	pixelSizeRGBA=4,	
	pixelSizeLuminance=1
};
const int pixelSizes[]=
{
	pixelSizeRGB,
	pixelSizeRGBA,
	pixelSizeLuminance
};

enum imageSource
{
	imageSourceInvalid,		// no image data
	imageSourceFile,		// loaded from file
	imageSourceMemory		// created in memory
};
enum rectMode
{
	rectModeOff,
	rectModeClose,
	rectModeLoop,
	rectModeThorus,
	rectModeMirror,
};

#include "Iterators\PixelIterators.h"

class Image : public BaseDocument
{	
public:
	Image(Workspace *ws);
	~Image(void);
	Image(const char *path,Workspace *ws,const char *Name=NULL);
	Image(int width,int height,int imageType,Workspace *ws);

	int getWidth();
	int getHeight();
	int getPixelFormat();

	template<class Pixel>
	inline PixelIterator<Pixel> getPixel(int x,int y);
	template<class Pixel>
	inline PixelIterator<Pixel> getRect(int let,int top,int right,int bottom);
	
	bool isValid();
	void setSize(int w,int h);
	template<class Pixel>
	void setPixel(int x,int y,const Pixel &c);
//	void createSurface(IDisplay *display);

	void updateDDrawSurface();
	void convertFormat(int newType);
	//ISurface *getSurface();

	int imageCreate(int width,int height,int imageType);
	//virtual BaseView * createView(MainFrame *frm);
	int loadImage(const char *file);
	int saveImage(const char *file);
	int load(std::ifstream &in);
	int save(std::ofstream &out);

	const char * getName();
	void resize(int newWidth,int newHeight);
protected:
	int pixelFormat;
	int surfaceUpdated;
	int valid;

	static int imageCounter;
	//ISurface *surface;

	int width;
	int height;

	int source;

	int imageName;	//IL image name
	char *fileName;	//source name
	int8 *data;	
};

class Filter: public BaseDocument
{
protected:
	Image *lastSource;
	Image *lastTarget;
	//virtual PixelIterator getPixelResult(int x,int y);
public:
	Image *getLastTarget();
	Image *getLastSource();
	Filter(Workspace *ws);
	~Filter(void);
	virtual void init();
	virtual void applyTo(Image & source,Image &target);	
};

class MatrixFilter: public Filter
{
	int size;
	float *matrix;//square matrix [2*size+1]x[2*size+1]
	//virtual PixelIterator getPixelResult(int x,int y);
public:
	MatrixFilter(int filterSize,Workspace *ws);
	~MatrixFilter();
	virtual void init();

	virtual void applyTo(Image & source,Image &target);

	virtual void setElementFn(FunctionXY &fn); 

	virtual void setElement(signed int x,signed int y,float e);
	virtual float getElement(signed int x,signed int y);
	float & operator()(int x,int y); 
	//virtual float getSum();
	//virtual float getMin(int &x,int &y);

	int save(std::ofstream &out);
	int load(std::ifstream &in);
	//virtual BaseView * createView(MainFrame *frm);
};

class ImageContainer
{
	std::list<Image*> images;
public:
	void addImage(Image *img);
	void clear();
};

class Operator
	: public BaseDocument
{
public:
	Operator(const char *name,Workspace *ws);
	Image *lastSource[2];
	Image *lastTarget;
public:
	virtual void init();
	virtual void apply(){};
	virtual void setSource(Image *a,Image *b);
	virtual void setSource(Image *a[2]);
	virtual void setTarget(Image *target);
};

class Correlation: public Operator
{
	int size;
	float *matrix;
	float threshold;

	void setElement(signed int x,signed int y,float e);
	float getEletemt(signed int x,signed int y);
	float sum();
	void set(float c);
public:
	Correlation(int size,float t,Workspace *ws);
	~Correlation();
	virtual void init(int size,float t);
	virtual void apply();
	int save(std::ofstream &out);
	int load(std::ifstream &in);
	//virtual BaseView * createView(MainFrame *frm);
};

class OpticFlow: public Operator
{
	int size;
	float matrix;
public:
	void init(int filterSize);	
	virtual void apply();
	int save(std::ofstream &out);
	int load(std::ifstream &in);
	//virtual BaseView * createView(MainFrame *frm);
};
