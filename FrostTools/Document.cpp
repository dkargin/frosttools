<<<<<<< HEAD
#include "stdafx.h"

#include "Document.h"
//#include "resource.h"
//#include "BaseView.h"
#include "ImageTools.h"
//#include "MainFrm.h"

#include "fstream"


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

Workspace *gWorkspace=NULL;
///////////////////////////////////////////////////////////////////////
/// Base Object
///////////////////////////////////////////////////////////////////////

BaseDocument::BaseDocument(const char *str,Workspace *ws,int nType)
:name(str)
,owner(ws)
,type(nType)
//,ListenerSingle(NULL)
//,view(NULL)
{}

BaseDocument::~BaseDocument()
{
	/*if(view)
		delete view;*/
}

void BaseDocument::show()
{
	owner->showView(this);
}

void BaseDocument::hide()
{
}

void BaseDocument::setName(const std::string &str)
{
	name=str;
}
std::string BaseDocument::getName()
{
	return name;
}

//void BaseDocument::setView(BaseView *v)
//{
//	view=v;
//}
/*
BaseView * BaseDocument::getView()
{
	return (BaseView*)view;
}*/
int BaseDocument::getType()
{
	return type;
}


/////////////////////////////////////////////////////////////////////////
// Object container. STL container wrapper
/////////////////////////////////////////////////////////////////////////

Container::Container(const Container &c)
:data(c.data)
{
}
Container::Container(const std::vector<BaseDocument*> &v)
:data(v)
{
}
Container::~Container()
{
}
int Container::getCount()
{
	return data.size();
}
BaseDocument ** Container::getData()
{
	return  &data[0];
}

/////////////////////////////////////////////////////////////////////////
// Workspace Class. Object management
/////////////////////////////////////////////////////////////////////////
Workspace::Workspace()
{
	gWorkspace=this;
}
Workspace::~Workspace()
{
	std::list<BaseDocument*>::iterator it=documents.begin();
	for(;it!=documents.end();it++)
		delete (*it);
	documents.clear();
}
HRESULT Workspace::makeDialog(TString &RetPath)
{
/*	CFileDialog dlg(true);
	CString str=_T("All files (*.*)");str+=(TCHAR)NULL;
	str+=_T("*.*");str+=(TCHAR)NULL;
	str+=_T("BMP image");str+=(TCHAR)NULL;
	str+=_T("*.bmp");str+=(TCHAR)NULL;
	str+=_T("TGA image");str+=(TCHAR)NULL;
	str+=_T("*.tga");str+=(TCHAR)NULL;
	str+=_T("JPEG image");str+=(TCHAR)NULL;
	str+=_T("*.jpg");str+=(TCHAR)NULL;
	str+=_T("GIF image");str+=(TCHAR)NULL;
	str+=_T("*.gif");str+=(TCHAR)NULL;
	str+=_T("PCX image");str+=(TCHAR)NULL;
	str+=_T("*.pcx");str+=(TCHAR)NULL;
	TCHAR strName[_MAX_PATH]=_T("");
	dlg.m_ofn.lpstrFilter=str;
	dlg.m_ofn.nFilterIndex=1;
	dlg.m_ofn.lpstrFile=strName;
	//dlg.m_ofn.
	
	if(dlg.DoModal()!=IDOK)
	{
		return S_FALSE;
	}
	RetPath=strName;*/
	return S_OK;
}
HRESULT Workspace::docLoad(const TCHAR *Path,const char *Name)
{
	assert(Path!=NULL);
	TStringC path=T2C(Path);
	int w,h;
	Image *img=new Image(path.c_str(),this,Name);

	if(img->isValid())
	{
		w=img->getWidth();
		h=img->getHeight();
		MatrixFilter f(1,this);
		f(-1,-1)= 1;f( 0,-1)=-1;f( 1,-1)= 0;
		f(-1, 0)=-1;f( 0, 0)= 1;f( 1, 0)= 0;
		f(-1, 1)= 0;f( 0, 1)= 0;f( 1, 1)= 0;
		
		//GaussDistribution2D gd(0,0,1.5);
		Image *tmp=new Image(w,h,img->getPixelFormat(),this);

		f.applyTo(*tmp,*img);
		documents.push_back(tmp);
		documents.push_back(img);
		//mainFrame->addView(img);
		updateListeners();
	}
	else
	{
		delete img;
		return S_FALSE;
	}
	return S_OK;

}

//void Workspace::setMainFrame(MainFrame *wnd)
//{
//	mainFrame=wnd;
//}

void Workspace::showView(BaseDocument *doc)
{
	/*if(mainFrame)
		mainFrame->addView(doc);*/
}

Container Workspace::getImages(char *path)
{
	// parameter "path" not used
	std::list<BaseDocument*>::iterator it;
	std::vector<BaseDocument *> res;
	for(it=documents.begin();it!=documents.end();it++)
	{
		if((*it)->getType()==documentTypeImage)
			res.insert(res.end(),*it);
	}
	return Container(res);
}
Container Workspace::getFilters(char *path)
{
	// parameter "path" not used
	std::list<BaseDocument*>::iterator it;
	std::vector<BaseDocument *> res;
	for(it=documents.begin();it!=documents.end();it++)
	{
		if((*it)->getType()==documentTypeFilter)
			res.insert(res.end(),*it);
	}
	return Container(res);
}

BaseDocument * Workspace::addCorrelation()
{
	BaseDocument * doc=new Correlation(10,0,this);
	if(doc==NULL)
		return NULL;
	documents.push_back(doc);
	updateListeners();
	return doc;
}

BaseDocument * Workspace::addMatrixFilter()
{
	BaseDocument * doc=new MatrixFilter(2,this);
	if(doc==NULL)
		return NULL;
	documents.push_back(doc);
	updateListeners();
	return doc;
}

Container Workspace::applyFilter(Container data, BaseDocument *filter, char *result)
{
	assert(filter);
	assert(filter->getType()==documentTypeFilter);
	//Container res;
	Filter *f=(Filter*)filter;
	Image *tmp;
	if(result==NULL)	//save item data to arguments
	{
		for(int i=0;i<data.getCount();i++)
		{
			
		}
	}
	return Container(data);
}

HRESULT Workspace::save(const TCHAR *path)
{
	std::ofstream out(path);
	std::list<BaseDocument*>::iterator it;
	for(it=documents.begin();it!=documents.end();it++)
	{
		out<<(*it)->getType();
		out<<(*it)->getName();
		(*it)->save(out);
	}
	return S_OK;
}
HRESULT Workspace::load(const TCHAR *path)
{
	//this->
	int type;
	std::string tmpStr;
	BaseDocument *object=NULL;
	std::ifstream in(path);
	std::list<BaseDocument*>::iterator it;
	for(it=documents.begin();it!=documents.end();it++)
	{		
		in>>type;
		in>>tmpStr;	//read className;
		object=createObject(tmpStr);
		switch(type)
		{
			//case 
		}
		in>>tmpStr;
		//(*it)->save(out);
	}
	return S_OK;
}
BaseDocument * Workspace::createObject(const std::string &str)
{
	if(str=="image")
		return new Image(this);
	return NULL;
=======
#include "stdafx.h"

#include "Document.h"
//#include "resource.h"
//#include "BaseView.h"
#include "ImageTools.h"
//#include "MainFrm.h"

#include "fstream"


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

Workspace *gWorkspace=NULL;
///////////////////////////////////////////////////////////////////////
/// Base Object
///////////////////////////////////////////////////////////////////////

BaseDocument::BaseDocument(const char *str,Workspace *ws,int nType)
:name(str)
,owner(ws)
,type(nType)
//,ListenerSingle(NULL)
//,view(NULL)
{}

BaseDocument::~BaseDocument()
{
	/*if(view)
		delete view;*/
}

void BaseDocument::show()
{
	owner->showView(this);
}

void BaseDocument::hide()
{
}

void BaseDocument::setName(const std::string &str)
{
	name=str;
}
std::string BaseDocument::getName()
{
	return name;
}

//void BaseDocument::setView(BaseView *v)
//{
//	view=v;
//}
/*
BaseView * BaseDocument::getView()
{
	return (BaseView*)view;
}*/
int BaseDocument::getType()
{
	return type;
}


/////////////////////////////////////////////////////////////////////////
// Object container. STL container wrapper
/////////////////////////////////////////////////////////////////////////

Container::Container(const Container &c)
:data(c.data)
{
}
Container::Container(const std::vector<BaseDocument*> &v)
:data(v)
{
}
Container::~Container()
{
}
int Container::getCount()
{
	return data.size();
}
BaseDocument ** Container::getData()
{
	return  &data[0];
}

/////////////////////////////////////////////////////////////////////////
// Workspace Class. Object management
/////////////////////////////////////////////////////////////////////////
Workspace::Workspace()
{
	gWorkspace=this;
}
Workspace::~Workspace()
{
	std::list<BaseDocument*>::iterator it=documents.begin();
	for(;it!=documents.end();it++)
		delete (*it);
	documents.clear();
}
HRESULT Workspace::makeDialog(TString &RetPath)
{
/*	CFileDialog dlg(true);
	CString str=_T("All files (*.*)");str+=(TCHAR)NULL;
	str+=_T("*.*");str+=(TCHAR)NULL;
	str+=_T("BMP image");str+=(TCHAR)NULL;
	str+=_T("*.bmp");str+=(TCHAR)NULL;
	str+=_T("TGA image");str+=(TCHAR)NULL;
	str+=_T("*.tga");str+=(TCHAR)NULL;
	str+=_T("JPEG image");str+=(TCHAR)NULL;
	str+=_T("*.jpg");str+=(TCHAR)NULL;
	str+=_T("GIF image");str+=(TCHAR)NULL;
	str+=_T("*.gif");str+=(TCHAR)NULL;
	str+=_T("PCX image");str+=(TCHAR)NULL;
	str+=_T("*.pcx");str+=(TCHAR)NULL;
	TCHAR strName[_MAX_PATH]=_T("");
	dlg.m_ofn.lpstrFilter=str;
	dlg.m_ofn.nFilterIndex=1;
	dlg.m_ofn.lpstrFile=strName;
	//dlg.m_ofn.
	
	if(dlg.DoModal()!=IDOK)
	{
		return S_FALSE;
	}
	RetPath=strName;*/
	return S_OK;
}
HRESULT Workspace::docLoad(const TCHAR *Path,const char *Name)
{
	assert(Path!=NULL);
	TStringC path=T2C(Path);
	int w,h;
	Image *img=new Image(path.c_str(),this,Name);

	if(img->isValid())
	{
		w=img->getWidth();
		h=img->getHeight();
		MatrixFilter f(1,this);
		f(-1,-1)= 1;f( 0,-1)=-1;f( 1,-1)= 0;
		f(-1, 0)=-1;f( 0, 0)= 1;f( 1, 0)= 0;
		f(-1, 1)= 0;f( 0, 1)= 0;f( 1, 1)= 0;
		
		//GaussDistribution2D gd(0,0,1.5);
		Image *tmp=new Image(w,h,img->getPixelFormat(),this);

		f.applyTo(*tmp,*img);
		documents.push_back(tmp);
		documents.push_back(img);
		//mainFrame->addView(img);
		updateListeners();
	}
	else
	{
		delete img;
		return S_FALSE;
	}
	return S_OK;

}

//void Workspace::setMainFrame(MainFrame *wnd)
//{
//	mainFrame=wnd;
//}

void Workspace::showView(BaseDocument *doc)
{
	/*if(mainFrame)
		mainFrame->addView(doc);*/
}

Container Workspace::getImages(char *path)
{
	// parameter "path" not used
	std::list<BaseDocument*>::iterator it;
	std::vector<BaseDocument *> res;
	for(it=documents.begin();it!=documents.end();it++)
	{
		if((*it)->getType()==documentTypeImage)
			res.insert(res.end(),*it);
	}
	return Container(res);
}
Container Workspace::getFilters(char *path)
{
	// parameter "path" not used
	std::list<BaseDocument*>::iterator it;
	std::vector<BaseDocument *> res;
	for(it=documents.begin();it!=documents.end();it++)
	{
		if((*it)->getType()==documentTypeFilter)
			res.insert(res.end(),*it);
	}
	return Container(res);
}

BaseDocument * Workspace::addCorrelation()
{
	BaseDocument * doc=new Correlation(10,0,this);
	if(doc==NULL)
		return NULL;
	documents.push_back(doc);
	updateListeners();
	return doc;
}

BaseDocument * Workspace::addMatrixFilter()
{
	BaseDocument * doc=new MatrixFilter(2,this);
	if(doc==NULL)
		return NULL;
	documents.push_back(doc);
	updateListeners();
	return doc;
}

Container Workspace::applyFilter(Container data, BaseDocument *filter, char *result)
{
	assert(filter);
	assert(filter->getType()==documentTypeFilter);
	//Container res;
	Filter *f=(Filter*)filter;
	Image *tmp;
	if(result==NULL)	//save item data to arguments
	{
		for(int i=0;i<data.getCount();i++)
		{
			
		}
	}
	return Container(data);
}

HRESULT Workspace::save(const TCHAR *path)
{
	std::ofstream out(path);
	std::list<BaseDocument*>::iterator it;
	for(it=documents.begin();it!=documents.end();it++)
	{
		out<<(*it)->getType();
		out<<(*it)->getName();
		(*it)->save(out);
	}
	return S_OK;
}
HRESULT Workspace::load(const TCHAR *path)
{
	//this->
	int type;
	std::string tmpStr;
	BaseDocument *object=NULL;
	std::ifstream in(path);
	std::list<BaseDocument*>::iterator it;
	for(it=documents.begin();it!=documents.end();it++)
	{		
		in>>type;
		in>>tmpStr;	//read className;
		object=createObject(tmpStr);
		switch(type)
		{
			//case 
		}
		in>>tmpStr;
		//(*it)->save(out);
	}
	return S_OK;
}
BaseDocument * Workspace::createObject(const std::string &str)
{
	if(str=="image")
		return new Image(this);
	return NULL;
>>>>>>> bd17e7de82f5e0c1302ffcdcf53da1a9448d3f2d
}