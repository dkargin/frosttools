<<<<<<< HEAD
#pragma once
//#include "..\mathTools\chrono.h"
#include <cv.h>

#define FrostTools_Use_Types
#include "../frostTools.h"
#include "PixelIterators.h"
//tolua_begin
enum PixelFormat
{	
	formatRGB,		//3 Red\Green\Blue bytes
	formatRGBA,		//4 Reg\Greeb\Blue bytes
	formatLuminance,	//1 grayscale byte
	formatFloat,		// single float32 value
	formatGrayscale=formatLuminance,
	formatUnsupported
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

struct FormatDesc
{
	int width;
	int height;
	int depth;
	int channels;
	FormatDesc():width(0),height(0),depth(0),channels(0){}
	FormatDesc(int w,int h,int c=1,int d=8):width(w),height(h),depth(d),channels(c){};
	FormatDesc(const FormatDesc &desc):width(desc.width),height(desc.height),depth(desc.depth),channels(desc.channels){}
};

FormatDesc getFormat(const IplImage *image);
FormatDesc getFormat(const Image &image);

bool operator==(const FormatDesc &a,const FormatDesc &b);

//tolua_end

//tolua_begin
class Image// : public BaseDocument
{	
	//tolua_end
private:
	ImageView fullView;
	int imageCreate(int width,int height,int imageType);
	int imageCreate(const ImageView &view);
	//bool copyImage(const Image *r);
	void reset();
public:
	enum State
	{
		Invalid=0,
		Loaded,		// corresponding file is loaded
		Changed,	// we've done some changes with image
		Unloaded,	// image is not loaded now
		Bitmap
	};
	IplImage * getCore();
	int load();
	void unload();
	//tolua_begin
	
	~Image();
	Image(const std::string &path,const TCHAR *Name=NULL);
	Image(const FormatDesc &format);
	Image(const Image &img);
	//tolua_end
	Image(const ImageView &view);
	//tolua_begin
	virtual uint32 width() const;
	virtual uint32 height() const;
	virtual uint32 depth() const;
	virtual uint32 channels() const;
	template<class T> void resize(const Vector2D<T> &v)
	{
		resize(v[0],v[1]);
	}
	void resize(int w,int h);
	//tolua_end
	operator IplImage * ();

	ImageView getView(int cx, int cy, int width, int height);
	ImageView getView(_AABB<vec2f> rect);
	ImageView getView(_AABB<vec2i> rect);
	// get bound clipped view
	ImageView getSafeView(_AABB<vec2i> rect);
	ImageView getSafeView(_AABB<vec2f> rect);
	ImageView & getFullView();
	bool isValid() const;	

	void fit(const Image * img);
	virtual void update();

	//tolua_begin
	
	int saveImage(const char *file);
	
	void differential();
	void rotate(float x,float y,float angle);
	//tolua_end
	const char * getName()const{ return fileName.c_str();}
	std::string fileName;	//source name
protected:
	IplImage *image;
	State state;
	//int loadImage();
};


class FilterManager
{
	typedef std::list<Image> Container;
	Container images;
public:
	FilterManager();
	~FilterManager();	
	Container::iterator find(const FormatDesc &format);
	Image & queryImage(const FormatDesc &format);
};


class Filter
{
protected:
	static FilterManager filterManager[2];
public:		
	Image & operator ()(Image &source)
	{
		Image *target=&filterManager[0].queryImage(getResultFormat(source));
		if(target==&source)
			target=&filterManager[1].queryImage(getResultFormat(source));
		applyTo(source,*target);
		return *target;
	}
	virtual FormatDesc getResultFormat(Image &source)
	{
		return getFormat(source);
	}
	virtual void applyTo(Image &source,Image &target)=0;	
	friend Image & operator * (Filter &filter,Image &image); 
};

inline Image & operator * (Filter &filter,Image &source)
{	
	return filter(source);	
}

class Binarisation: public Filter
{
protected:
	vec3 scale;
	float min,max;
public:
	Binarisation(float *scale,float smin,float smax);
	virtual void applyTo(Image &source,Image &target);
	virtual FormatDesc getResultFormat(Image &source)
	{
		FormatDesc res=getFormat(source);		
		res.depth=8;
		res.channels=1;
		return res;
	}
};
class Invert: public Filter
{
	int max;
public:
	Invert(int m);
	virtual void applyTo(Image &source,Image &target);
};
class MatrixFilter: public Filter
{
	int size;
	float *matrix;
public:
	MatrixFilter(int filterSize,const float *data);
	~MatrixFilter();
	virtual void init();

	virtual void applyTo(Image &source,Image &target);

	virtual void setElement(signed int x,signed int y,float e);
	virtual float getElement(signed int x,signed int y);
	float & operator()(int x,int y); 	

	int save(std::ofstream &out);
	int load(std::ifstream &in);
};

class FilterCopy: public Filter
{
public:
	virtual void applyTo(Image &source,Image &target)
	{
		target=source;
	}
};

class Smooth: public Filter
{
	int type;
	int size[2];
	float threshold[2];
public:
	Smooth(int type,int sz0,int sz1,float thres0,float thres1);
	virtual void init(int type,int sz0,int sz1,float thres0,float thres1);
	virtual void applyTo(Image &source,Image &target);
};

class FilterGrow: public Filter
{
	int value;
public:
	FilterGrow(int val);
	virtual void applyTo(Image &source,Image &target);
};

class BrightnessContrast:public Filter
{
	CvHistogram *hist;
	CvMat* lut_mat;
	uchar lut[256];
	int hist_size;
	
	float brightness;	// [-1.0,1.0]
	float contrast;		// [-1.0,1.0]	
public:
	BrightnessContrast(int size=64);
	~BrightnessContrast();
	void set(float b,float c);
	virtual void applyTo(Image &source,Image &target);
	virtual void draw(IplImage *image);
};
class Operator
	//: public BaseDocument
{
public:
	Operator(const char *name/*,Workspace *ws*/);
	ImageView/*<int8>*/ *lastSource[2];
	ImageView/*<int8>*/ *lastTarget;
public:
	virtual void init();
	virtual void apply()=0;
	virtual void setSource(ImageView/*<int8>*/ &a,ImageView/*<int8>*/ &b);
	//virtual void setSource(ImageView/*<int8>*/ *a[2]);
	virtual void setTarget(ImageView/*<int8>*/ &target);
	//virtual void process(Image *a,Image *b);
};

class Correlation: public Operator
{
	int size;
	int stride;
	float *matrix;
	float threshold;
	//Chrono chrono;

	void setElement(signed int x,signed int y,float e);
	float getEletemt(signed int x,signed int y);
	float sum();
	void set(float c);
public:
	Correlation(int size,int stride,float t/*,Workspace *ws*/);
	~Correlation();
	virtual void init(int size,float t);
	//virtual Image & oper
	virtual void apply();
	int save(std::ofstream &out);
	int load(std::ifstream &in);
};

class Correlation2: public Operator
{
public:
	Correlation2(float t/*,Workspace *ws*/);
	~Correlation2();
	virtual void init(int size,float t);
	virtual void apply();
};

class OpticFlow: public Operator
{
	int winSize;	
public:
	OpticFlow(int size);
	void init(int filterSize);	
	virtual void apply();
	int save(std::ofstream &out);
	int load(std::ifstream &in);
};

class Hypot: public Operator
{
public:
	Hypot()
		:Operator("distance")
	{}
	virtual void apply();
};

Image buildIntegralImage(const Image *source);

class IntegralImage
{
public:
	typedef unsigned int value_type;
	IntegralImage();
	IntegralImage(ImageView/*<int8>*/ &view);
	~IntegralImage();	
	void setImage(ImageView/*<int8>*/ &view);
	value_type getRectSum(int x,int y,int cx,int cy);	
	int x_size();	// get width
	int y_size();	// get height
	//void update();	// recalculate image
	value_type & operator ()(int x,int y);
	void saveImage(const char *path);
protected:	
	IplImage *image;
};

/////////////////////////////////////////////////////////////////////////////
// Haar feature element
/////////////////////////////////////////////////////////////////////////////
struct HaarRectDef
{
	int left;
	int right;
	int top;
	int bottom;
	float weight;

	HaarRectDef();
	HaarRectDef(const HaarRectDef & def);
	HaarRectDef(int left,int top,int right,int bottom,bool sign);

	void setRect(int left,int top,int right,int bottom,bool sign);
	bool pointInRect(int x,int y);
	inline int area() const
	{
		return (bottom-top)*(right-left);
	}
};

std::wostream & operator<<(std::wostream & out, const HaarRectDef &def);
std::wistream & operator>>(std::wistream & in, HaarRectDef &def);

class HaarFeature
{
public:
	static const int rectCountMax=4;
	HaarRectDef rect[rectCountMax];

	int rectCount;
	signed int sign;
	float threshold;

	int width,height;
	int test(float value);	// threshold test
public:
	HaarFeature():
		rectCount(0),
		sign(0),
		threshold(0),
		width(0),
		height(0)
	{}
	HaarFeature(HaarRectDef *def,int count,int w,int h);
	HaarFeature(const HaarFeature &feature):
		rectCount(feature.rectCount),
		sign(feature.sign),
		threshold(feature.threshold),
		width(feature.width),
		height(feature.height)
	{
		for(int i=0;i<rectCountMax;i++)
			rect[i]=feature.rect[i];
	}
	void addRect(const HaarRectDef &def);
	void setSize(int w,int h);
	float value(IntegralImage &image,int x,int y);
	int test(IntegralImage &image,int x,int y);
	// Training weak classifier. Parameters: 
	// x		- item from training set
	// y		- item classification: 0 - nonobjct, 1 - object
	// count	- size of training
	// Result : calculating threshold,sign,error
	void train(IntegralImage *x,float * values, char *y, int count);	// calculate threshold and sign
	void draw(ImageView/*<int8>*/ &view);
	bool getCoord(int x,int y);	
};

std::ostream & operator<<(std::ostream & out, const HaarFeature &def);
std::istream & operator>>(std::istream & in, HaarFeature &def);


typedef ImageView/*<int8>*/ ImageView_b;
=======
#pragma once
//#include "..\mathTools\chrono.h"
#include <cv.h>

#define FrostTools_Use_Types
#include "../frostTools.h"
#include "PixelIterators.h"
//tolua_begin
enum PixelFormat
{	
	formatRGB,		//3 Red\Green\Blue bytes
	formatRGBA,		//4 Reg\Greeb\Blue bytes
	formatLuminance,	//1 grayscale byte
	formatFloat,		// single float32 value
	formatGrayscale=formatLuminance,
	formatUnsupported
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

struct FormatDesc
{
	int width;
	int height;
	int depth;
	int channels;
	FormatDesc():width(0),height(0),depth(0),channels(0){}
	FormatDesc(int w,int h,int c=1,int d=8):width(w),height(h),depth(d),channels(c){};
	FormatDesc(const FormatDesc &desc):width(desc.width),height(desc.height),depth(desc.depth),channels(desc.channels){}
};

FormatDesc getFormat(const IplImage *image);
FormatDesc getFormat(const Image &image);

bool operator==(const FormatDesc &a,const FormatDesc &b);

//tolua_end

//tolua_begin
class Image// : public BaseDocument
{	
	//tolua_end
private:
	ImageView fullView;
	int imageCreate(int width,int height,int imageType);
	int imageCreate(const ImageView &view);
	//bool copyImage(const Image *r);
	void reset();
public:
	enum State
	{
		Invalid=0,
		Loaded,		// corresponding file is loaded
		Changed,	// we've done some changes with image
		Unloaded,	// image is not loaded now
		Bitmap
	};
	IplImage * getCore();
	int load();
	void unload();
	//tolua_begin
	
	~Image();
	Image(const std::string &path,const TCHAR *Name=NULL);
	Image(const FormatDesc &format);
	Image(const Image &img);
	//tolua_end
	Image(const ImageView &view);
	//tolua_begin
	virtual uint32 width() const;
	virtual uint32 height() const;
	virtual uint32 depth() const;
	virtual uint32 channels() const;
	template<class T> void resize(const Vector2D<T> &v)
	{
		resize(v[0],v[1]);
	}
	void resize(int w,int h);
	//tolua_end
	operator IplImage * ();

	ImageView getView(int cx, int cy, int width, int height);
	ImageView getView(_AABB<vec2f> rect);
	ImageView getView(_AABB<vec2i> rect);
	// get bound clipped view
	ImageView getSafeView(_AABB<vec2i> rect);
	ImageView getSafeView(_AABB<vec2f> rect);
	ImageView & getFullView();
	bool isValid() const;	

	void fit(const Image * img);
	virtual void update();

	//tolua_begin
	
	int saveImage(const char *file);
	
	void differential();
	void rotate(float x,float y,float angle);
	//tolua_end
	const char * getName()const{ return fileName.c_str();}
	std::string fileName;	//source name
protected:
	IplImage *image;
	State state;
	//int loadImage();
};


class FilterManager
{
	typedef std::list<Image> Container;
	Container images;
public:
	FilterManager();
	~FilterManager();	
	Container::iterator find(const FormatDesc &format);
	Image & queryImage(const FormatDesc &format);
};


class Filter
{
protected:
	static FilterManager filterManager[2];
public:		
	Image & operator ()(Image &source)
	{
		Image *target=&filterManager[0].queryImage(getResultFormat(source));
		if(target==&source)
			target=&filterManager[1].queryImage(getResultFormat(source));
		applyTo(source,*target);
		return *target;
	}
	virtual FormatDesc getResultFormat(Image &source)
	{
		return getFormat(source);
	}
	virtual void applyTo(Image &source,Image &target)=0;	
	friend Image & operator * (Filter &filter,Image &image); 
};

inline Image & operator * (Filter &filter,Image &source)
{	
	return filter(source);	
}

class Binarisation: public Filter
{
protected:
	vec3 scale;
	float min,max;
public:
	Binarisation(float *scale,float smin,float smax);
	virtual void applyTo(Image &source,Image &target);
	virtual FormatDesc getResultFormat(Image &source)
	{
		FormatDesc res=getFormat(source);		
		res.depth=8;
		res.channels=1;
		return res;
	}
};
class Invert: public Filter
{
	int max;
public:
	Invert(int m);
	virtual void applyTo(Image &source,Image &target);
};
class MatrixFilter: public Filter
{
	int size;
	float *matrix;
public:
	MatrixFilter(int filterSize,const float *data);
	~MatrixFilter();
	virtual void init();

	virtual void applyTo(Image &source,Image &target);

	virtual void setElement(signed int x,signed int y,float e);
	virtual float getElement(signed int x,signed int y);
	float & operator()(int x,int y); 	

	int save(std::ofstream &out);
	int load(std::ifstream &in);
};

class FilterCopy: public Filter
{
public:
	virtual void applyTo(Image &source,Image &target)
	{
		target=source;
	}
};

class Smooth: public Filter
{
	int type;
	int size[2];
	float threshold[2];
public:
	Smooth(int type,int sz0,int sz1,float thres0,float thres1);
	virtual void init(int type,int sz0,int sz1,float thres0,float thres1);
	virtual void applyTo(Image &source,Image &target);
};

class FilterGrow: public Filter
{
	int value;
public:
	FilterGrow(int val);
	virtual void applyTo(Image &source,Image &target);
};

class BrightnessContrast:public Filter
{
	CvHistogram *hist;
	CvMat* lut_mat;
	uchar lut[256];
	int hist_size;
	
	float brightness;	// [-1.0,1.0]
	float contrast;		// [-1.0,1.0]	
public:
	BrightnessContrast(int size=64);
	~BrightnessContrast();
	void set(float b,float c);
	virtual void applyTo(Image &source,Image &target);
	virtual void draw(IplImage *image);
};
class Operator
	//: public BaseDocument
{
public:
	Operator(const char *name/*,Workspace *ws*/);
	ImageView/*<int8>*/ *lastSource[2];
	ImageView/*<int8>*/ *lastTarget;
public:
	virtual void init();
	virtual void apply()=0;
	virtual void setSource(ImageView/*<int8>*/ &a,ImageView/*<int8>*/ &b);
	//virtual void setSource(ImageView/*<int8>*/ *a[2]);
	virtual void setTarget(ImageView/*<int8>*/ &target);
	//virtual void process(Image *a,Image *b);
};

class Correlation: public Operator
{
	int size;
	int stride;
	float *matrix;
	float threshold;
	//Chrono chrono;

	void setElement(signed int x,signed int y,float e);
	float getEletemt(signed int x,signed int y);
	float sum();
	void set(float c);
public:
	Correlation(int size,int stride,float t/*,Workspace *ws*/);
	~Correlation();
	virtual void init(int size,float t);
	//virtual Image & oper
	virtual void apply();
	int save(std::ofstream &out);
	int load(std::ifstream &in);
};

class Correlation2: public Operator
{
public:
	Correlation2(float t/*,Workspace *ws*/);
	~Correlation2();
	virtual void init(int size,float t);
	virtual void apply();
};

class OpticFlow: public Operator
{
	int winSize;	
public:
	OpticFlow(int size);
	void init(int filterSize);	
	virtual void apply();
	int save(std::ofstream &out);
	int load(std::ifstream &in);
};

class Hypot: public Operator
{
public:
	Hypot()
		:Operator("distance")
	{}
	virtual void apply();
};

Image buildIntegralImage(const Image *source);

class IntegralImage
{
public:
	typedef unsigned int value_type;
	IntegralImage();
	IntegralImage(ImageView/*<int8>*/ &view);
	~IntegralImage();	
	void setImage(ImageView/*<int8>*/ &view);
	value_type getRectSum(int x,int y,int cx,int cy);	
	int x_size();	// get width
	int y_size();	// get height
	//void update();	// recalculate image
	value_type & operator ()(int x,int y);
	void saveImage(const char *path);
protected:	
	IplImage *image;
};

/////////////////////////////////////////////////////////////////////////////
// Haar feature element
/////////////////////////////////////////////////////////////////////////////
struct HaarRectDef
{
	int left;
	int right;
	int top;
	int bottom;
	float weight;

	HaarRectDef();
	HaarRectDef(const HaarRectDef & def);
	HaarRectDef(int left,int top,int right,int bottom,bool sign);

	void setRect(int left,int top,int right,int bottom,bool sign);
	bool pointInRect(int x,int y);
	inline int area() const
	{
		return (bottom-top)*(right-left);
	}
};

std::wostream & operator<<(std::wostream & out, const HaarRectDef &def);
std::wistream & operator>>(std::wistream & in, HaarRectDef &def);

class HaarFeature
{
public:
	static const int rectCountMax=4;
	HaarRectDef rect[rectCountMax];

	int rectCount;
	signed int sign;
	float threshold;

	int width,height;
	int test(float value);	// threshold test
public:
	HaarFeature():
		rectCount(0),
		sign(0),
		threshold(0),
		width(0),
		height(0)
	{}
	HaarFeature(HaarRectDef *def,int count,int w,int h);
	HaarFeature(const HaarFeature &feature):
		rectCount(feature.rectCount),
		sign(feature.sign),
		threshold(feature.threshold),
		width(feature.width),
		height(feature.height)
	{
		for(int i=0;i<rectCountMax;i++)
			rect[i]=feature.rect[i];
	}
	void addRect(const HaarRectDef &def);
	void setSize(int w,int h);
	float value(IntegralImage &image,int x,int y);
	int test(IntegralImage &image,int x,int y);
	// Training weak classifier. Parameters: 
	// x		- item from training set
	// y		- item classification: 0 - nonobjct, 1 - object
	// count	- size of training
	// Result : calculating threshold,sign,error
	void train(IntegralImage *x,float * values, char *y, int count);	// calculate threshold and sign
	void draw(ImageView/*<int8>*/ &view);
	bool getCoord(int x,int y);	
};

std::ostream & operator<<(std::ostream & out, const HaarFeature &def);
std::istream & operator>>(std::istream & in, HaarFeature &def);


typedef ImageView/*<int8>*/ ImageView_b;
>>>>>>> bd17e7de82f5e0c1302ffcdcf53da1a9448d3f2d
// PS: некрасиво выглядит этот класс