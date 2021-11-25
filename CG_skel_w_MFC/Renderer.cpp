#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c
#define INDEX_ZB(width,x,y) (x+y*width)
#define DEFAULT_DIMS 512

Renderer::Renderer() :m_width(DEFAULT_DIMS), m_height(DEFAULT_DIMS)
{
	InitOpenGLRendering();
	CreateBuffers(DEFAULT_DIMS, DEFAULT_DIMS);
}
Renderer::Renderer(int width, int height) :m_width(width), m_height(height)
{
	InitOpenGLRendering();
	CreateBuffers(width,height);
}

Renderer::~Renderer(void)
{
	DestroyBuffers();
}



void Renderer::CreateBuffers(int width, int height)
{
	m_width=width;
	m_height=height;	
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3*m_width*m_height];
	m_zbuffer = new float[m_width * m_height];
}

void Renderer::DestroyBuffers()
{
	delete[] m_outBuffer;
	delete[] m_zbuffer;
}

void Renderer::SetDemoBuffer()
{
	//vertical line
	for(int i=0; i<m_width/3; i++)
	{
		m_outBuffer[INDEX(m_width,256,i,0)]=1;	m_outBuffer[INDEX(m_width,256,i,1)]=0;	m_outBuffer[INDEX(m_width,256,i,2)]=0;

	}
	//horizontal line
	for(int i=0; i<m_width/3; i++)
	{
		m_outBuffer[INDEX(m_width,i,256,0)]=1;	m_outBuffer[INDEX(m_width,i,256,1)]=0;	m_outBuffer[INDEX(m_width,i,256,2)]=1;

	}
}

// method for changing the color of a single pixel
void Renderer::drawPixel(int x, int y, const vec3& color)
{
	if (x < 0 || x > m_width - 1 || y < 0 || y > m_height - 1)
		return;
	m_outBuffer[INDEX(m_width, x, y, 0)] = color.x; m_outBuffer[INDEX(m_width, x, y, 1)] = color.y; m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
}

void Renderer::drawLine(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, const vec3& color)
{
	drawLine((int)x0, (int)y0, (int)x1, (int)y1, color);
}

void Renderer::drawLine(int x0, int y0, int x1, int y1, const vec3& color)
{
	if (x0 > m_width + border || x0 < -border || x1 > m_width + border || x1 < -border || y0 > m_height + border || y0 < -border || y1 > m_height + border || y1 < -border)
		return;
	if (abs(y1 - y0) < abs(x1 - x0))
	{
		if (x0 < x1)
			lineflat(x0, y0, x1, y1, color);
		else
			lineflat(x1, y1, x0, y0, color);
	}
	else
	{
		if (y0 < y1)
			lineSteep(x0, y0, x1, y1, color);
		else
			lineSteep(x1, y1, x0, y0, color);
	}
}

void  Renderer::lineSteep(int x0, int y0, int x1, int y1, const vec3& color)
{
	///forward moves x
	int dx = x1 - x0;
	int dy = y1 - y0;
	int xdir = 1;
	if (dx < 0)
	{
		xdir *= -1;
		dx *= -1;
	}
	int d = 2 * dx - dy;
	int x = x0;
	for (int y = y0; y < y1; y++)
	{
		if (d > 0)
		{
			x += xdir;
			d += 2 * (dx - dy);
		}
		else
		{
			d += 2 * dx;
		}
		drawPixel(x, y, color);
	}

}
void  Renderer::lineflat(int x0, int y0, int x1, int y1, const vec3& color)
{
	///forward moves y
	int dx = x1 - x0;
	int dy = y1 - y0;
	int ydir = 1;
	if (dy < 0)
	{
		ydir *= -1;
		dy *= -1;
	}
	int d = 2 * dy - dx;
	int y = y0;
	for (int x = x0; x < x1; x++)
	{
		if (d > 0)
		{
			y += ydir;
			d += 2 * (dy - dx);
		}
		else
		{
			d += 2 * dy;
		}
		drawPixel(x, y, color);
	}
}

void Renderer::DrawTriangles(const std::vector<vec3>& vertices, const int count, const vec3& color)
{
	for (int i = 0; i < count; i+=3)
	{
		drawTriangle(vertices[i + 0], vertices[i + 1], vertices[i + 2], color);
	}
}

void Renderer::drawTriangleFlat(vec3 p0, vec3 p1, vec3 p2, const vec3& color)
{
	// find max y
}

void Renderer::drawTriangle(vec3 p0, vec3 p1, vec3 p2, const vec3& color)
{
	drawLine(p0.x, p0.y, p1.x, p1.y, color);
	drawLine(p1.x, p1.y, p2.x, p2.y, color);
	drawLine(p2.x, p2.y, p0.x, p0.y, color);
}

void Renderer::clearBuffer()
{
	GLfloat bgColor[2] = { 0.050 , 0.215};
	GLfloat color;
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			color = (bgColor[1] - bgColor[0]) * ((GLfloat)y / (GLfloat)m_height) + bgColor[0];
			m_outBuffer[INDEX(m_width, x, y, 0)] = color; m_outBuffer[INDEX(m_width, x, y, 1)] = color; m_outBuffer[INDEX(m_width, x, y, 2)] = color;
		}
	}
}

vec2 Renderer::getDims()
{
	return vec2(m_width, m_height);
}

void Renderer::drawPlusSign(vec4 pos, vec3 color)
{
	//vec3 color = vec3(1.0, 0.0, 0.0);
	drawLine((int)pos.x, (int)pos.y - 2, (int)pos.x, (int)pos.y + 4, color);
	drawLine((int)pos.x + 1, (int)pos.y - 2, (int)pos.x + 1, (int)pos.y + 4, color);

	drawLine((int)pos.x - 2, (int)pos.y, (int)pos.x + 4, (int)pos.y, color);
	drawLine((int)pos.x - 2, (int)pos.y + 1, (int)pos.x + 4, (int)pos.y + 1, color);
}

void Renderer::reshape(int width, int height)
{
	DestroyBuffers();
	CreateBuffers(width, height);
}

/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[]={
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[]={
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1};
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc)+sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );
	GLint  vPosition = glGetAttribLocation( program, "vPosition" );

	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0 );

	GLint  vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid *) sizeof(vtc) );
	glProgramUniform1i( program, glGetUniformLocation(program, "texture"), 0 );
	a = glGetError();
}

void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}

void Renderer::SwapBuffers()
{

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glutSwapBuffers();
	a = glGetError();
}