<<<<<<< HEAD
#pragma once;
class Workspace;
class BaseView; // tolua_export
class BaseDocument;
class MainFrame; // tolua_export
//#include "Lua\luaBase.h"

#include "frostTools.h"


enum DocumentType
{
	documentTypeImage,
	documentTypeFilter,
	documentTypeOperator
};

class Container
{
	int dataType;
	std::vector<BaseDocument*> data;
public:
	Container(const Container &);
	Container(const std::vector<BaseDocument*> &);
	~Container();
	int getCount();
	BaseDocument ** getData();
	int getDataType();
};

class BaseDocument//:public ListenerSingle<BaseDocument>,public Listening<BaseDocument> // tolua_export
{ // tolua_export
protected:
	Workspace	*owner;
	//CWnd		*view;
	std::string name;
	struct objectDef
	{
		int classType:4;		
	};
	int type;
	

	TCHAR documentPath[MAX_PATH];
public:
	BaseDocument(const char *name,Workspace *ws,int nType);
	virtual ~BaseDocument();

	void setName(const std::string &str);
	void setView(BaseView * view);
	//int copyObject(O)
	std::string getName();
	//BaseView * getView();
	int getType();

	void update(); 

	virtual void show(); // tolua_export
	virtual void hide(); // tolua_export

	virtual int save(std::ofstream &out)=0;
	virtual int load(std::ifstream &in)=0;
	//tolua_begin
	//virtual BaseView * createView(MainFrame *frm)=0;

};
//tolua_end
class Workspace: public Listening<Workspace>
{
public:
	std::list<BaseDocument*> documents;
	std::set<ListenerSingle<Workspace> *> listeners;
	std::map<std::string,int> nameCounter;				//for name generator;

	MainFrame *mainFrame;	
	TCHAR *workspacePath;
public:
	Workspace();
	~Workspace();	

	Container applyFilter(Container data, BaseDocument *filter, char *result);
	Container getImages(char *path);	//get all images from path
	Container getFilters(char *path);	//get al filters from path

	Container * openImages();			//open a set of images from directory	

	void showView(BaseDocument *doc);
	void setMainFrame(MainFrame *wnd);
	
	HRESULT makeDialog(TString &RetPath);
	HRESULT docLoad(const TCHAR *Path,const char *Name=NULL);

	HRESULT documentOpen()
	{
		TString S;
		HRESULT R;
		if (S_OK!=(R=makeDialog(S)))
			return R;
		return docLoad(S.c_str());
	}
	HRESULT load(const TCHAR *path);
	HRESULT save(const TCHAR *path);

	BaseDocument * addMatrixFilter();
	BaseDocument * addCorrelation();

	BaseDocument * createObject(const std::string &str);

	std::string getName(const std::string &name);
=======
#pragma once;
class Workspace;
class BaseView; // tolua_export
class BaseDocument;
class MainFrame; // tolua_export
//#include "Lua\luaBase.h"

#include "frostTools.h"


enum DocumentType
{
	documentTypeImage,
	documentTypeFilter,
	documentTypeOperator
};

class Container
{
	int dataType;
	std::vector<BaseDocument*> data;
public:
	Container(const Container &);
	Container(const std::vector<BaseDocument*> &);
	~Container();
	int getCount();
	BaseDocument ** getData();
	int getDataType();
};

class BaseDocument//:public ListenerSingle<BaseDocument>,public Listening<BaseDocument> // tolua_export
{ // tolua_export
protected:
	Workspace	*owner;
	//CWnd		*view;
	std::string name;
	struct objectDef
	{
		int classType:4;		
	};
	int type;
	

	TCHAR documentPath[MAX_PATH];
public:
	BaseDocument(const char *name,Workspace *ws,int nType);
	virtual ~BaseDocument();

	void setName(const std::string &str);
	void setView(BaseView * view);
	//int copyObject(O)
	std::string getName();
	//BaseView * getView();
	int getType();

	void update(); 

	virtual void show(); // tolua_export
	virtual void hide(); // tolua_export

	virtual int save(std::ofstream &out)=0;
	virtual int load(std::ifstream &in)=0;
	//tolua_begin
	//virtual BaseView * createView(MainFrame *frm)=0;

};
//tolua_end
class Workspace: public Listening<Workspace>
{
public:
	std::list<BaseDocument*> documents;
	std::set<ListenerSingle<Workspace> *> listeners;
	std::map<std::string,int> nameCounter;				//for name generator;

	MainFrame *mainFrame;	
	TCHAR *workspacePath;
public:
	Workspace();
	~Workspace();	

	Container applyFilter(Container data, BaseDocument *filter, char *result);
	Container getImages(char *path);	//get all images from path
	Container getFilters(char *path);	//get al filters from path

	Container * openImages();			//open a set of images from directory	

	void showView(BaseDocument *doc);
	void setMainFrame(MainFrame *wnd);
	
	HRESULT makeDialog(TString &RetPath);
	HRESULT docLoad(const TCHAR *Path,const char *Name=NULL);

	HRESULT documentOpen()
	{
		TString S;
		HRESULT R;
		if (S_OK!=(R=makeDialog(S)))
			return R;
		return docLoad(S.c_str());
	}
	HRESULT load(const TCHAR *path);
	HRESULT save(const TCHAR *path);

	BaseDocument * addMatrixFilter();
	BaseDocument * addCorrelation();

	BaseDocument * createObject(const std::string &str);

	std::string getName(const std::string &name);
>>>>>>> bd17e7de82f5e0c1302ffcdcf53da1a9448d3f2d
};