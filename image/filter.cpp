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
#include "stdafx.h"


#ifdef _DEBUG
#pragma comment(lib,"cv.lib")
#pragma comment(lib,"cxcore.lib")
#pragma comment(lib,"cvaux.lib")
#pragma comment(lib,"highgui.lib")
#else
#pragma comment(lib,"cv.lib")
#pragma comment(lib,"cxcore.lib")
#pragma comment(lib,"cvaux.lib")
#pragma comment(lib,"highgui.lib")
#endif
#ifdef _CH_
#pragma package <opencv>
#endif
#include <highgui.h>

//#include "..\frostTools\frostTools.h"


using namespace std;
int clamp(int min,int max,int value)
{
	if(value<min)
		return min;
	if(value>max)
		return max;
	return value;
}

bool areCompatible(Image *a,Image *b)
{
	return (a && 
			b && 
			a->isValid() && 
			b->isValid() &&
			a->width()==b->width()&&
			a->height()==b->height() && 
			a->depth()==b->depth() &&
			a->channels()==b->channels());	
}


/////////////////////////////////////////////////////////
// FilterManager
/////////////////////////////////////////////////////////
FilterManager Filter::filterManager[2];

FilterManager::FilterManager()
{
}

FilterManager::~FilterManager()
{
}

bool operator==(const FormatDesc &a,const FormatDesc &b)
{
	return	a.width==b.width && 
			a.height==b.height &&
			a.depth==b.depth &&
			a.channels==b.channels;
}

FilterManager::Container::iterator FilterManager::find(const FormatDesc &format)
{
	Container::iterator it=images.begin();
	while(it!=images.end())
	{
		if(getFormat(*it)==format)
			break;
		it++;
	}
	return it;
}

Image & FilterManager::queryImage(const FormatDesc &format)
{
	Container::iterator it=find(format);
	if(it==images.end())
	{
		images.push_back(Image(format));
		return images.back();
	}
	return *it;
}
/////////////////////////////////////////////////////////
// Matrix filter CV wrapping
/////////////////////////////////////////////////////////
MatrixFilter::MatrixFilter(int filterSize,const float * data)
{
	size=filterSize;
	matrix=new float[size*size];
	memcpy(matrix,data,sizeof(float)*size*size);
}

MatrixFilter::~MatrixFilter()
{
	if(matrix)
		delete matrix;
}

void MatrixFilter::applyTo(Image &target,Image &source)
{
	if(!areCompatible(&target,&source))
		return;
	int w=target.width();
	int h=target.height();	
	
//	int x1,y1;
//	float r;
	int size2=size/2;

	ImageView targetView=target.getFullView();
		
	//int fs=1+size+size;		
	//trg=tmp->getRect<int8>(0,0,w,h);
	//src=source.getRect<int8>(0,0,w,h);

	//for(int j=src.size();j;j--,src++,trg++)
	//	*trg=(int8)src*matrix[0];		
	/*int x=1;
	int y=0;	
	for(int i=0;i<size*size;i++)
	{
		ImageView::iterator trg=targetView.begin();
		ImageView::iterator end=targetView.end();
		ImageView sourceView=source.getView(x-size2,y-size2,w+x-size2,h+y-size2);
		ImageView::iterator src=sourceView.begin();

		for(;trg!=end;src++,trg++)		
			*trg=((int8)*trg)+matrix[i]*((int8)*src);
		x++;
		if(x==size)
		{
			x=0;
			y++;
		}
	}*/
	/*ImageView sourceView=target.getFullView();
	ImageView::iterator src=sourceView.begin();
	ImageView::iterator trg=targetView.begin();
	for(;src!=sourceView.end();src++,trg++)
		*trg=*src;*/
	CvMat kernel=cvMat(size,size,CV_32FC1,matrix);

	//cvSobel(source.image,target.image,1,0);
	//memcpy(kernel->data.fl,matrix,sizeof(float)*size*size);
	cvFilter2D(source, target/*tmp->image*/,&kernel,cvPoint(-size/2,-size/2));	
	//cvSet(target.image,cvScalar(128,0,0,0));
	//if(&target==&source)
	//	target.copyImage(tmp);
}

void MatrixFilter::init()
{}

void MatrixFilter::setElement(signed int x,signed int y,float e)
{
	if(	x<0 || x>=size || y<0 || y>=size)
		return;	
	matrix[x+y*(size+size)]=e;
}

float & MatrixFilter::operator () (int x,int y)
{
	return matrix[x+y*(size+size)];
}
float MatrixFilter::getElement(signed int x,signed int y)
{
	if(	x<0 || x>=size || y<0 || y>=size)
		return 0;	
	return matrix[x+y*(size+size)];
}
int MatrixFilter::save(std::ofstream &out)
{
	out<<size;
	out.write((char *)matrix,sizeof(float)*size*size);
	return 0;
}
int MatrixFilter::load(std::ifstream &in)
{
	in>>size;
	if(matrix)
		delete matrix;
	matrix=new float[size*size];
	in.read((char *)matrix,sizeof(float)*size*size);
	return 0;
}
/////////////////////////////////////////////////////////
// Smooth filter CV wrapping
/////////////////////////////////////////////////////////
Smooth::Smooth(int t,int sz0,int sz1,float thres0,float thres1)
{
	init(t,sz0,sz1,thres0,thres1);
}
void Smooth::init(int t,int sz0,int sz1,float thres0,float thres1)
{
	type=t;
	size[0]=sz0;
	size[1]=sz1;
	threshold[0]=thres0;
	threshold[1]=thres1;
}
void Smooth::applyTo(Image &source,Image &target)
{
	//std::cout<<"Smoothing\n";
	cvSmooth(source,target,type,size[0],size[1],threshold[0],threshold[1]);
}

Operator::Operator(const char *name)
{}
void Operator::init()
{
}
void Operator::setSource(ImageView &a, ImageView &b)
{
	lastSource[0]=&a;
	lastSource[1]=&b;
}
void Operator::setTarget(ImageView &target)
{
	lastTarget=&target;
}
///////////////////////////////////////////////////////////
// Correlation 
///////////////////////////////////////////////////////////

Correlation::Correlation(int s,int u,float t/*,Workspace *ws*/)
:Operator("Correlation")
{
	size=s;
	stride=u;
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
//correlationR.exe corr corr_test1.jpg corr_test1.jpg 4 1 test00.jpg
void Correlation::apply()
{	
	int sizel=(size-1)/2;
	int sizeh=size-sizel;
	int width=lastSource[0]->x_size();
	int height=lastSource[0]->y_size();
	ImageView/*<int8>*/ view=lastTarget->getView(0,0,width,height);
	//PixelIterator<int8> t=lastTarget->getRect<int8>(0,0,width,height);
	ImageView/*<int8>*/::iterator t=view.begin();
	//cout<<"width = "<<width<<endl;
	//cout<<"height= "<<height<<endl;	
	//cout<<"sizel = "<<sizel<<endl;
	//cout<<"sizeh = "<<sizeh<<endl;


	
//	cout<<"source[0]="<<ios::hex<<(long)lastSource[0]<<endl;
//	cout<<"source[1]="<<ios::hex<<(long)lastSource[1]<<endl;
	//cout<<view.st
	//signed int x,y;
	signed int xmin,ymin;
	//signed int ye;
	int length=0;
	float val=0;
	//int stride=size;
	

	int lw=lastSource[1]->x_size();
	int lh=lastSource[1]->y_size();
	float scale=1.0/(size*size);//1.0/(lw*lh);
//	cout<<"scale ="<<scale<<endl;

	//lastSource[0]->print();
	//lastSource[1]->print();

	//chrono.reset();
	//chrono.start();
	int flag=16;
	for(int y1=0;y1<height;y1++)
		for(int x1=0;x1<width;x1++,t++)
		{
			/*	if(!flag)
			{				
				flag=-1;
				//chrono.stop();
				//std::cout<<"Estimated time :"<<chrono.time_in_seconds()*width*height/16.0<<std::endl;				
			}*/
			//*t=0;
			int8 diffMax=255;

			
			/*for(int y3=y2;y3<y2+sh;y3++)
				for(int x3=x2;x3<x2+sw;x3++)
				{
					char val1=(*lastSource[0])(x3,y3);
					char val2=(*lastSource[1])(x3,y3);
					diff+=abs(val1-val2);
				}*/
			//diff*=scale;
			//cout<<(int)diff<<' ';
//			diff=(*lastSource[0])(x2,y2)-(*lastSource[1])(x2,y2);		
			int x2,y2;
			
			for(y2=std::max(y1-size,0);y2 < std::min(y1 + size,width);y2+=stride)
				for(x2=std::max(x1-size,0);x2 < std::min(x1 + size,width);x2+=stride)
				{
					ImageView scanWindow0=lastSource[0]->getView(x1-sizel,y1-sizel,size,size);
					ImageView scanWindow1=lastSource[1]->getView(x2-sizel,y2-sizel,size,size);				
					float diff=difference_sum<float>(scanWindow0,scanWindow1)*scale;
					//diff+=difference_sum<float>(s2,s3)*scale;
					
					if(diff<diffMax)
					{
						xmin=x2-x1;
						ymin=y2-y1;
						diffMax=diff;
					}
				}			
			*t=diffMax;//distanceTable[abs(xmin-x1)][abs(ymin-y1)];			
		}
}

void Correlation2::apply()
{
}

OpticFlow::OpticFlow(int size)
:Operator("OpticFlow"),winSize(size)
{
}

void OpticFlow::apply()
{
	CvMat *resX,*resY;
	int width=lastSource[0]->x_size();
	int height=lastSource[0]->y_size();
	int size=width*height;
	resX=cvCreateMat(height,width,CV_32FC1);
	resY=cvCreateMat(height,width,CV_32FC1);
	_CrtDbgBreak();
	cvCalcOpticalFlowLK(lastSource[0]->getImage()->getCore(),lastSource[0]->getImage()->getCore(),cvSize(winSize,winSize),resX,resY);
	//ImageView/*<int8>*/ view(*lastTarget,0,0,width-1,height-1);
	ImageView/*<int8>*/::iterator it;
	int i=0;
	float *x=(float*)resX->data.fl;//resX->imageData;
	float *y=(float*)resY->data.fl;;//->imageData;
	for(it=lastTarget->begin();it!=lastTarget->end();it++,i++)
		*it=x[i]*x[i]+y[i]*y[i];	
}

void Hypot::apply()
{
	if(	lastSource[0]->x_size()!=lastSource[1]->x_size() ||
		lastSource[0]->y_size()!=lastSource[1]->y_size())
		return;
	ImageView::iterator it0=lastSource[0]->begin();
	ImageView::iterator it1=lastSource[1]->begin();
	ImageView::iterator trg=lastTarget->begin();
	ImageView::iterator end=lastSource[0]->end();

	for(;it0!=end;it0++,it1++,trg++)
	{
		int8 a0=*it0;
		int8 a1=*it1;
		*trg=sqrtl(a0*a0+a1*a1);
	}
}



/////////////////////////////////////////////////////////////////
// IntegralImage, obsolete form
/////////////////////////////////////////////////////////////////
IntegralImage::IntegralImage()
:image(NULL)
{
}
IntegralImage::IntegralImage(ImageView/*<int8>*/ &view)
:image(NULL)
{
	setImage(view);
}
IntegralImage::~IntegralImage()
{
	if(image)
		cvReleaseImage(&image);
}
void IntegralImage::setImage(ImageView/*<int8>*/ &view)
{	
	if(image)
		cvReleaseImage(&image);
	int w=view.x_size();//img->width();
	int h=view.y_size();//img->height();
	image=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);

	for(int y=0;y<h;y++)
		for(int x=0;x<w;x++)
		{
			value_type &v=(*this)(x,y);
			v=*view(x,y);
			if(x && y)
				v+=(-(*this)(x-1,y-1)+(*this)(x,y-1)+(*this)(x-1,y));
			else if(y && !x)
				v+=(*this)(0,y-1);
			else if(x && !y)
				v+=(*this)(x-1,0);
		}
}
void IntegralImage::saveImage(const char *file)
{
	Image res(FormatDesc(x_size(),y_size()));
	ImageView view=res.getFullView();
	ImageView::iterator it=view.begin();
	value_type max=log((float)getRectSum(0,0,x_size()-1,y_size()-1));
	for(int y=0;y<y_size();y++)
		for(int x=0;x<x_size();x++,it++)
			*it=255.0f*log((float)getRectSum(0,0,x,y))/max;
	res.saveImage(file);
			
}
IntegralImage::value_type IntegralImage::getRectSum(int x,int y,int cx,int cy)
{
	int kx=std::min(image->width-1,x+cx);
	int ky=std::min(image->height-1,y+cy);
	x=std::max(int(0),x);
	y=std::max(int(0),y);
	return (*this)(kx,ky)+(*this)(x,y)+(*this)(x,y)-(*this)(kx,y)-(*this)(x,ky);
}
IntegralImage::value_type & IntegralImage::operator ()(int x,int y)
{
	assert(image);
	assert(x>=0 && x<image->width && y>=0 && y<image->height);
	return *reinterpret_cast<value_type*>(image->imageData+sizeof(float)*x+y*image->widthStep);
}
int IntegralImage::x_size()
{
	return image->width;
}
int IntegralImage::y_size()
{
	return image->height;
}

inline float getFloat(const int8 *ptr,int type)
{
	float res=0;
	if(type==formatLuminance)
		res=*ptr;
	if(type==formatFloat)
		res=*(float*)ptr;
	return res;
}
Image *buildIntegralImage(const ImageView &source)
{
	Image *image=new Image(FormatDesc(source.x_size(),source.y_size(),1,32));
	ImageView target=image->getFullView();
	int w=source.x_size();
	int h=source.y_size();
	ImageView::iterator it_src=source.begin();
	ImageView::iterator it_trg=target.begin();
	for(int y=0;y<h;y++)
		for(int x=0;x<w;x++,it_src++,it_trg++)
		{
			float &v=*(float*)target(x,y);
			//v=*source(x,y);
			//v=getFloat(source(x,y),source.parent->pixelFormat);
			if(x && y)
			{
				//v+=(-(*this)(x-1,y-1)+(*this)(x,y-1)+(*this)(x-1,y));
				v-=*(float*)target(x-1,y-1);
				v+=*(float*)target(x,y-1);
				v+=*(float*)target(x-1,y);
			}
			else if(y && !x)
			{
				v+=*(float*)target(0,y-1);
			}
			else if(x && !y)
			{
				v+=*(float*)target(x-1,0);
			}			
		}	
	return image;
}
/////////////////////////////////////////////////////////////////////////////
// Haar Feature utilities
/////////////////////////////////////////////////////////////////////////////
HaarRectDef::HaarRectDef()
	:left(0),right(0),top(0),bottom(0),weight(1)
{}
HaarRectDef::HaarRectDef(const HaarRectDef & def)
	:left(def.left),right(def.right),top(def.top),bottom(def.bottom),weight(def.weight)
{}
HaarRectDef::HaarRectDef(int x,int y,int cx,int cy,bool sign)
{
	setRect(x,y,cx,cy,sign);
}
void HaarRectDef::setRect(int x,int y,int cx,int cy,bool sign)
{
	left=MIN(x,cx);
	top=MIN(y,cy);
	right=MAX(x,cx);
	bottom=MAX(y,cy);
	weight=1.0f/((right-left)*(bottom-top));
	if(!sign)
		weight=-weight;
}
bool HaarRectDef::pointInRect(int x,int y)
{
	return x<right && x>=left && y<bottom && y>=top;
}

std::ostream & operator<<(std::ostream & out, const HaarRectDef &def)
{
	out<<"HaarRect "<<def.left<<" "<<def.top<<" "<<def.right<<" "<<def.bottom<<" "<<def.weight;
	return out;
}

std::istream & operator>>(std::istream & in, HaarRectDef &def)
{
	string str;
	in>>str>>def.left>>def.top>>def.right>>def.bottom>>def.weight;
	return in;
}

HaarFeature::HaarFeature(HaarRectDef *def,int count,int w,int h)
:sign(1),threshold(0),rectCount(0)
{

	assert(count<=rectCountMax);
	for(int i=0;i<count;i++)
		addRect(def[i]);

	width=w;
	height=h;

	int sum=w*h;
	for(int i=0;i<count;i++)	
	{
		int area=rect[i].area();
		assert(area>0);
		sum-=area;
	}
	assert(!sum);
	
}
void HaarFeature::setSize(int w, int h)
{
	width=w;
	height=h;
}

void HaarFeature::addRect(const HaarRectDef &def)
{
	if(rectCount<rectCountMax)
	{
		rect[rectCount]=def;
		rect[rectCount].weight=1.0/def.area();
		rectCount++;
	}
}	

float HaarFeature::value(IntegralImage &image,int x,int y)
{
	float sum=0;
	for(int i=0;i<rectCount;i++)
	{
		RECT rc;
		rc.left=x+rect[i].left;
		rc.right=x+rect[i].right;
		rc.top=y+rect[i].top;
		rc.bottom=y+rect[i].bottom;
		sum+=(image.getRectSum(	x+rect[i].left,
								y+rect[i].top,
								x+rect[i].right,
								y+rect[i].bottom)*rect[i].weight);
	}
	//sum/=(width*height);
	return sum;
}
int HaarFeature::test(float val)
{
	return (sign*val>sign*threshold)? 0 : 1;	
}
int HaarFeature::test(IntegralImage &image,int x,int y)
{
	float val=value(image,x,y);
	return (sign*val>sign*threshold)? 0 : 1;	
}

void HaarFeature::train(IntegralImage *x,float * values, char *y, int count)
{
	int positiveCount=0;
	int negativeCount=0;
	float positiveCenter=0;
	float negativeCenter=0;
	// 1. Get classifier values for each sample
	for(int i=0;i<count;i++)
	{
		values[i]=value(x[i],0,0);
		if(y[i])
		{
			positiveCount++;
			positiveCenter+=values[i];
		}
		else
		{
			negativeCount++;
			negativeCenter+=values[i];
		}
	}
	// 2. Calculate mass center for positive and negative samples		
	positiveCenter/=positiveCount;
	negativeCenter/=negativeCount;
	// 3. Calculate threshold and sign
	sign = (positiveCenter<negativeCenter) ? 1 : -1;
	threshold=0.5* (positiveCenter + negativeCenter);
}
bool HaarFeature::getCoord(int x,int y)
{
	//float fx=x/(float)width;
	//float fy=y/(float)height;
	for(int i=0;i<rectCountMax;i++)
	{
		if(rect[i].pointInRect(x,y))
			return rect[i].weight >= 0;
	}
	assert(false);
}
void HaarFeature::draw(ImageView_b &view)
{
	ImageView_b::iterator it=view.begin();
	for(int y=0;y<view.y_size();y++)
		for(int x=0;x<view.x_size();x++,it++)
		{
			if(getCoord(x,y))
				*it=200;
			else
				*it=64;
		}
}
std::ostream & operator<<(std::ostream & out, const HaarFeature &def)
{
	out<<"HaarFeature"<<std::endl;
	out<<" "<<def.threshold;	// порог
	out<<" "<<def.sign;		// с какой стороны порога сравниваем (больше или меньше)
	out<<" "<<def.width;		// размеры окна
	out<<" "<<def.height;
	out<<" "<<def.rectCount;	// число прямоугольников
	out<<std::endl;	
	for(int i=0;i<def.rectCount;i++)
		out<<" "<<def.rect[i]<<std::endl;		// сами прямоугольники
	return out;
}
std::istream & operator>>(std::istream & in, HaarFeature &def)
{
	char str[64];
//	in.getline(str,63);
	in>>str;
	if(string(str)=="HaarFeature")
	{
		in>>def.threshold;
		in>>def.sign;
		in>>def.width;
		in>>def.height;
		in>>def.rectCount;
		for(int i=0;i<def.rectCount;i++)
			in>>def.rect[i];
	}
	return in;
}

////////////////////////////////////////////////////////////////////
// RGB->binary filter
////////////////////////////////////////////////////////////////////
Binarisation::Binarisation(float *s, float smin, float smax)
:min(smin),max(smax),scale(s)
{
	scale.normalise();
}
void Binarisation::applyTo(Image &source,Image &target)
{
	IplImage *tmp=cvCreateImage(cvGetSize(source),8,1);
	if(source.channels()>1)
		cvSetImageCOI( source, 1 );

	cvCopy(source,tmp);	
	cvScale(tmp,target,(float)scale[0]);
	for(int c = 1; c < source.channels(); c++ )
	{
		// extract the c-th color plane
		if(source.channels()>1)
			cvSetImageCOI( source, c+1 );
		cvCopy(source,tmp);
		cvAddWeighted(tmp,(float)scale[c],target,1.0f,0.0f,target);			
	}
	cvResetImageROI(source);
	cvThreshold( target , target, max,255,CV_THRESH_TOZERO_INV);
	cvThreshold( target, target, min,1,CV_THRESH_BINARY);		 
	cvDilate(target,tmp,0,1);
	cvCopy(tmp,target);
	cvReleaseImage(&tmp);
}
Invert::Invert(int m)
:max(m)
{}
void Invert::applyTo(Image &source,Image &target)
{
	IplImage *tmp=cvCreateImage(cvGetSize(source),8,1);
	cvSet(tmp,cvScalar(max));
	cvSub(tmp,source,target);	
	cvReleaseImage(&tmp);
}

FilterGrow::FilterGrow(int val)
:value(val)
{}
void FilterGrow::applyTo(Image &source,Image &target)
{

}
////////////////////////////////////////////////////////////////
// BrightnessContrast
////////////////////////////////////////////////////////////////
BrightnessContrast::BrightnessContrast(int size)
:hist(NULL),hist_size(size)
{
	/*range_0[0]=0;
	range_0[1]=256;
	ranges[0]=&range_0;
	ranges[1]=&range_0+1;*/
	float range_0[]={0,256};
	float* ranges[] = { range_0 };
	hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    lut_mat = cvCreateMatHeader( 1, 256, CV_8UC1 );
    cvSetData(lut_mat, lut, 0 );
}
BrightnessContrast::~BrightnessContrast()
{
	cvReleaseHist(&hist);
}
void BrightnessContrast::set(float b,float c)
{
	brightness=b;
	contrast=c;
}
void BrightnessContrast::applyTo(Image &source,Image &target)
{
	//int brightness = _brightness - 100;
    //int contrast = _contrast - 100;
    int i, bin_w;
    

    /*
     * The algorithm is by Werner D. Streidt
     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
     */
    if( contrast > 0 )
    {
        double delta = 127.*contrast;
        double a = 255./(255. - delta -delta);
        double b = a*(brightness*255. - delta);
        for( i = 0; i < 256; i++ )
        {
            int v = cvRound(a*i + b);
            if( v < 0 )
                v = 0;
            if( v > 255 )
                v = 255;
            lut[i] = (uchar)v;
        }
    }
    else
    {
        double delta = -128.*contrast;
        double a = (256.-delta*2)/255.;
        double b = a*brightness*255. + delta;
        for( i = 0; i < 256; i++ )
        {
            int v = cvRound(a*i + b);
            if( v < 0 )
                v = 0;
            if( v > 255 )
                v = 255;
            lut[i] = (uchar)v;
        }
    }

    cvLUT( source, target, lut_mat );
    cvShowImage( "image", target );

	IplImage *img=target.getCore();
    cvCalcHist(&img, hist, 0, NULL );
}
void BrightnessContrast::draw(IplImage *image)
{
	float max_value = 0;
	float a=0,b=0,delta=0;
	if(contrast>0)
	{
		delta = 127.*contrast;
		a = 255./(255. - delta -delta);
        b = a*(brightness*255. - delta);
	}
	else
	{
		delta = -128.*contrast;
        a = (256.-delta*2)/255.;
        b = a*brightness*255. + delta;
	}
	int i, bin_w;
	cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
    cvScale( hist->bins, hist->bins, ((double)image->height)/max_value, 0 );
    /*cvNormalizeHist( hist, 1000 );*/

    cvSet( image, cvScalarAll(255), 0 );
    bin_w = cvRound((double)image->width/hist_size);

    for( i = 0; i < hist_size; i++ )
        cvRectangle( image, cvPoint(i*bin_w, image->height),
                     cvPoint((i+1)*bin_w, image->height - cvRound(cvGetReal1D(hist->bins,i))),
                     cvScalarAll(0), -1, 8, 0 );
	float scaleY=image->height/255.0;
	float scaleX=image->width/255.0;
	cvLine(image,cvPoint(0,b*scaleY),cvPoint(255*scaleX,(255*a+b)*scaleY),cvScalar(0,0,255));
   
    //cvShowImage( "histogram", image );
}
//#ifdef _CH_
//#pragma package <opencv>
//#endif
//
//#ifndef _EiC
//#include "cv.h"
//#include "highgui.h"
//#include <stdio.h>
//#endif
//
//char file_name[] = "baboon.jpg";
//
//int _brightness = 100;
//int _contrast = 100;
//
//int hist_size = 64;
//float range_0[]={0,256};
//float* ranges[] = { range_0 };
//IplImage *src_image = 0, *dst_image = 0, *hist_image = 0;
//CvHistogram *hist;
//uchar lut[256];
//CvMat* lut_mat;
//
///* brightness/contrast callback function */
//void update_brightcont( int arg )
//{
//    int brightness = _brightness - 100;
//    int contrast = _contrast - 100;
//    int i, bin_w;
//    float max_value = 0;
//
//    /*
//     * The algorithm is by Werner D. Streidt
//     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
//     */
//    if( contrast > 0 )
//    {
//        double delta = 127.*contrast/100;
//        double a = 255./(255. - delta*2);
//        double b = a*(brightness - delta);
//        for( i = 0; i < 256; i++ )
//        {
//            int v = cvRound(a*i + b);
//            if( v < 0 )
//                v = 0;
//            if( v > 255 )
//                v = 255;
//            lut[i] = (uchar)v;
//        }
//    }
//    else
//    {
//        double delta = -128.*contrast/100;
//        double a = (256.-delta*2)/255.;
//        double b = a*brightness + delta;
//        for( i = 0; i < 256; i++ )
//        {
//            int v = cvRound(a*i + b);
//            if( v < 0 )
//                v = 0;
//            if( v > 255 )
//                v = 255;
//            lut[i] = (uchar)v;
//        }
//    }
//
//    cvLUT( src_image, dst_image, lut_mat );
//    cvShowImage( "image", dst_image );
//
//    cvCalcHist( &dst_image, hist, 0, NULL );
//    cvZero( dst_image );
//    cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
//    cvScale( hist->bins, hist->bins, ((double)hist_image->height)/max_value, 0 );
//    /*cvNormalizeHist( hist, 1000 );*/
//
//    cvSet( hist_image, cvScalarAll(255), 0 );
//    bin_w = cvRound((double)hist_image->width/hist_size);
//
//    for( i = 0; i < hist_size; i++ )
//        cvRectangle( hist_image, cvPoint(i*bin_w, hist_image->height),
//                     cvPoint((i+1)*bin_w, hist_image->height - cvRound(cvGetReal1D(hist->bins,i))),
//                     cvScalarAll(0), -1, 8, 0 );
//   
//    cvShowImage( "histogram", hist_image );
//}
//
//
//int main( int argc, char** argv )
//{
//    // Load the source image. HighGUI use.
//    src_image = cvLoadImage( argc == 2 ? argv[1] : file_name, 0 );
//
//    if( !src_image )
//    {
//        printf("Image was not loaded.\n");
//        return -1;
//    }
//
//    dst_image = cvCloneImage(src_image);
//    hist_image = cvCreateImage(cvSize(320,200), 8, 1);
//    hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
//    lut_mat = cvCreateMatHeader( 1, 256, CV_8UC1 );
//    cvSetData( lut_mat, lut, 0 );
//
//    cvNamedWindow("image", 0);
//    cvNamedWindow("histogram", 0);
//
//    cvCreateTrackbar("brightness", "image", &_brightness, 200, update_brightcont);
//    cvCreateTrackbar("contrast", "image", &_contrast, 200, update_brightcont);
//
//    update_brightcont(0);
//    cvWaitKey(0);
//    
//    cvReleaseImage(&src_image);
//    cvReleaseImage(&dst_image);
//
//    cvReleaseHist(&hist);
//
//    return 0;
//}
//
//#ifdef _EiC
//main(1,"demhist.c");
//#endif
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
#include "stdafx.h"


#ifdef _DEBUG
#pragma comment(lib,"cv.lib")
#pragma comment(lib,"cxcore.lib")
#pragma comment(lib,"cvaux.lib")
#pragma comment(lib,"highgui.lib")
#else
#pragma comment(lib,"cv.lib")
#pragma comment(lib,"cxcore.lib")
#pragma comment(lib,"cvaux.lib")
#pragma comment(lib,"highgui.lib")
#endif
#ifdef _CH_
#pragma package <opencv>
#endif
#include <highgui.h>

//#include "..\frostTools\frostTools.h"


using namespace std;
int clamp(int min,int max,int value)
{
	if(value<min)
		return min;
	if(value>max)
		return max;
	return value;
}

bool areCompatible(Image *a,Image *b)
{
	return (a && 
			b && 
			a->isValid() && 
			b->isValid() &&
			a->width()==b->width()&&
			a->height()==b->height() && 
			a->depth()==b->depth() &&
			a->channels()==b->channels());	
}


/////////////////////////////////////////////////////////
// FilterManager
/////////////////////////////////////////////////////////
FilterManager Filter::filterManager[2];

FilterManager::FilterManager()
{
}

FilterManager::~FilterManager()
{
}

bool operator==(const FormatDesc &a,const FormatDesc &b)
{
	return	a.width==b.width && 
			a.height==b.height &&
			a.depth==b.depth &&
			a.channels==b.channels;
}

FilterManager::Container::iterator FilterManager::find(const FormatDesc &format)
{
	Container::iterator it=images.begin();
	while(it!=images.end())
	{
		if(getFormat(*it)==format)
			break;
		it++;
	}
	return it;
}

Image & FilterManager::queryImage(const FormatDesc &format)
{
	Container::iterator it=find(format);
	if(it==images.end())
	{
		images.push_back(Image(format));
		return images.back();
	}
	return *it;
}
/////////////////////////////////////////////////////////
// Matrix filter CV wrapping
/////////////////////////////////////////////////////////
MatrixFilter::MatrixFilter(int filterSize,const float * data)
{
	size=filterSize;
	matrix=new float[size*size];
	memcpy(matrix,data,sizeof(float)*size*size);
}

MatrixFilter::~MatrixFilter()
{
	if(matrix)
		delete matrix;
}

void MatrixFilter::applyTo(Image &target,Image &source)
{
	if(!areCompatible(&target,&source))
		return;
	int w=target.width();
	int h=target.height();	
	
//	int x1,y1;
//	float r;
	int size2=size/2;

	ImageView targetView=target.getFullView();
		
	//int fs=1+size+size;		
	//trg=tmp->getRect<int8>(0,0,w,h);
	//src=source.getRect<int8>(0,0,w,h);

	//for(int j=src.size();j;j--,src++,trg++)
	//	*trg=(int8)src*matrix[0];		
	/*int x=1;
	int y=0;	
	for(int i=0;i<size*size;i++)
	{
		ImageView::iterator trg=targetView.begin();
		ImageView::iterator end=targetView.end();
		ImageView sourceView=source.getView(x-size2,y-size2,w+x-size2,h+y-size2);
		ImageView::iterator src=sourceView.begin();

		for(;trg!=end;src++,trg++)		
			*trg=((int8)*trg)+matrix[i]*((int8)*src);
		x++;
		if(x==size)
		{
			x=0;
			y++;
		}
	}*/
	/*ImageView sourceView=target.getFullView();
	ImageView::iterator src=sourceView.begin();
	ImageView::iterator trg=targetView.begin();
	for(;src!=sourceView.end();src++,trg++)
		*trg=*src;*/
	CvMat kernel=cvMat(size,size,CV_32FC1,matrix);

	//cvSobel(source.image,target.image,1,0);
	//memcpy(kernel->data.fl,matrix,sizeof(float)*size*size);
	cvFilter2D(source, target/*tmp->image*/,&kernel,cvPoint(-size/2,-size/2));	
	//cvSet(target.image,cvScalar(128,0,0,0));
	//if(&target==&source)
	//	target.copyImage(tmp);
}

void MatrixFilter::init()
{}

void MatrixFilter::setElement(signed int x,signed int y,float e)
{
	if(	x<0 || x>=size || y<0 || y>=size)
		return;	
	matrix[x+y*(size+size)]=e;
}

float & MatrixFilter::operator () (int x,int y)
{
	return matrix[x+y*(size+size)];
}
float MatrixFilter::getElement(signed int x,signed int y)
{
	if(	x<0 || x>=size || y<0 || y>=size)
		return 0;	
	return matrix[x+y*(size+size)];
}
int MatrixFilter::save(std::ofstream &out)
{
	out<<size;
	out.write((char *)matrix,sizeof(float)*size*size);
	return 0;
}
int MatrixFilter::load(std::ifstream &in)
{
	in>>size;
	if(matrix)
		delete matrix;
	matrix=new float[size*size];
	in.read((char *)matrix,sizeof(float)*size*size);
	return 0;
}
/////////////////////////////////////////////////////////
// Smooth filter CV wrapping
/////////////////////////////////////////////////////////
Smooth::Smooth(int t,int sz0,int sz1,float thres0,float thres1)
{
	init(t,sz0,sz1,thres0,thres1);
}
void Smooth::init(int t,int sz0,int sz1,float thres0,float thres1)
{
	type=t;
	size[0]=sz0;
	size[1]=sz1;
	threshold[0]=thres0;
	threshold[1]=thres1;
}
void Smooth::applyTo(Image &source,Image &target)
{
	//std::cout<<"Smoothing\n";
	cvSmooth(source,target,type,size[0],size[1],threshold[0],threshold[1]);
}

Operator::Operator(const char *name)
{}
void Operator::init()
{
}
void Operator::setSource(ImageView &a, ImageView &b)
{
	lastSource[0]=&a;
	lastSource[1]=&b;
}
void Operator::setTarget(ImageView &target)
{
	lastTarget=&target;
}
///////////////////////////////////////////////////////////
// Correlation 
///////////////////////////////////////////////////////////

Correlation::Correlation(int s,int u,float t/*,Workspace *ws*/)
:Operator("Correlation")
{
	size=s;
	stride=u;
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
//correlationR.exe corr corr_test1.jpg corr_test1.jpg 4 1 test00.jpg
void Correlation::apply()
{	
	int sizel=(size-1)/2;
	int sizeh=size-sizel;
	int width=lastSource[0]->x_size();
	int height=lastSource[0]->y_size();
	ImageView/*<int8>*/ view=lastTarget->getView(0,0,width,height);
	//PixelIterator<int8> t=lastTarget->getRect<int8>(0,0,width,height);
	ImageView/*<int8>*/::iterator t=view.begin();
	//cout<<"width = "<<width<<endl;
	//cout<<"height= "<<height<<endl;	
	//cout<<"sizel = "<<sizel<<endl;
	//cout<<"sizeh = "<<sizeh<<endl;


	
//	cout<<"source[0]="<<ios::hex<<(long)lastSource[0]<<endl;
//	cout<<"source[1]="<<ios::hex<<(long)lastSource[1]<<endl;
	//cout<<view.st
	//signed int x,y;
	signed int xmin,ymin;
	//signed int ye;
	int length=0;
	float val=0;
	//int stride=size;
	

	int lw=lastSource[1]->x_size();
	int lh=lastSource[1]->y_size();
	float scale=1.0/(size*size);//1.0/(lw*lh);
//	cout<<"scale ="<<scale<<endl;

	//lastSource[0]->print();
	//lastSource[1]->print();

	//chrono.reset();
	//chrono.start();
	int flag=16;
	for(int y1=0;y1<height;y1++)
		for(int x1=0;x1<width;x1++,t++)
		{
			/*	if(!flag)
			{				
				flag=-1;
				//chrono.stop();
				//std::cout<<"Estimated time :"<<chrono.time_in_seconds()*width*height/16.0<<std::endl;				
			}*/
			//*t=0;
			int8 diffMax=255;

			
			/*for(int y3=y2;y3<y2+sh;y3++)
				for(int x3=x2;x3<x2+sw;x3++)
				{
					char val1=(*lastSource[0])(x3,y3);
					char val2=(*lastSource[1])(x3,y3);
					diff+=abs(val1-val2);
				}*/
			//diff*=scale;
			//cout<<(int)diff<<' ';
//			diff=(*lastSource[0])(x2,y2)-(*lastSource[1])(x2,y2);		
			int x2,y2;
			
			for(y2=std::max(y1-size,0);y2 < std::min(y1 + size,width);y2+=stride)
				for(x2=std::max(x1-size,0);x2 < std::min(x1 + size,width);x2+=stride)
				{
					ImageView scanWindow0=lastSource[0]->getView(x1-sizel,y1-sizel,size,size);
					ImageView scanWindow1=lastSource[1]->getView(x2-sizel,y2-sizel,size,size);				
					float diff=difference_sum<float>(scanWindow0,scanWindow1)*scale;
					//diff+=difference_sum<float>(s2,s3)*scale;
					
					if(diff<diffMax)
					{
						xmin=x2-x1;
						ymin=y2-y1;
						diffMax=diff;
					}
				}			
			*t=diffMax;//distanceTable[abs(xmin-x1)][abs(ymin-y1)];			
		}
}

void Correlation2::apply()
{
}

OpticFlow::OpticFlow(int size)
:Operator("OpticFlow"),winSize(size)
{
}

void OpticFlow::apply()
{
	CvMat *resX,*resY;
	int width=lastSource[0]->x_size();
	int height=lastSource[0]->y_size();
	int size=width*height;
	resX=cvCreateMat(height,width,CV_32FC1);
	resY=cvCreateMat(height,width,CV_32FC1);
	_CrtDbgBreak();
	cvCalcOpticalFlowLK(lastSource[0]->getImage()->getCore(),lastSource[0]->getImage()->getCore(),cvSize(winSize,winSize),resX,resY);
	//ImageView/*<int8>*/ view(*lastTarget,0,0,width-1,height-1);
	ImageView/*<int8>*/::iterator it;
	int i=0;
	float *x=(float*)resX->data.fl;//resX->imageData;
	float *y=(float*)resY->data.fl;;//->imageData;
	for(it=lastTarget->begin();it!=lastTarget->end();it++,i++)
		*it=x[i]*x[i]+y[i]*y[i];	
}

void Hypot::apply()
{
	if(	lastSource[0]->x_size()!=lastSource[1]->x_size() ||
		lastSource[0]->y_size()!=lastSource[1]->y_size())
		return;
	ImageView::iterator it0=lastSource[0]->begin();
	ImageView::iterator it1=lastSource[1]->begin();
	ImageView::iterator trg=lastTarget->begin();
	ImageView::iterator end=lastSource[0]->end();

	for(;it0!=end;it0++,it1++,trg++)
	{
		int8 a0=*it0;
		int8 a1=*it1;
		*trg=sqrtl(a0*a0+a1*a1);
	}
}



/////////////////////////////////////////////////////////////////
// IntegralImage, obsolete form
/////////////////////////////////////////////////////////////////
IntegralImage::IntegralImage()
:image(NULL)
{
}
IntegralImage::IntegralImage(ImageView/*<int8>*/ &view)
:image(NULL)
{
	setImage(view);
}
IntegralImage::~IntegralImage()
{
	if(image)
		cvReleaseImage(&image);
}
void IntegralImage::setImage(ImageView/*<int8>*/ &view)
{	
	if(image)
		cvReleaseImage(&image);
	int w=view.x_size();//img->width();
	int h=view.y_size();//img->height();
	image=cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);

	for(int y=0;y<h;y++)
		for(int x=0;x<w;x++)
		{
			value_type &v=(*this)(x,y);
			v=*view(x,y);
			if(x && y)
				v+=(-(*this)(x-1,y-1)+(*this)(x,y-1)+(*this)(x-1,y));
			else if(y && !x)
				v+=(*this)(0,y-1);
			else if(x && !y)
				v+=(*this)(x-1,0);
		}
}
void IntegralImage::saveImage(const char *file)
{
	Image res(FormatDesc(x_size(),y_size()));
	ImageView view=res.getFullView();
	ImageView::iterator it=view.begin();
	value_type max=log((float)getRectSum(0,0,x_size()-1,y_size()-1));
	for(int y=0;y<y_size();y++)
		for(int x=0;x<x_size();x++,it++)
			*it=255.0f*log((float)getRectSum(0,0,x,y))/max;
	res.saveImage(file);
			
}
IntegralImage::value_type IntegralImage::getRectSum(int x,int y,int cx,int cy)
{
	int kx=std::min(image->width-1,x+cx);
	int ky=std::min(image->height-1,y+cy);
	x=std::max(int(0),x);
	y=std::max(int(0),y);
	return (*this)(kx,ky)+(*this)(x,y)+(*this)(x,y)-(*this)(kx,y)-(*this)(x,ky);
}
IntegralImage::value_type & IntegralImage::operator ()(int x,int y)
{
	assert(image);
	assert(x>=0 && x<image->width && y>=0 && y<image->height);
	return *reinterpret_cast<value_type*>(image->imageData+sizeof(float)*x+y*image->widthStep);
}
int IntegralImage::x_size()
{
	return image->width;
}
int IntegralImage::y_size()
{
	return image->height;
}

inline float getFloat(const int8 *ptr,int type)
{
	float res=0;
	if(type==formatLuminance)
		res=*ptr;
	if(type==formatFloat)
		res=*(float*)ptr;
	return res;
}
Image *buildIntegralImage(const ImageView &source)
{
	Image *image=new Image(FormatDesc(source.x_size(),source.y_size(),1,32));
	ImageView target=image->getFullView();
	int w=source.x_size();
	int h=source.y_size();
	ImageView::iterator it_src=source.begin();
	ImageView::iterator it_trg=target.begin();
	for(int y=0;y<h;y++)
		for(int x=0;x<w;x++,it_src++,it_trg++)
		{
			float &v=*(float*)target(x,y);
			//v=*source(x,y);
			//v=getFloat(source(x,y),source.parent->pixelFormat);
			if(x && y)
			{
				//v+=(-(*this)(x-1,y-1)+(*this)(x,y-1)+(*this)(x-1,y));
				v-=*(float*)target(x-1,y-1);
				v+=*(float*)target(x,y-1);
				v+=*(float*)target(x-1,y);
			}
			else if(y && !x)
			{
				v+=*(float*)target(0,y-1);
			}
			else if(x && !y)
			{
				v+=*(float*)target(x-1,0);
			}			
		}	
	return image;
}
/////////////////////////////////////////////////////////////////////////////
// Haar Feature utilities
/////////////////////////////////////////////////////////////////////////////
HaarRectDef::HaarRectDef()
	:left(0),right(0),top(0),bottom(0),weight(1)
{}
HaarRectDef::HaarRectDef(const HaarRectDef & def)
	:left(def.left),right(def.right),top(def.top),bottom(def.bottom),weight(def.weight)
{}
HaarRectDef::HaarRectDef(int x,int y,int cx,int cy,bool sign)
{
	setRect(x,y,cx,cy,sign);
}
void HaarRectDef::setRect(int x,int y,int cx,int cy,bool sign)
{
	left=MIN(x,cx);
	top=MIN(y,cy);
	right=MAX(x,cx);
	bottom=MAX(y,cy);
	weight=1.0f/((right-left)*(bottom-top));
	if(!sign)
		weight=-weight;
}
bool HaarRectDef::pointInRect(int x,int y)
{
	return x<right && x>=left && y<bottom && y>=top;
}

std::ostream & operator<<(std::ostream & out, const HaarRectDef &def)
{
	out<<"HaarRect "<<def.left<<" "<<def.top<<" "<<def.right<<" "<<def.bottom<<" "<<def.weight;
	return out;
}

std::istream & operator>>(std::istream & in, HaarRectDef &def)
{
	string str;
	in>>str>>def.left>>def.top>>def.right>>def.bottom>>def.weight;
	return in;
}

HaarFeature::HaarFeature(HaarRectDef *def,int count,int w,int h)
:sign(1),threshold(0),rectCount(0)
{

	assert(count<=rectCountMax);
	for(int i=0;i<count;i++)
		addRect(def[i]);

	width=w;
	height=h;

	int sum=w*h;
	for(int i=0;i<count;i++)	
	{
		int area=rect[i].area();
		assert(area>0);
		sum-=area;
	}
	assert(!sum);
	
}
void HaarFeature::setSize(int w, int h)
{
	width=w;
	height=h;
}

void HaarFeature::addRect(const HaarRectDef &def)
{
	if(rectCount<rectCountMax)
	{
		rect[rectCount]=def;
		rect[rectCount].weight=1.0/def.area();
		rectCount++;
	}
}	

float HaarFeature::value(IntegralImage &image,int x,int y)
{
	float sum=0;
	for(int i=0;i<rectCount;i++)
	{
		RECT rc;
		rc.left=x+rect[i].left;
		rc.right=x+rect[i].right;
		rc.top=y+rect[i].top;
		rc.bottom=y+rect[i].bottom;
		sum+=(image.getRectSum(	x+rect[i].left,
								y+rect[i].top,
								x+rect[i].right,
								y+rect[i].bottom)*rect[i].weight);
	}
	//sum/=(width*height);
	return sum;
}
int HaarFeature::test(float val)
{
	return (sign*val>sign*threshold)? 0 : 1;	
}
int HaarFeature::test(IntegralImage &image,int x,int y)
{
	float val=value(image,x,y);
	return (sign*val>sign*threshold)? 0 : 1;	
}

void HaarFeature::train(IntegralImage *x,float * values, char *y, int count)
{
	int positiveCount=0;
	int negativeCount=0;
	float positiveCenter=0;
	float negativeCenter=0;
	// 1. Get classifier values for each sample
	for(int i=0;i<count;i++)
	{
		values[i]=value(x[i],0,0);
		if(y[i])
		{
			positiveCount++;
			positiveCenter+=values[i];
		}
		else
		{
			negativeCount++;
			negativeCenter+=values[i];
		}
	}
	// 2. Calculate mass center for positive and negative samples		
	positiveCenter/=positiveCount;
	negativeCenter/=negativeCount;
	// 3. Calculate threshold and sign
	sign = (positiveCenter<negativeCenter) ? 1 : -1;
	threshold=0.5* (positiveCenter + negativeCenter);
}
bool HaarFeature::getCoord(int x,int y)
{
	//float fx=x/(float)width;
	//float fy=y/(float)height;
	for(int i=0;i<rectCountMax;i++)
	{
		if(rect[i].pointInRect(x,y))
			return rect[i].weight >= 0;
	}
	assert(false);
}
void HaarFeature::draw(ImageView_b &view)
{
	ImageView_b::iterator it=view.begin();
	for(int y=0;y<view.y_size();y++)
		for(int x=0;x<view.x_size();x++,it++)
		{
			if(getCoord(x,y))
				*it=200;
			else
				*it=64;
		}
}
std::ostream & operator<<(std::ostream & out, const HaarFeature &def)
{
	out<<"HaarFeature"<<std::endl;
	out<<" "<<def.threshold;	// порог
	out<<" "<<def.sign;		// с какой стороны порога сравниваем (больше или меньше)
	out<<" "<<def.width;		// размеры окна
	out<<" "<<def.height;
	out<<" "<<def.rectCount;	// число прямоугольников
	out<<std::endl;	
	for(int i=0;i<def.rectCount;i++)
		out<<" "<<def.rect[i]<<std::endl;		// сами прямоугольники
	return out;
}
std::istream & operator>>(std::istream & in, HaarFeature &def)
{
	char str[64];
//	in.getline(str,63);
	in>>str;
	if(string(str)=="HaarFeature")
	{
		in>>def.threshold;
		in>>def.sign;
		in>>def.width;
		in>>def.height;
		in>>def.rectCount;
		for(int i=0;i<def.rectCount;i++)
			in>>def.rect[i];
	}
	return in;
}

////////////////////////////////////////////////////////////////////
// RGB->binary filter
////////////////////////////////////////////////////////////////////
Binarisation::Binarisation(float *s, float smin, float smax)
:min(smin),max(smax),scale(s)
{
	scale.normalise();
}
void Binarisation::applyTo(Image &source,Image &target)
{
	IplImage *tmp=cvCreateImage(cvGetSize(source),8,1);
	if(source.channels()>1)
		cvSetImageCOI( source, 1 );

	cvCopy(source,tmp);	
	cvScale(tmp,target,(float)scale[0]);
	for(int c = 1; c < source.channels(); c++ )
	{
		// extract the c-th color plane
		if(source.channels()>1)
			cvSetImageCOI( source, c+1 );
		cvCopy(source,tmp);
		cvAddWeighted(tmp,(float)scale[c],target,1.0f,0.0f,target);			
	}
	cvResetImageROI(source);
	cvThreshold( target , target, max,255,CV_THRESH_TOZERO_INV);
	cvThreshold( target, target, min,1,CV_THRESH_BINARY);		 
	cvDilate(target,tmp,0,1);
	cvCopy(tmp,target);
	cvReleaseImage(&tmp);
}
Invert::Invert(int m)
:max(m)
{}
void Invert::applyTo(Image &source,Image &target)
{
	IplImage *tmp=cvCreateImage(cvGetSize(source),8,1);
	cvSet(tmp,cvScalar(max));
	cvSub(tmp,source,target);	
	cvReleaseImage(&tmp);
}

FilterGrow::FilterGrow(int val)
:value(val)
{}
void FilterGrow::applyTo(Image &source,Image &target)
{

}
////////////////////////////////////////////////////////////////
// BrightnessContrast
////////////////////////////////////////////////////////////////
BrightnessContrast::BrightnessContrast(int size)
:hist(NULL),hist_size(size)
{
	/*range_0[0]=0;
	range_0[1]=256;
	ranges[0]=&range_0;
	ranges[1]=&range_0+1;*/
	float range_0[]={0,256};
	float* ranges[] = { range_0 };
	hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    lut_mat = cvCreateMatHeader( 1, 256, CV_8UC1 );
    cvSetData(lut_mat, lut, 0 );
}
BrightnessContrast::~BrightnessContrast()
{
	cvReleaseHist(&hist);
}
void BrightnessContrast::set(float b,float c)
{
	brightness=b;
	contrast=c;
}
void BrightnessContrast::applyTo(Image &source,Image &target)
{
	//int brightness = _brightness - 100;
    //int contrast = _contrast - 100;
    int i, bin_w;
    

    /*
     * The algorithm is by Werner D. Streidt
     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
     */
    if( contrast > 0 )
    {
        double delta = 127.*contrast;
        double a = 255./(255. - delta -delta);
        double b = a*(brightness*255. - delta);
        for( i = 0; i < 256; i++ )
        {
            int v = cvRound(a*i + b);
            if( v < 0 )
                v = 0;
            if( v > 255 )
                v = 255;
            lut[i] = (uchar)v;
        }
    }
    else
    {
        double delta = -128.*contrast;
        double a = (256.-delta*2)/255.;
        double b = a*brightness*255. + delta;
        for( i = 0; i < 256; i++ )
        {
            int v = cvRound(a*i + b);
            if( v < 0 )
                v = 0;
            if( v > 255 )
                v = 255;
            lut[i] = (uchar)v;
        }
    }

    cvLUT( source, target, lut_mat );
    cvShowImage( "image", target );

	IplImage *img=target.getCore();
    cvCalcHist(&img, hist, 0, NULL );
}
void BrightnessContrast::draw(IplImage *image)
{
	float max_value = 0;
	float a=0,b=0,delta=0;
	if(contrast>0)
	{
		delta = 127.*contrast;
		a = 255./(255. - delta -delta);
        b = a*(brightness*255. - delta);
	}
	else
	{
		delta = -128.*contrast;
        a = (256.-delta*2)/255.;
        b = a*brightness*255. + delta;
	}
	int i, bin_w;
	cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
    cvScale( hist->bins, hist->bins, ((double)image->height)/max_value, 0 );
    /*cvNormalizeHist( hist, 1000 );*/

    cvSet( image, cvScalarAll(255), 0 );
    bin_w = cvRound((double)image->width/hist_size);

    for( i = 0; i < hist_size; i++ )
        cvRectangle( image, cvPoint(i*bin_w, image->height),
                     cvPoint((i+1)*bin_w, image->height - cvRound(cvGetReal1D(hist->bins,i))),
                     cvScalarAll(0), -1, 8, 0 );
	float scaleY=image->height/255.0;
	float scaleX=image->width/255.0;
	cvLine(image,cvPoint(0,b*scaleY),cvPoint(255*scaleX,(255*a+b)*scaleY),cvScalar(0,0,255));
   
    //cvShowImage( "histogram", image );
}
//#ifdef _CH_
//#pragma package <opencv>
//#endif
//
//#ifndef _EiC
//#include "cv.h"
//#include "highgui.h"
//#include <stdio.h>
//#endif
//
//char file_name[] = "baboon.jpg";
//
//int _brightness = 100;
//int _contrast = 100;
//
//int hist_size = 64;
//float range_0[]={0,256};
//float* ranges[] = { range_0 };
//IplImage *src_image = 0, *dst_image = 0, *hist_image = 0;
//CvHistogram *hist;
//uchar lut[256];
//CvMat* lut_mat;
//
///* brightness/contrast callback function */
//void update_brightcont( int arg )
//{
//    int brightness = _brightness - 100;
//    int contrast = _contrast - 100;
//    int i, bin_w;
//    float max_value = 0;
//
//    /*
//     * The algorithm is by Werner D. Streidt
//     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
//     */
//    if( contrast > 0 )
//    {
//        double delta = 127.*contrast/100;
//        double a = 255./(255. - delta*2);
//        double b = a*(brightness - delta);
//        for( i = 0; i < 256; i++ )
//        {
//            int v = cvRound(a*i + b);
//            if( v < 0 )
//                v = 0;
//            if( v > 255 )
//                v = 255;
//            lut[i] = (uchar)v;
//        }
//    }
//    else
//    {
//        double delta = -128.*contrast/100;
//        double a = (256.-delta*2)/255.;
//        double b = a*brightness + delta;
//        for( i = 0; i < 256; i++ )
//        {
//            int v = cvRound(a*i + b);
//            if( v < 0 )
//                v = 0;
//            if( v > 255 )
//                v = 255;
//            lut[i] = (uchar)v;
//        }
//    }
//
//    cvLUT( src_image, dst_image, lut_mat );
//    cvShowImage( "image", dst_image );
//
//    cvCalcHist( &dst_image, hist, 0, NULL );
//    cvZero( dst_image );
//    cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
//    cvScale( hist->bins, hist->bins, ((double)hist_image->height)/max_value, 0 );
//    /*cvNormalizeHist( hist, 1000 );*/
//
//    cvSet( hist_image, cvScalarAll(255), 0 );
//    bin_w = cvRound((double)hist_image->width/hist_size);
//
//    for( i = 0; i < hist_size; i++ )
//        cvRectangle( hist_image, cvPoint(i*bin_w, hist_image->height),
//                     cvPoint((i+1)*bin_w, hist_image->height - cvRound(cvGetReal1D(hist->bins,i))),
//                     cvScalarAll(0), -1, 8, 0 );
//   
//    cvShowImage( "histogram", hist_image );
//}
//
//
//int main( int argc, char** argv )
//{
//    // Load the source image. HighGUI use.
//    src_image = cvLoadImage( argc == 2 ? argv[1] : file_name, 0 );
//
//    if( !src_image )
//    {
//        printf("Image was not loaded.\n");
//        return -1;
//    }
//
//    dst_image = cvCloneImage(src_image);
//    hist_image = cvCreateImage(cvSize(320,200), 8, 1);
//    hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
//    lut_mat = cvCreateMatHeader( 1, 256, CV_8UC1 );
//    cvSetData( lut_mat, lut, 0 );
//
//    cvNamedWindow("image", 0);
//    cvNamedWindow("histogram", 0);
//
//    cvCreateTrackbar("brightness", "image", &_brightness, 200, update_brightcont);
//    cvCreateTrackbar("contrast", "image", &_contrast, 200, update_brightcont);
//
//    update_brightcont(0);
//    cvWaitKey(0);
//    
//    cvReleaseImage(&src_image);
//    cvReleaseImage(&dst_image);
//
//    cvReleaseHist(&hist);
//
//    return 0;
//}
//
//#ifdef _EiC
//main(1,"demhist.c");
//#endif
>>>>>>> bd17e7de82f5e0c1302ffcdcf53da1a9448d3f2d
//