#include "stdafx.h"

#include "../unsorted.hpp"
DistanceTable<char,255,int> distanceTable;

FormatDesc getFormat(const Image &image)
{
	FormatDesc res;
	res.width=image.width();
	res.height=image.height();
	res.depth=image.depth();
	res.channels=image.channels();
	return res;
}
FormatDesc getFormat(const IplImage *image)
{
	FormatDesc res;
	res.width=image->width;
	res.height=image->height;
	res.depth=image->depth;
	res.channels=image->nChannels;
	return res;
}
FormatDesc getFormat(const ImageView &view)
{
	FormatDesc res;
	res.width=view.x_size();
	res.height=view.y_size();
	res.channels=view.parent->channels();
	res.depth=view.parent->depth();
	return res;
}

IplImage * create(const FormatDesc &desc)
{
	return cvCreateImage(cvSize(desc.width,desc.height),desc.depth,desc.channels);
}
///////////////////////////////////////////////////////////////////////////////////
// Image
///////////////////////////////////////////////////////////////////////////////////
Image::~Image(void)
{
	reset();
}

Image::Image(const std::string& file,const TCHAR *Name)
:image(NULL),state(Invalid)
{	
	state=Unloaded;
	fileName=file;
}

Image::Image(const FormatDesc &format)
:image(NULL),state(Invalid)
{
	image=create(format);
	if(image)
		state=Bitmap;
	else
		state=Invalid;
	update();
}

Image::Image(const Image &img)
:image(NULL),state(Invalid)
{
	if(img.state==Bitmap || img.state==Changed)
	{
		image=cvCloneImage(img.image);
		state=Bitmap;
	}
	else if(img.state==Unloaded)
	{
		fileName=img.fileName;
		state=Unloaded;
	}
	else if(img.state==Loaded)
	{
		fileName=img.fileName;
		state=Unloaded;
		load();
	}
	update();
}
//tolua_end
Image::Image(const ImageView &view)
:image(NULL),state(Invalid)
{
	image=create(getFormat(view));
	int x=view.x_size();
	int y=view.y_size();
	
	//imageCreate(x,y,view.parent->pixelFormat);

	if(image)
	{		
		//pixelFormat=formatLuminance;	
		ImageView/*<int8>*/::iterator it=view.begin();
		ImageView/*<int8>*/::iterator end=view.end();
		ImageView/*<int8>*/ full(*this,0,0,x,y);
		ImageView/*<int8>*/::iterator trg=full.begin();
		int j=full.x_size()*full.y_size();
		for(;it!=end;it++,trg++,j--)
			*trg=*it;			
		update();
		state=Bitmap;
	}
	else
		state=Invalid;
	//return state!=Invalid;
}
bool Image::isValid() const
{
	return image;
}
uint32 Image::width() const
{
	return (image)?(image->width):0;
}
uint32 Image::height() const
{
	return (image)?(image->height):0;
}

uint32 Image::channels() const
{
	return (image)?(image->nChannels):0;
}
uint32 Image::depth() const 
{
	return (image)?(image->depth):0;
}
Image::operator IplImage * ()
{
	return getCore();
}
IplImage * Image::getCore()
{
	load();
	return image;
}
ImageView Image::getView(int cx, int cy, int width, int height)
{
	load();
	ImageView res(*this,cx,cy,width,height);		
	return res;
}	
ImageView Image::getView(_AABB<vec2i> rect)
{
	load();
	ImageView res(*this,rect.min(0),rect.min(1),rect.size(0),rect.size(1));
	return res;
}
ImageView Image::getView(_AABB<vec2f> rect)
{
	load();
	ImageView res(*this,rect.min(0),rect.min(1),rect.size(0),rect.size(1));
	return res;
}
ImageView Image::getSafeView(_AABB<vec2i> rect)
{
	load();
	int minx=MAX(rect.min(0),0);
	int miny=MAX(rect.min(1),0);
	int maxx=MIN(rect.max(0),width());
	int maxy=MIN(rect.max(1),height());
	ImageView res(*this,minx,miny,maxx-minx,maxy-miny);
	return res;
}
ImageView Image::getSafeView(_AABB<vec2f> rect)
{
	load();
	int minx=MAX(rect.min(0),0);
	int miny=MAX(rect.min(1),0);
	int maxx=MIN(rect.max(0),width());
	int maxy=MIN(rect.max(1),height());
	ImageView res(*this,minx,miny,maxx-minx,maxy-miny);
	return res;
}
ImageView & Image::getFullView()
{
	load();
	return fullView;
}

void Image::resize(int nWidth, int nHeight)
{
	load();
	if(isValid())
	{		
		IplImage *tmp=cvCreateImage(cvSize(nWidth,nHeight),image->depth,image->nChannels);
		cvResize(image,tmp,CV_INTER_CUBIC);	
		cvReleaseImage(&image);
		image=tmp;
		if(state==Loaded)state=Changed;
		update();
	}
}
void Image::reset()
{
	if(image)
	{
		cvReleaseImage(&image);
		image=NULL;
		if(state==Loaded || state==Changed || state==Bitmap)
			state=Invalid;
	}
}
void Image::update()
{
	if(image)
		fullView=ImageView/*<int8>*/(*this,0,0,width(),height());
}

int Image::load()
{
	if(state==Unloaded)
	{
		//std::cout<<"Loading image from file<"<<fileName.c_str()<<">\n";
		reset();
		IplImage *tmp=cvLoadImage(fileName.c_str());	
		if(!tmp)
			return 0;
		//std::cout<<"-width="<<tmp->width<<"\n-height="<<tmp->height<<"\n";
		// convert image to grayscale
		image=cvCreateImage(cvSize(tmp->width,tmp->height),IPL_DEPTH_8U, 1);
		cvCvtColor(tmp, image, CV_BGR2GRAY);
		cvReleaseImage(&tmp);		
		if(image!=NULL)
		{
			state=Loaded;
			//std::cout<<"-sucsess\n";
			update();
		}
		else
		{
			state=Invalid;
		}		
		return image!=NULL;	
	}
}
//int Image::loadImage(char const *fileN)
//{	
//	std::cout<<"Loading image from file<"<<fileN<<">\n";
//	reset();
//	fileName=fileN;
//	source=imageSourceFile;	
//	IplImage *tmp=cvLoadImage(fileN);	
//	if(!tmp)
//		return 0;
//	std::cout<<"-width="<<tmp->width<<"\n-height="<<tmp->height<<"\n";
//	image=cvCreateImage(cvSize(tmp->width,tmp->height),IPL_DEPTH_8U, 1);
//	cvCvtColor(tmp, image, CV_BGR2GRAY);
//	//cvReleaseImage(&tmp);		
//	valid=(image!=NULL);
//	update();
//	std::cout<<"-sucsess\n";
//	return valid;		
//}
int Image::saveImage(const char*path)
{	
	//std::cout<<"Saving image to file <"<<path<<">\n";
	this->getCore();
	int res=cvSaveImage(path,image);
	fileName=path;	
	state=Loaded;
	return res;
}
void Image::rotate(float x,float y,float angle)
{
	CvMat *rot=cvCreateMat(2,3,CV_32FC1); 
	cv2DRotationMatrix(cvPoint2D32f(x,y),angle,1,rot);
	IplImage *tmp=cvCreateImage(cvSize(width(),height()),IPL_DEPTH_8U, 1);
	cvWarpAffine(image,tmp,rot);
	cvCopy(tmp,image);
	cvReleaseImage(&tmp);
	cvReleaseMat(&rot);
}

void Image::differential()
{	
	Image *tmp[2];
	load();
	tmp[0]=new Image(FormatDesc(width(),height()));
	tmp[1]=new Image(FormatDesc(width(),height()));

	cvSobel(image,tmp[0]->image,1,0);
	cvSobel(image,tmp[1]->image,0,1);
	ImageView/*<int8>*/ trg=getFullView();	
	ImageView/*<int8>*/::iterator it_src0,it_src1;
	ImageView/*<int8>*/::iterator it_trg;
	ImageView/*<int8>*/::iterator it_end;

	ImageView/*<int8>*/ src0=tmp[0]->getFullView();
	ImageView/*<int8>*/ src1=tmp[1]->getFullView();

	it_src0=src0.begin();
	it_src1=src1.begin();
	it_trg=trg.begin();
	it_end=trg.end();

	for(;it_trg!=it_end;it_src0++,it_src1++,it_trg++)
	{
		int v0=*it_src0;
		int v1=*it_src1;		
		*it_trg=distanceTable(v0,v1);
	}
	if(state==Loaded)
		state=Changed;
	delete tmp[0];
	delete tmp[1];
}