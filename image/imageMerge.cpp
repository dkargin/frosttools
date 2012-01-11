#include "stdafx.h"
//#include "subPathBlock.h"
enum
{
	imageTableFixCols,
	imageTableFixRows,
	imageTableFixAspect
};
/*
“аблица из рисунков одинакового размера
*/
class ImageTable: public Image
{	
public:
	typedef std::vector<ImageView/*<int8>*/> Container;
	
	int cols;
	int rows;
	int mode;
	int borderSize;
	int borderExternSize;
	int localWidth;
	int localHeight;

	bool bActiveCreate;
	bool needResize;

	int surfaceUpdated;
	Container views;	
public:	
	ImageTable();

	void preCreateImage(int lWidth,int lHeight);
	void addImage(Image *img);
	void addView(ImageView/*<int8>*/ &view);
	void updateTable();		// redraw surface
	void setMode(int value);
	void setSize(int col,int row);	
	void setBorderSize(int size);
};

ImageTable::ImageTable()
{
	surfaceUpdated=false;
	localWidth=0;
	localHeight=0;
	borderSize=0;
	borderExternSize=0;
	rows=1;
	cols=1;
	mode=imageTableFixCols;	
}
void ImageTable::setBorderSize(int size)
{
	borderSize=size;
}

void ImageTable::setMode(int value)
{
	mode=value;
}

void ImageTable::addImage(Image *img)
{
	if(!views.size()) // if we add first image
	{
		localWidth=img->width();
		localHeight=img->height();
	}
	views.push_back(img->getFullView());
	surfaceUpdated=false;
}
void ImageTable::addView(ImageView/*<int8>*/ &view)
{	
	if(!views.size())
	{
		localWidth=view.x_size();
		localHeight=view.y_size();
	}
	views.push_back(view);
	surfaceUpdated=false;
}

void ImageTable::updateTable()
{
	if(surfaceUpdated)
		return;
	// 1. Calculate real column number
	int rcol;
	int rrow;
	int totalWidth;
	int totalHeight;
	int count=views.size();

	int x0=0;
	int y0=0;
	int dx=localWidth;
	int dy=localHeight;
	

	switch(mode)
	{
	case imageTableFixCols:
		rcol=cols;
		rrow=((float)count/cols+0.5f);
		break;
	case imageTableFixRows:
		rrow=rows;
		rcol=((float)count/rows+0.5f);
		break;
	case imageTableFixAspect:
		break;
	}
	// 2. Render to surface
	totalWidth=localWidth*rcol;
	totalHeight=localHeight*rrow;
	
	totalWidth+=borderSize*(rcol-1);
	totalHeight+=borderSize*(rrow-1);
	dx+=borderSize;
	dy+=borderSize;

	totalWidth+= (borderExternSize+borderExternSize);
	totalHeight+= (borderExternSize+borderExternSize);
	x0+=borderExternSize;
	y0+=borderExternSize;
	
	int x=x0;
	int y=x0;
	imageCreate(totalWidth,totalHeight,formatLuminance);

	Container::iterator it=views.begin();
	Container::iterator end=views.end();

	int i=0,j=0;
	for(;it!=end;it++,j++)
	{
		if(j>=cols)
		{
			i++;
			j=0;
			x=x0;			
			y+=dy;			
		}
		x+=dx;
		ImageView view=getView(x,y,localWidth,localHeight);
		copy(view,*it);

	}
	surfaceUpdated=true;
}
/*
argv[0] - file list
argv[1] - mode {cols,rows}
argv[2] - count
argv[3] - internal border
argv[4] - output image
*/
void imageMerge(int argc,char *argv[])
{
	int nCols=1;
//	int nRows;
	FileList list(argv[0]);
	std::string mode(argv[1]);
	ImageTable table;
	if(mode=="cols")
	{
		stringstream(argv[2])>>table.cols;
		table.setMode(imageTableFixCols);
	}
	if(mode=="rows")
	{
		stringstream(argv[2])>>table.rows;
		table.setMode(imageTableFixRows);
	}
	for(FileList::iterator it=list.begin();it!=list.end();it++)
	{
		
	}
}

/*
void drawPathBlocks(char *file,char *dirOut)
{
	//std::ifstream in(file);
	FILE *in=fopen(file,"rb");
	int size;
	fread((char*)&size,sizeof(int),1,in);
	//in.read((char*)&size,sizeof(int));
	//std::vector<pathProject::SubPathBlock16> blocks(size);

	char sys[256];
	sprintf(sys,"mkdir %s",dirOut);
	system(sys);

	//pathProject::SubPathBlock16 *blocks=new pathProject::SubPathBlock16[size];
	for(int i=0;i<size;i++)
	{
		pathProject::SubPathBlock16 block;
		block.load(in);
		block.getMinMax(block.bounds);
		Image image(16,16,formatLuminance);
		ImageView view=image.getFullView();
		ImageView::iterator it=view.begin();
		for(int y=0;y<16;y++)
			for(int x=0;x<16;x++,it++)
			{
				if(block.getCheck(x,y))
					*it=128;
				else
					*it=0;
			}		
		sprintf(sys,"%s\\block%04d.jpg",dirOut,i);
		image.saveImage(sys);
		
	}
	//delete []blocks;
	fclose(in);
}*/
