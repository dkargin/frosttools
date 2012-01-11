#pragma once;

#include "3dmath.h"

enum ColorType
{
	colorTypeRGB,
	colorTypeRGBA,
	colorTypeIllumination,
};

enum DataObjectType
{
	dataObjectTypeDiagram,
	dataObjectTypeMap2D,
	dataObjectTypeGraph,
	dataObjectGeometry2D,
	dataObjectGeometry3D
};
//объект для отрисовки
class DataObject
{
	int objectType;
	Matrix pose;
	vec3 color;
public:
	DataObject(int type);
	virtual void begin();
	virtual void end();

	virtual void setPose(const Matrix &m);
	virtual void setPosition(const vec3 &v);
	virtual void setColor(float *color);
	virtual void setColor(float r,float g,float b);
	virtual void selLineWidth(int width);
	virtual void setPointSize(int size);

	virtual int getPointSize();
	virtual int getLineWidth();
	virtual vec3 getPosition();
	virtual Matrix getPose();
	virtual void render()=0;
	virtual bool isType(int type);

	
};
namespace DataObjects
{

class Geometry3D
{
	vec3 currentPos;
public:
	virtual void  moveTo3D(float x,float y,float z);	
	virtual void  lineTo3D(int x,int y);	
	virtual void  line3D(float x1,float y1,float x2,float y2);
};
class Geometry2D: public DataObject
{
	float lineWidth;
	float lineColor[4];
	float brushColor[4];
	vec3 last;
public:
	Geometry2D();
	virtual void drawRect(int x1,int y1,int x2,int y2);
	virtual void moveTo(int x,int y);
	virtual void lineTo(int x,int y);
	virtual void line(int x1,int y1,int x2,int y2);	
	virtual void render();
};
///////////////////////////////////////////////////////////////////
// Object for operations concerned to raster map
///////////////////////////////////////////////////////////////////
class Map2D: public DataObject
{
	float cellWidth;
	float cellHeight;
	float cellIntervalHor;
	float cellIntervalVer;
	int mapWidth;
	int mapHeight;
	GLuint textureName;
	bool bUseTexture;
	unsigned char *texData;
public:
	Map2D();
	void initCellSize(float w,float h);
	void initMapSize(int w,int h);

	virtual void begin();
	virtual void render();
	virtual void drawCell(int x,int y);
	virtual void drawLine(float x1,float y1,float x2,float y2);
	virtual void drawCellCircle(float x,float y);
	virtual void drawCellsw(WORD *data,int x0,int y0,int x1,int y1);
	virtual void drawCellsf(float *data,int x0,int y0,int x1,int y1);	

	void createTexture(WORD *data);
	void updateTexture(WORD *data,int x0,int y0,int x1,int y1);
};

class Text: public DataObject
{
	virtual void drawText(float x,float y,const char *text,...);
};
};
//старая версия рендерера, скоро будет заменена
class DataRenderer//: public IDataRenderer
{
	int lineWidth;
	int glutWindow;
	COLORREF brushColor;
	COLORREF penColor;
	enum controlMode
	{
		modeGDI,
		modeGLUT,
	}controlMode;	
public:
	DataRenderer(HDC dc,int cw,int ch,int cihor,int civer);
	DataRenderer(int glutWindow);
	~DataRenderer();
	bool initDC(HDC dc);

	virtual void initSize(int cellSize,int cellDistance);

	virtual void setColorBorder(BYTE r,BYTE g,BYTE b);
	virtual void setColorFill(BYTE r,BYTE g,BYTE b);
	virtual void setLineWidth(int w);
	//virtual void drawCell(int x,int y);
	//virtual void drawLine(int x1,int y1,int x2,int y2);
	//virtual void drawCellCircle(int x,int y);

	virtual void start();
	virtual void finish();
	virtual void render();

	virtual void setViewport(int x,int y,int w,int h);
	DataObject *createObject(int type,const std::string &name);
	DataObject *getObject(const std::string& name);

	//RECT GetCellRect(DWORD x,DWORD y);
	//POINT GetCellCenter(DWORD x,DWORD y);
	//POINT GetCellLT(DWORD x,DWORD y);
protected:
	typedef std::map<std::string,DataObject*> Container;
	typedef Container::iterator ContainerIt;
	Container dataObjects;
	HDC hdc;
	HGLRC hrc;
};
