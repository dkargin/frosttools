#include "CVect.h"
#include <gl/gl.h>

// Installs specific OpenGL state in constructor and restores previous state in the destructor
class GLParam
{	
public:
	typedef GLint (*ParamWrapper) (GLint, GLint);
	GLParam(GLint param, GLint state = TRUE);
	~GLParam();
private:
	ParamWrapper GetWrapper(GLint param);

	GLint param;
	GLint state, oldState;
	ParamWrapper wrapper;
};

// Draw polyline using triangle strip
class CMeshLine
{
public:
	struct Color { Scalar r, g, b, a; };
	struct Vertex
	{
		vec2 pos;
		Color color;
	};
	Vector<vec2> line;
	
	Scalar lineWidth;

	CMeshLine();
	enum Type
	{
		TypeUp,
		TypeDown,
		TypeBoth,
	}type;
	
	// setup vertex buffer for max line length
	void InitBuffers(int maxLength);
	// update vertex buffer
	void UpdateLine(size_t size, const Color & start, const Color & end, float widthFactor);	
	// draw line
	void Draw();
private:
	Vector<Vertex> vertexBuffer;	// vertex buffer
	int primitivesCount;			// primitives stored
	size_t InsertVertex(size_t index, const vec2& pos, const vec2 & up, const Color & color);	// add items for new line segment
	Color Interpolate(const Color & start, const Color & end, size_t index, size_t size) const;	// interpolate color between line segments
	vec2 GetUpVector(size_t index, size_t size) const;											
};