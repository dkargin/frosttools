#include "CDrawUtil.h"
#include "utility.h"
#include "glTools.h"

// apply parameters using glEnableClientState/glDisableClientState
GLint ApplyClientState(GLint param, GLint state)
{
	GLint oldState = glIsEnabled(param);
	if(oldState != state)
		state?glEnableClientState(param):glDisableClientState(param);
	return oldState;		
}

// apply parameters using glEnable/glDisable
GLint ApplyBase(GLint param, GLint state)
{
	GLint oldState = glIsEnabled(param);
	if(oldState != state)
		state?glEnable(param):glDisable(param);	
	return oldState;
}

// apply parameters using glEnable/glDisable
GLint ApplyMatrixMode(GLint param, GLint state)
{
	GLint oldState = 0;
	glGetIntegerv(GL_MATRIX_MODE, &oldState);

	if(oldState != state)
		glMatrixMode(state);		
	return oldState;
}

// apply glShadeModel
GLint ApplyShadeModel(GLint param, GLint state)
{
	GLint oldState = 0;
	glGetIntegerv(GL_SHADE_MODEL, &oldState);

	if(oldState != state)
		glShadeModel(state);		
	return oldState;
}

// apply blending parameters using glEnable/glDisable
GLint ApplyBlend(GLint param, GLint state)
{
	GLint src, dst;
	int err0 = glGetError();
	glGetIntegerv(GL_BLEND_SRC, &src);
	glGetIntegerv(GL_BLEND_DST, &dst);
	int err1 = glGetError();
	GLint oldState = 0;

	if(param == GL_BLEND_SRC)
	{	
		oldState = src;
		if(oldState != state)
			glBlendFunc(state,dst);
	}
	else if(param == GL_BLEND_DST)
	{
		oldState = dst;
		if(oldState != state)
			glBlendFunc(src,state);
	}
	int err2 = glGetError();
	return oldState;
}

// save previous state and set new state
GLParam::GLParam(GLint param, GLint state)
	:param(param), state(state), wrapper(NULL)
{			
	wrapper = GetWrapper(param);
	ASSERT(wrapper);
	oldState = wrapper(param,state);
}

// restore old state
GLParam::~GLParam()
{
	wrapper(param, oldState);
}

// get wrapper for specific state type
GLParam::ParamWrapper GLParam::GetWrapper(GLint param)
{
	switch(param)
	{
	case GL_BLEND_SRC:
	case GL_BLEND_DST:
		return ApplyBlend;
	case GL_SHADE_MODEL:
	case GL_SMOOTH:
	case GL_FLAT:
		return ApplyShadeModel;
	case GL_VERTEX_ARRAY:
	case GL_COLOR_ARRAY:
	case GL_TEXTURE_COORD_ARRAY:
	case GL_NORMAL_ARRAY:
		return ApplyClientState;
	case GL_MATRIX_MODE:
		return ApplyMatrixMode;
	default:
		return ApplyBase;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// CMeshLine implementation
///////////////////////////////////////////////////////////////////////////////////
CMeshLine::CMeshLine()
	:type(TypeUp), primitivesCount(0)
{}

//
void CMeshLine::InitBuffers(int maxLength)
{
	line.Allocate(maxLength);
	vertexBuffer.Allocate((maxLength + 1) * 2);	
}

size_t CMeshLine::InsertVertex(size_t index, const vec2& pos, const vec2 & up, const Color & color)
{
	if(type == TypeUp)		
	{
		vertexBuffer[index].pos = pos + up;
		//vertexBuffer[index].color = color;	
		vertexBuffer[index].color.a = 0.f;	
		vertexBuffer[index].color.r = 0.f;	
		vertexBuffer[index].color.g = 0.f;	
		vertexBuffer[index].color.b = 0.f;	
		vertexBuffer[index+1].pos = pos;	
		vertexBuffer[index+1].color = color;		
	}
	else if(type == TypeDown)		
	{
		vertexBuffer[index].pos = pos - up;		
		vertexBuffer[index].color.a = 0.f;	
		vertexBuffer[index].color.r = 0.f;	
		vertexBuffer[index].color.g = 0.f;	
		vertexBuffer[index].color.b = 0.f;	
		vertexBuffer[index+1].pos = pos;	
		vertexBuffer[index+1].color = color;		
	}
	else
	{
		vertexBuffer[index].pos = pos + up;
		vertexBuffer[index].color = color;		
		vertexBuffer[index+1].pos = pos - up;	
		vertexBuffer[index+1].color = color;		
	}
	return index + 2;
}

vec2 CMeshLine::GetUpVector(size_t index, size_t size) const
{
	ASSERT(size > 2);
	//return vec2(0,lineWidth);

	if(index == 0)
	{
		vec2 dir = line[1] - line[0];
		dir.Normalize();	
		return vec2(-dir.y, dir.x ) * lineWidth;
	}
	else if(index == size - 1)
	{
		vec2 dir = line[size - 1] - line[size - 2];
		dir.Normalize();	
		return vec2(-dir.y, dir.x ) * lineWidth;
	}	
	else //if(index > 1)
	{
		vec2 dir = line[index] - line[index - 1];
		dir.Normalize();	
		return vec2(-dir.y, dir.x ) * lineWidth;
	}	
	/*
	else 
	{
		vec2 dirPrev = (line[index] - line[index-1]).Normalize();
		vec2 dirCur = (line[index+1] - line[index]).Normalize();		
		vec2 dir = (dirCur + dirPrev).Normalize() * lineWidth;
		return vec2(-dir.y, dir.x);
	}*/
	
}
CMeshLine::Color CMeshLine::Interpolate(const CMeshLine::Color &start, const CMeshLine::Color &end, size_t index, size_t size) const
{	
	float factor = (float)index / size;
	float invFactor = 1.f - factor;

	Color result;
	result.r = start.r * invFactor + end.r * factor;
	result.g = start.g * invFactor + end.g * factor;
	result.b = start.b * invFactor + end.b * factor;
	result.a = start.a * invFactor + end.a * factor;
	return result;
}

void CMeshLine::UpdateLine(size_t size, const Color & start, const Color & end, float widthFactor)
{
	ASSERT( size <= line.Length() );

	primitivesCount = 0;
	for(size_t i = 0; i < size; i++)
	{
		primitivesCount = InsertVertex(primitivesCount, line[i], GetUpVector(i, size), Interpolate(start, end, i, size));		
	}	
};

void CMeshLine::Draw()
{
	if(primitivesCount)
	{
		GLParam params[] = 
		{
			GLParam(GL_TEXTURE_2D, FALSE),
			GLParam(GL_BLEND, TRUE),
			GLParam(GL_DEPTH_TEST, TRUE),
			GLParam(GL_CULL_FACE, FALSE),
			GLParam(GL_VERTEX_ARRAY, TRUE),
			GLParam(GL_COLOR_ARRAY, TRUE),
			//GLParam(GL_BLEND_SRC, GL_SRC_ALPHA, ApplyBlend),
			//GLParam(GL_BLEND_DST, GL_ONE_MINUS_SRC_ALPHA, ApplyBlend),
			GLParam(GL_BLEND_SRC, GL_SRC_COLOR),
			GLParam(GL_BLEND_DST, GL_ONE),
			GLParam(GL_SHADE_MODEL, GL_SMOOTH),
		};	
		glShadeModel(GL_SMOOTH);

		glVertexPointer( 2, GL_FLOAT, sizeof(Vertex), &vertexBuffer[0].pos );
		glColorPointer(4, GL_FLOAT, sizeof(Vertex), &vertexBuffer[0].color );
		glDrawArrays(GL_TRIANGLE_STRIP,0, primitivesCount);	
	}
}