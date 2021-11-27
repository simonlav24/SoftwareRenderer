#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) ((x)+(y)*(width))*3+(c)
#define INDEX_ZB(width,x,y) ((x)+(y)*(width))
#define DEFAULT_DIMS 512

#define det(p2, p0, p1)  0.5*((p0.x-p2.x)*(p1.y-p2.y)-(p1.x-p2.x)*(p0.y-p2.y))
#define area(p1, p2, p3) abs(0.5*(p1.x*(p2.y-p3.y)+p2.x*(p3.y-p1.y)+p3.x*(p1.y-p2.y)))

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

void Renderer::clearBuffer()
{
	GLfloat bgColor[2] = { 0.050 , 0.215 };
	GLfloat color;
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			color = (bgColor[1] - bgColor[0]) * ((GLfloat)y / (GLfloat)m_height) + bgColor[0];
			m_outBuffer[INDEX(m_width, x, y, 0)] = color; m_outBuffer[INDEX(m_width, x, y, 1)] = color; m_outBuffer[INDEX(m_width, x, y, 2)] = color;
			m_zbuffer[INDEX_ZB(m_width, x, y)] = 101;
		}
	}
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

void Renderer::drawLine(vec3 a, vec3 b, const vec3& color)
{
	if (a.x > m_width + border || a.x < -border || b.x > m_width + border || b.x < -border || a.y > m_height + border || a.y < -border || b.y > m_height + border || b.y < -border)
		return;
	vec3 start, end;
	bool steep;
	if (abs(b.y - a.y) < abs(b.x - a.x))
	{
		if (a.x < b.x)
		{
			start = a;
			end = b;
		}
		else
		{
			start = b;
			end = a;
		}
		steep = false;
	}
	else
	{
		if (a.y < b.y)
		{
			start = a;
			end = b;
		}
		else
		{
			start = b;
			end = a;
		}
		steep = true;
	}
	int dx = end.x - start.x;
	int dy = end.y - start.y;
	if (steep)
	{
		int xdir = 1;
		if (dx < 0)
		{
			xdir *= -1;
			dx *= -1;
		}
		int d = 2 * dx - dy;
		int x = start.x;
		for (int y = start.y; y < end.y; y++)
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
	else
	{
		int ydir = 1;
		if (dy < 0)
		{
			ydir *= -1;
			dy *= -1;
		}
		int d = 2 * dy - dx;
		int y = start.y;
		for (int x = start.x; x < end.x; x++)
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
}

void Renderer::drawLine(int x0, int y0, int x1, int y1, const vec3& color)
{
	drawLine(vec3(x0, y0, 0), vec3(x1, y1, 0), color);
}

void Renderer::DrawTriangles(const std::vector<vec3>& vertices, const int count, const vec3& color)
{
	for (int i = 0; i < count; i+=3)
	{
		drawTriangleFlat(vertices[i + 0], vertices[i + 1], vertices[i + 2], color);
		drawTriangleWire(vertices[i + 0], vertices[i + 1], vertices[i + 2], color);
	}
}

vec3 findCoeficients(vec3 point, vec3 p0, vec3 p1, vec3 p2)
{
	// find area using determinant
	GLfloat A = area(p0, p1, p2);
	GLfloat A0 = area(point, p1, p2);
	GLfloat A1 = area(point, p0, p2);
	GLfloat A2 = area(point, p0, p1);
	//GLfloat A2 = 1 - A0 - A1;
	//cout << A0 + A1 + A2 << " " << A << endl;

	return vec3(A0 / A, A1 / A, A2 / A);

}

float sdot(vec3 p1, vec3 p2, vec3 p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool isInside(vec3 pt, vec3 v1, vec3 v2, vec3 v3)
{
	float d1, d2, d3;
	bool has_neg, has_pos;

	d1 = sdot(pt, v1, v2);
	d2 = sdot(pt, v2, v3);
	d3 = sdot(pt, v3, v1);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

void Renderer::drawTriangleFlat(vec3 p0, vec3 p1, vec3 p2, const vec3& color)
{
	int yMax = max(p0.y, max(p1.y, p2.y));
	int yMin = min(p0.y, min(p1.y, p2.y));

	int xMax = max(p0.x, max(p1.x, p2.x));
	int xMin = min(p0.x, min(p1.x, p2.x));
	
	if (yMax >= m_height || yMin < 0 || xMax >= m_width || xMin < 0)
		return;

	vec3 c1(1.0, 0, 0);
	vec3 c2(0, 1.0, 0);
	vec3 c3(0, 0, 1.0);

	vec3 alpha;
	vec3 Color;

	GLfloat zValue;

	for (int y = yMin; y <= yMax; y++)
	{
		for (int x = xMin; x <= xMax; x++)
		{
			if (isInside(vec3(x, y, 0), p0, p1, p2))
			{
				alpha = findCoeficients(vec3(x, y, 0), p0, p1, p2);
				zValue = alpha.x * p0.z + alpha.y * p1.z + alpha.z * p2.z;
				//if(rand() > RAND_MAX - 100)
				//	cout << zValue << " " << m_zbuffer[INDEX_ZB(m_width, x, y)] << endl;
				vec3 Color = alpha.x * c1 + alpha.y * c2 + alpha.z * c3;
				if (zValue < m_zbuffer[INDEX_ZB(m_width, x, y)])
				{
					m_zbuffer[INDEX_ZB(m_width, x, y)] = zValue;
					drawPixel(x, y, Color);
				}

			}
		}
	}
}



GLfloat lineIntersection(vec3 p1, vec3 p2, vec3 p3, vec3 p4)
{
	GLfloat Denom = (p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x);
	GLfloat x = (p1.x * p2.y - p1.y * p2.x) * (p3.x - p4.x) - (p1.x - p2.x) * (p3.x * p4.y - p3.y * p4.x);
	return x / Denom;
	//GLfloat y = (p1.x * p2.y - p1.y * p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x * p4.y - p3.y * p4.x);
	//return vec3(x / Denom, y / Denom, 0);
}

void Renderer::drawTriangleWire(vec3 p0, vec3 p1, vec3 p2, const vec3& color)
{
	drawLine(p0, p1, color);
	drawLine(p1, p2, color);
	drawLine(p2, p0, color);
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

void Renderer::drawLightIndicator(vec4 pos, vec3 color)
{
	GLfloat size = 15.0;
	GLfloat size2 = 10.6;
	vec3 Pos(pos.x, pos.y, pos.z);
	drawLine(Pos + vec3(-size, 0.0, 0.0), Pos + vec3(size, 0.0, 0.0), color);
	drawLine(Pos + vec3(0.0, -size, 0.0), Pos + vec3(0.0, size, 0.0), color);
	drawLine(Pos + vec3(-size2, -size2, 0.0), Pos + vec3(size2, size2, 0.0), color);
	drawLine(Pos + vec3(-size2, size2, 0.0), Pos + vec3(size2, -size2, 0.0), color);
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