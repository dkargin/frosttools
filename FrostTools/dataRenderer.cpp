#include "stdafx.h"

/////////////////////////////////////////////////////////////////////
// Здесь я занимаюсь хернёй всякой. Скорее всего всё это бесполезно
/////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "dataRenderer.h"

#include <gl\gl.h>
#include <gl\glut.h>

#pragma comment(lib,"opengl32.lib");

#define CELL_WIDTH 3
#define CELL_HEIGHT 3
#define CELL_HORDIST 0
#define CELL_VERDIST 0

#pragma warning(disable:4244)

template <class Type>
inline void swap(Type &a,Type &b)
{
	Type tmp=a;a=b;b=tmp;
}
//unsigned char texData[512*512*3]; 
float * glMatrix(Matrix Mat)
{
	Matrix Mt=Mat;
	Mt.transpose();
	glMultMatrixf((float*)Mt.x);		
	return (float*)Mt.x;
}

DataObject::DataObject(int type)
:pose(1),objectType(type)
{

}
void DataObject::selLineWidth(int width)
{
	glLineWidth(width);
}
void DataObject::setPointSize(int size)
{
	glPointSize(size);
}
int DataObject::getLineWidth()
{
	int res;
	glGetIntegerv(GL_LINE_WIDTH,&res);
	return res;
}
int DataObject::getPointSize()
{
	int res;
	glGetIntegerv(GL_POINT_SIZE,&res);
	return res;	
}
void DataObject::setPose(const Matrix &m)
{
	pose=m;
}

void DataObject::setPosition(const vec3 &v)
{
	pose.origin(v);
}

void DataObject::setColor(float *color)
{
	glColor3fv(color);
}
void DataObject::setColor(float r,float g,float b)
{
	glColor3f(r,g,b);
}

bool DataObject::isType(int type)
{
	return type==objectType;
}

Matrix DataObject::getPose()
{
	return pose;
}

vec3 DataObject::getPosition()
{
	return pose.origin();
}

void DataObject::begin()
{
	glPushMatrix();
	glMatrix(pose);
	
}
void DataObject::end()
{
	glPopMatrix();
}

DataObjects::Map2D::Map2D()
:DataObject(DataObjectType::dataObjectTypeMap2D)
{
	cellWidth=1;
	cellHeight=1;
	mapWidth=0;
	mapHeight=0;
	textureName=0;
	bUseTexture=false;

}

void DataObjects::Map2D::initCellSize(float w,float h)
{
	cellWidth=w;
	cellHeight=h;	
}
void DataObjects::Map2D::begin()
{
	DataObject::begin();
	glScalef(cellWidth,cellHeight,1);
}
void DataObjects::Map2D::drawCell(int x,int y)
{	
	//glColor3f(0,0,0);
	glBegin(GL_QUADS);
	glVertex2f(x-0.5,y-0.5);
	glVertex2f(x+0.5,y-0.5);
	glVertex2f(x+0.5,y+0.5);
	glVertex2f(x-0.5,y+0.5);
	glEnd();
}
void DataObjects::Map2D::drawLine(float x1,float y1,float x2,float y2)
{
	glBegin(GL_LINES);
	glVertex2f(x1,y1);
	glVertex2f(x2,y2);
	glEnd();
}
void DataObjects::Map2D::drawCellCircle(float x,float y)
{
	//glColor3f(1,0,0);
	//glPointSize(cellWidth);
	glBegin(GL_QUADS);
	glVertex3f(x-0.5,y,1.5);
	glVertex3f(x,y+0.5,1.5);
	glVertex3f(x+0.5,y,1.5);
	glVertex3f(x,y-0.5,1.5);	
	glEnd();
	/*glBegin(GL_POINTS);
	glVertex3f(x,y,1.0);
	glEnd();*/
}
void DataObjects::Map2D::render()
{
	if(bUseTexture)
	{
		glEnable(GL_TEXTURE_2D);
		//glBindTexture(GL_TEXTURE_2D,textureName);
		glBegin(GL_QUADS);
		glTexCoord2f(0,0);glVertex2f(-0.5,-0.5);
		glTexCoord2f(1,0);glVertex2f(mapWidth+0.5,-0.5);
		glTexCoord2f(1,1);glVertex2f(mapWidth+0.5,mapHeight+0.5);
		glTexCoord2f(0,1);glVertex2f(-0.5,mapHeight+0.5);	
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
}
void DataObjects::Map2D::drawCellsf(float *data, int x0, int y0, int x1, int y1)
{
}
void DataObjects::Map2D::drawCellsw(WORD *data, int x0, int y0, int x1, int y1)
{
}
void DataObjects::Map2D::initMapSize(int w, int h)
{
	mapWidth=w;
	mapHeight=h;
}
void DataObjects::Map2D::createTexture(WORD *data)
{
	if(!mapWidth || !mapHeight)
		return;
	if(!textureName)
		glGenTextures(1,&textureName);
	else
		glBindTexture(GL_TEXTURE_2D,textureName);
	texData=new unsigned char[3*mapWidth*mapHeight];
	for(int i=0,j=0;i<mapWidth*mapHeight;i++)
	{
		texData[j++]=data[i];
		texData[j++]=data[i];
		texData[j++]=data[i];
	}
	glTexImage2D(GL_TEXTURE_2D,0,3,mapWidth,mapHeight,0,GL_RGB,GL_UNSIGNED_BYTE,texData);
	bUseTexture=true;
}
void DataObjects::Map2D::updateTexture(WORD *data, int x0, int y0, int x1, int y1)
{
}
DataObjects::Geometry2D::Geometry2D()
:DataObject(DataObjectType::dataObjectGeometry2D),last(0,0,0)
{
}
/*
void DataObjects::Geometry2D::begin()
{
	DataObject::begin();
}*/

void DataObjects::Geometry2D::moveTo(int x,int y)
{
	last.x=x;
	last.y=y;
	last.z=0;
}
void DataObjects::Geometry2D::lineTo(int x,int y)
{
	glBegin(GL_LINES);
	glVertex2i(last.x,last.y);
	glVertex2i(x,y);
	glEnd();	
	last.x=x;
	last.y=y;
	last.z=0;
}
void DataObjects::Geometry2D::line(int x1,int y1,int x2,int y2)
{
	moveTo(x1,y1);
	lineTo(x2,y2);
}
void DataObjects::Geometry2D::render()
{
}
void DataObjects::Geometry2D::drawRect(int x1,int y1,int x2,int y2)
{
	//if(
	moveTo(x1,y1);
	lineTo(x2,y1);
	lineTo(x2,y2);
	lineTo(x1,y2);
	lineTo(x1,y1);
}

DataRenderer::DataRenderer(HDC dc, int cw, int ch, int cihor, int civer)
{
	hdc=dc;	
	setColorBorder(128,10,14);
	controlMode=modeGDI;
	lineWidth=1;
}
DataRenderer::DataRenderer(int window)
:glutWindow(window)
{
	controlMode=modeGLUT;
}
DataRenderer::~DataRenderer()
{
	ContainerIt it;
	for(it=dataObjects.begin();it!=dataObjects.end();it++)
		delete (it->second);
}
void DataRenderer::initSize(int cellSize,int cellDistance)
{
//	cellWidth=cellSize;
//	cellHeight=cellSize;
//	cellIntervalHor=cellDistance;
//	cellIntervalVer=cellDistance;
}

bool DataRenderer::initDC(HDC dc)
{
	hdc=dc;
	// Заполняем поля структуры
	static PIXELFORMATDESCRIPTOR pfd = 	{
	sizeof(PIXELFORMATDESCRIPTOR),  // размер структуры
	1,                              // номер версии
	PFD_DRAW_TO_WINDOW   |          // поддержка вывода в окно
	PFD_SUPPORT_OPENGL   |          // поддержка OpenGL
	PFD_DOUBLEBUFFER,               // двойная буферизация
	PFD_TYPE_RGBA,         // цвета в режиме RGBA
	24,                    // 32-разряда на цвет
	0, 0, 0, 0, 0, 0,      // биты цвета игнорируются
	0,                     // не используется альфа параметр
	0,                     // смещение цветов игнорируются
	0,                     // буфер аккумулятора не используется
	0, 0, 0, 0,            // биты аккумулятора игнорируются
	32,                    // 32-разрядный буфер глубины
	0,                     // буфер трафарета не используется
	0,                     // вспомогательный буфер не используется
	PFD_MAIN_PLANE,        // основной слой
	0,                     // зарезервирован
	0, 0, 0                // маски слоя игнорируются
	};

	int pixelFormat;

	// Поддерживает ли система необходимый формат пикселей?
	if((pixelFormat = ::ChoosePixelFormat(hdc, &pfd)) == 0){
		MessageBox(0,_T("С заданным форматом пикселей работать нельзя"),_T("GL output"),MB_OK);
		return FALSE;
	}
	if (::SetPixelFormat(hdc, pixelFormat, &pfd) == FALSE)
	{
		MessageBox(0,_T("Ошибка при выполнении SetPixelFormat"),_T("GL output"),MB_OK);
		return FALSE;
	}
	if((hrc = ::wglCreateContext(hdc)) == NULL) return false;
	return TRUE;
}
void DataRenderer::setColorBorder(BYTE r,BYTE g,BYTE b)
{
	penColor=RGB(r,g,b);
}
void DataRenderer::setColorFill(BYTE r,BYTE g,BYTE b)
{
}
void DataRenderer::setLineWidth(int w)
{
	lineWidth=w;
}

void DataRenderer::render()
{
	for(ContainerIt it=dataObjects.begin();it!=dataObjects.end();it++)
	{
		it->second->begin();
		it->second->render();
		it->second->end();
	}
}
void DataRenderer::finish()
{	
	if(controlMode==modeGLUT)
		glutSwapBuffers();
	else
	{
		glFinish();
		SwapBuffers(hdc);
	}
}
void DataRenderer::start()
{
	if(controlMode==modeGDI && !wglMakeCurrent(hdc, hrc))return;
	glClearColor(1.0f, 0.5f, 0.5f, 1.0f); //Background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear buffers
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glEnable(GL_DEPTH_TEST);
	int err=glGetError();
}

void DataRenderer::setViewport(int x, int y, int w, int h)
{
	if(controlMode==modeGDI &&!wglMakeCurrent(hdc, hrc))return;
	glViewport(0,0,w,h);	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,w,h,0,-100,100);
	glMatrixMode(GL_MODELVIEW);
}

DataObject *DataRenderer::createObject(int type,const std::string &name)
{
	DataObject *object=NULL;
	switch(type)
	{
	case DataObjectType::dataObjectTypeMap2D:
		object=(DataObject*)new DataObjects::Map2D();
		break;
	case DataObjectType::dataObjectGeometry2D:
		object=(DataObject*)new DataObjects::Geometry2D();
		break;
	}
	if(object)
		dataObjects[name]=(object);
	return object;
}
DataObject * DataRenderer::getObject(const std::string &name)
{
	if(dataObjects.find(name)!=dataObjects.end())
		return dataObjects[name];
	else
		return NULL;
}

