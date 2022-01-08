#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) ((x)+(y)*(width))*3+(c)
#define MAT_INDEX(x,y,width) ((x)+(y)*(width))
#define INDEX_ZB(width,x,y) ((x)+(y)*(width))
#define DEFAULT_DIMS 512

#define det(p2, p0, p1)  0.5*((p0.x-p2.x)*(p1.y-p2.y)-(p1.x-p2.x)*(p0.y-p2.y))
#define area(p1, p2, p3) abs(0.5*(p1.x*(p2.y-p3.y)+p2.x*(p3.y-p1.y)+p3.x*(p1.y-p2.y)))

#define viewPort(a) vec3((m_width / 2.0) * (a.x + 1), (m_height / 2.0) * (a.y + 1), a.z)

#define OUT_BUFFER 0 
#define Z_BUFFER 1 
#define BLUR_BUFFER 2

#define MAX_NUM_OF_LIGHTS 6

Renderer::Renderer() :m_width(DEFAULT_DIMS), m_height(DEFAULT_DIMS)
{
	InitOpenGLRendering();
	CreateBuffers(DEFAULT_DIMS, DEFAULT_DIMS);
	Init();
}
Renderer::Renderer(int width, int height) :m_width(width), m_height(height)
{
	InitOpenGLRendering();
	CreateBuffers(width,height);
	Init();

	// temp:
	
}

void Renderer::Init()
{
	orthogonal = false;
	fogMode = false;
	SSAA = false;
	lightBloom = false;

	fogMaxdist = 1.0;
	fogMindist = -3.0;
	fogColor = vec3(0.1, 0.1, 0.3);

	shadingSetup = Flat;

	// calculate kernel based on fixed sigma and size
	int size = 15;
	float sigma = 10;
	gaussianKernel = new float[size];
	
	float mult = 1 / sqrtf(2 * 3.1415326 * sigma * sigma);
	for (int i = 0; i < size; i++)
	{
		float x = (float)(i - size / 2);
		gaussianKernel[i] = mult * expf((-(x * x)) / (2 * sigma * sigma));
	}
		
	glViewport(0, 0, m_width, m_height);
	// init shaders programs
	
	glProgramArray.line = InitShader("Shaders/line_vs.glsl", "Shaders/standart_color.glsl");
	glProgramArray.wireFrame = InitShader("Shaders/wireFrame_vs.glsl", "Shaders/standart_color.glsl");
	glProgramArray.flat_gouraud = InitShader("Shaders/flat_gouraud_vs.glsl", "Shaders/standart_color.glsl");
	glProgramArray.phong = InitShader("Shaders/phong_vs.glsl", "Shaders/phong_fs.glsl");
}

void Renderer::drawOriginAxis()
{
	// can refactor this to hardcoded to save memory
	vec4 vertices[6];
	vec4 colors[6];

	int i = 0;
	vertices[i++] = vec4(0.0, 0.0, 0.0, 1.0);
	vertices[i++] = vec4(1.0, 0.0, 0.0, 1.0);
	vertices[i++] = vec4(0.0, 0.0, 0.0, 1.0);
	vertices[i++] = vec4(0.0, 1.0, 0.0, 1.0);
	vertices[i++] = vec4(0.0, 0.0, 0.0, 1.0);
	vertices[i++] = vec4(0.0, 0.0, 1.0, 1.0);

	i = 0;
	colors[i++] = vec4(1.0, 0.0, 0.0, 1.0);
	colors[i++] = vec4(1.0, 0.0, 0.0, 1.0);
	colors[i++] = vec4(0.0, 1.0, 0.0, 1.0);
	colors[i++] = vec4(0.0, 1.0, 0.0, 1.0);
	colors[i++] = vec4(0.0, 0.0, 1.0, 1.0);
	colors[i++] = vec4(0.0, 0.0, 1.0, 1.0);

	mat4 transform = Proj * lookAt;

	glDrawLinesColors(vertices, colors, 6, transform, GL_LINES);
	
}

void Renderer::glDrawLinesColors(vec4* vertices, vec4* colors, int size, mat4 transform, GLuint lineMode)
{
	glUseProgram(glProgramArray.line);
	glUniformLocArray.worldModel = glGetUniformLocation(glProgramArray.line, "transform");

	// set transform
	glUniformMatrix4fv(glUniformLocArray.worldModel, 1, GL_TRUE, transform);

	// draw
	GLuint buffers[2];
	GLuint vao;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, buffers);

	GLuint vPositionLoc = glGetAttribLocation(glProgramArray.line, "vPosition");
	GLuint vColorLoc = glGetAttribLocation(glProgramArray.line, "vColor");

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * size, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vPositionLoc);
	glVertexAttribPointer(vPositionLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * size, colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vColorLoc);
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(vao);
	glLineWidth(1);
	glDrawArrays(lineMode, 0, size);
	glUseProgram(0);
	glDeleteVertexArrays(1, &vao);
}

void Renderer::glDrawLines(vec4* vertices, int size, vec4 color, mat4 transform, GLuint lineMode)
{
	glUseProgram(glProgramArray.line);
	glUniformLocArray.worldModel = glGetUniformLocation(glProgramArray.line, "transform");
	// set camera
	glUniformMatrix4fv(glUniformLocArray.worldModel, 1, GL_TRUE, transform);

	// draw
	GLuint buffers[2];
	GLuint vao;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, buffers);

	GLuint vPositionLoc = glGetAttribLocation(glProgramArray.line, "vPosition");
	GLuint vColorLoc = glGetAttribLocation(glProgramArray.line, "vColor");

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * size, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vPositionLoc);
	glVertexAttribPointer(vPositionLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) , color, GL_STATIC_DRAW);
	glVertexAttrib3fv(vColorLoc, color);
	//glVertexAttribPointer(vColorLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(vao);
	glLineWidth(1);
	glDrawArrays(lineMode, 0, size);
	glUseProgram(0);
	glDeleteVertexArrays(1, &vao);
}

Renderer::~Renderer(void)
{
	delete[] gaussianKernel;
	DestroyBuffers();
	
}

void Renderer::toggleFog() 
{ fogMode = !fogMode; }

vec3 Renderer::calculateFog(vec3 color, GLfloat zValue)
{
	GLfloat fogFactor = max(0.0f, min(1.0f, (fogMaxdist - zValue) / (fogMaxdist - fogMindist)));
	return color * fogFactor + fogColor * (1.0 - fogFactor);
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width=width;
	m_height=height;
	CreateOpenGLBuffer(); //Do not remove this line.
	//m_outBuffer = new float[3 * m_width * m_height];
	//m_zbuffer = new float[m_width * m_height];
	//m_blurBuffer = new float[3 * m_width * m_height];
}

void Renderer::DestroyBuffers()
{
	//delete[] m_outBuffer;
	//delete[] m_zbuffer;
	//delete[] m_blurBuffer;
}

void Renderer::clearBuffer()
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	/*float bgColor = 0.05f;
	vec3 color = fogMode ? fogColor : vec3(1.0f, 1.0f, 1.0f) * bgColor;
	vec3 additive = SSAA ? vec3(0.0003f, 0.0003f, 0.0003f) * 0.5f : vec3(0.0003f, 0.0003f, 0.0003f);
	if (fogMode)
		additive *= 0;
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			m_outBuffer[INDEX(m_width, x, y, 0)] = color.x;
			m_outBuffer[INDEX(m_width, x, y, 1)] = color.y;
			m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
			m_zbuffer[INDEX_ZB(m_width, x, y)] = 101;
		}
		color += additive;
	}
	if (lightBloom)
	{
		for (int y = 0; y < m_height; y++)
		{
			for (int x = 0; x < m_width; x++)
			{
				m_blurBuffer[INDEX(m_width, x, y, 0)] = 0.0;
				m_blurBuffer[INDEX(m_width, x, y, 1)] = 0.0;
				m_blurBuffer[INDEX(m_width, x, y, 2)] = 0.0;
			}
		}
	}

	// fancy background: (takes lot of time)
	//GLfloat bgColor[2] = { 0.050 , 0.215 };
	//color = vec3(1.0, 1.0, 1.0) * (bgColor[1] - bgColor[0]) * ((GLfloat)y / (GLfloat)m_height) + bgColor[0];
	*/
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

void Renderer::drawLine(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, const vec3& color)
{
	drawLine((int)x0, (int)y0, (int)x1, (int)y1, color);
}

void Renderer::drawLine(vec3 a, vec3 b, const vec3& color)
{
	glColor3f(color.x, color.y, color.z);
	glBegin(GL_LINES);
	{
		glVertex2f(a.x, a.y);
		glVertex2f(b.x, b.y);
	}
	glEnd();

	return;
	/*if (a.x > m_width + border || a.x < -border || b.x > m_width + border || b.x < -border || a.y > m_height + border || a.y < -border || b.y > m_height + border || b.y < -border)
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
			if (!(x < 0 || x >= m_width || y < 0 || y >= m_height))
			{
				m_outBuffer[INDEX(m_width, x, y, 0)] = color.x;
				m_outBuffer[INDEX(m_width, x, y, 1)] = color.y;
				m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
			}
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
			if (!(x < 0 || x >= m_width || y < 0 || y >= m_height))
			{
				m_outBuffer[INDEX(m_width, x, y, 0)] = color.x;
				m_outBuffer[INDEX(m_width, x, y, 1)] = color.y;
				m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
			}
		}
	}*/
}

void Renderer::drawLine(int x0, int y0, int x1, int y1, const vec3& color)
{
	drawLine(vec3(x0, y0, 0), vec3(x1, y1, 0), color);
}

void Renderer::DrawTriangles(const std::vector<vec3>& vertices, Material& mat, vector<vec4>& faceNormals)
{
	for (int i = 0; i < vertices.size(); i+=3)
	{
		//drawTriangleFlat(vertices[i + 0], vertices[i + 1], vertices[i + 2], color);
		//DrawTrianglePhong(vertices[i + 0], vertices[i + 1], vertices[i + 2], mat, faceNormals[i / 3]);

		drawTriangleWire(vertices[i + 0], vertices[i + 1], vertices[i + 2], mat.color);
	}
}

vec3 findCoeficients(vec3 point, vec3 p0, vec3 p1, vec3 p2)
{
	// find area using determinant
	GLfloat A = area(p0, p1, p2);
	GLfloat A0 = area(point, p1, p2);
	GLfloat A1 = area(point, p0, p2);
	GLfloat A2 = area(point, p0, p1);

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

// hsv, h:0->360, s:0->1, v:0->1 to rgb: 0->1 
vec3 hsav2rgb(vec3 hsv)
{
	static GLfloat max = 0;
	static GLfloat min = 360;

	float s = hsv.y;
	float v = hsv.z;
	float C = s * v;
	float X = C * (1 - abs(fmod(hsv.x / 60.0, 2.0) - 1.0));
	float m = v - C;
	float r, g, b;
	if (hsv.x >= 0 && hsv.x < 60) {
		r = C, g = X, b = 0;
	}
	else if (hsv.x >= 60 && hsv.x < 120) {
		r = X, g = C, b = 0;
	}
	else if (hsv.x >= 120 && hsv.x < 180) {
		r = 0, g = C, b = X;
	}
	else if (hsv.x >= 180 && hsv.x < 240) {
		r = 0, g = X, b = C;
	}
	else if (hsv.x >= 240 && hsv.x < 300) {
		r = X, g = 0, b = C;
	}
	else {
		r = C, g = 0, b = X;
	}
	int R = (r + m);
	int G = (g + m);
	int B = (b + m);
	return vec3(r, g, b);
}

void Renderer::DrawModel(vec3* vertexPositions, vec3* faceNormals, vec3* vertexNormals, int size, Material mat, mat4 worldModel, mat4 normalMat)
{
	vec3 Wirecolor = vec3(1.0, 0.5, 0.7);

	GLuint currentShading;
	switch (shadingSetup)
	{
	case WireFrame:
		currentShading = glProgramArray.wireFrame;
		break;
	case Flat:
	case Gouraud:
		currentShading = glProgramArray.flat_gouraud;
		break;
	case Phong:
		currentShading = glProgramArray.phong;
		break;
	}

	glUseProgram(currentShading);

	// make uniform: camtransform, projection
	glUniformLocArray.lookAt = glGetUniformLocation(currentShading, "lookAt");
	glUniformLocArray.projection = glGetUniformLocation(currentShading, "proj");
	glUniformMatrix4fv(glUniformLocArray.lookAt, 1, GL_TRUE, lookAt);
	glUniformMatrix4fv(glUniformLocArray.projection, 1, GL_TRUE, Proj);

	// make uniform: model, world
	glUniformLocArray.worldModel = glGetUniformLocation(currentShading, "worldModelMat");
	glUniformMatrix4fv(glUniformLocArray.worldModel, 1, GL_TRUE, worldModel);

	// make uniform normal mat
	glUniformLocArray.normals = glGetUniformLocation(currentShading, "normalMat");
	glUniformMatrix4fv(glUniformLocArray.normals, 1, GL_TRUE, normalMat);

	// make uniform material
	glUniformLocArray.ambient = glGetUniformLocation(currentShading, "matAmbient");
	glUniformLocArray.diffuse = glGetUniformLocation(currentShading, "matDiffuse");
	glUniformLocArray.specular = glGetUniformLocation(currentShading, "matSpecular");
	glUniformLocArray.emissive = glGetUniformLocation(currentShading, "matEmissive");
	glUniformLocArray.shininess = glGetUniformLocation(currentShading, "matShininess");
	glUniform3fv(glUniformLocArray.ambient, 1, mat.ambientColor);
	glUniform3fv(glUniformLocArray.diffuse, 1, mat.diffuseColor);
	glUniform3fv(glUniformLocArray.specular, 1, mat.specularColor);
	glUniform3fv(glUniformLocArray.emissive, 1, mat.emissiveColor);
	glUniform1f(glUniformLocArray.shininess, mat.shininessCoeficient);

	// make uniform viewer
	glUniformLocArray.viewer = glGetUniformLocation(currentShading, "viewerPos");
	glUniform3fv(glUniformLocArray.viewer, 1, viewerPos[0]);

	// make uniform Lights
	vec3 pointLightPositions[MAX_NUM_OF_LIGHTS];
	vec3 pointLightColors[MAX_NUM_OF_LIGHTS];
	for (int i = 0; i < min(MAX_NUM_OF_LIGHTS, sceneLights->size()); i++)
	{
		pointLightPositions[i] = sceneLights->at(i)->position;
		pointLightColors[i] = sceneLights->at(i)->color;
	}

	for (int i = 0; i < min(MAX_NUM_OF_LIGHTS, sceneLights->size()); i++)
	{
		char variable[50];

		sprintf(variable, "pointLightPosition[%d]", i);
		GLuint pointlightPositionLoc = glGetUniformLocation(currentShading, variable);
		glUniform3fv(pointlightPositionLoc, 1, pointLightPositions[i]);

		sprintf(variable, "pointLightColor[%d]", i);
		GLuint pointlightColorLoc = glGetUniformLocation(currentShading, variable);
		glUniform3fv(pointlightColorLoc, 1, pointLightColors[i]);
	}
	GLuint pointLightCountLoc = glGetUniformLocation(currentShading, "pointLightCount");
	glUniform1i(pointLightCountLoc, min(6, sceneLights->size()));

	// make input
	GLuint buffers[2];
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, buffers);

	// bind first buffer: vertices
	GLuint vPositionLoc = glGetAttribLocation(currentShading, "vPosition");
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * size, vertexPositions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vPositionLoc);
	glVertexAttribPointer(vPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// bind second buffer
	GLuint drawingMode = GL_FILL;
	if (shadingSetup == WireFrame)
	{
		drawingMode = GL_LINE;
		GLuint vColorLoc = glGetAttribLocation(currentShading, "vColor");
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3), Wirecolor, GL_STATIC_DRAW);
		glVertexAttrib3fv(vColorLoc, Wirecolor);
	}
	else if (shadingSetup == Flat || shadingSetup == Gouraud || shadingSetup == Phong)
	{
		GLuint vNormalLoc = glGetAttribLocation(currentShading, "vNormal");
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		if (shadingSetup == Flat)
			glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * size, faceNormals, GL_STATIC_DRAW);
		if (shadingSetup == Gouraud || shadingSetup == Phong)
			glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * size, vertexNormals, GL_STATIC_DRAW);
		glEnableVertexAttribArray(vNormalLoc);
		glVertexAttribPointer(vNormalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	// draw
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(vao);
	glPolygonMode(GL_FRONT_AND_BACK, drawingMode);
	glLineWidth(1);
	glDrawArrays(GL_TRIANGLES, 0, size);
	glUseProgram(0);
	glDeleteVertexArrays(1, &vao);
}

void Renderer::drawTrianglesWire(const std::vector<vec3>& vertices, Material& mat)
{
	for (int i = 0; i < vertices.size(); i += 3)
	{
		drawTriangleWire(vertices[i + 0], vertices[i + 1], vertices[i + 2], mat.color);
	}
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
	static const GLfloat points[3][4] = {
		{-0.1, -0.1f, 0.0f,1.0f},
		{0.1f, -0.1f, 0.0f,1.0f},
		{0.0f,  0.1f, 0.0f,1.0f}
	};


	/*
	//vec3 color = vec3(1.0, 0.0, 0.0);
	drawLine((int)pos.x, (int)pos.y - 2, (int)pos.x, (int)pos.y + 4, color);
	drawLine((int)pos.x + 1, (int)pos.y - 2, (int)pos.x + 1, (int)pos.y + 4, color);

	drawLine((int)pos.x - 2, (int)pos.y, (int)pos.x + 4, (int)pos.y, color);
	drawLine((int)pos.x - 2, (int)pos.y + 1, (int)pos.x + 4, (int)pos.y + 1, color);*/
}

void Renderer::drawLightIndicator(vec4 pos, vec3 color, vec4 direction)
{
	vec3 Pos(pos.x, pos.y, 0.0);
	vec3 Dir(direction.x, direction.y, 0.0);
	bool directional = !(direction.x == 0.0 && direction.y == 0.0 && direction.z == 0.0);
	GLfloat size = directional ? 7.5 : 15.0;
	GLfloat size2 = directional ? 5.3 : 10.6;

	mat4 transform = Proj * lookAt;

	vec4 vertex[2];
	vertex[0] = pos;
	vertex[1] = pos + vec4(1.0, 0.0,0.0,0.0);
	glDrawLines(vertex, 2, color, transform,GL_LINES);

	return;
	/*vec4 vertices[8];

	int i = 0;
	vertices[i++] = Pos + vec3(-size, 0.0, 0.0);
	vertices[i++] = Pos + vec3(size, 0.0, 0.0);
	vertices[i++] = Pos + vec3(0.0, -size, 0.0);
	vertices[i++] = Pos + vec3(0.0, size, 0.0);
	vertices[i++] = Pos + vec3(-size2, -size2, 0.0);
	vertices[i++] = Pos + vec3(size2, size2, 0.0);
	vertices[i++] = Pos + vec3(-size2, size2, 0.0);
	vertices[i++] = Pos + vec3(size2, -size2, 0.0);*/

	//glDrawLines(vertices, 8, color, GL_LINES);

	if (!(direction.x == 0.0 && direction.y == 0.0 && direction.z == 0.0))
	{
		drawLine(Pos , Dir, color);
		vec3 direction = normalize(Dir - Pos);
		drawLine(Dir, Dir - 7.5 * direction + 7.5 * vec3(-direction.y, direction.x, 0.0), color);
		drawLine(Dir, Dir - 7.5 * direction - 7.5 * vec3(-direction.y, direction.x, 0.0), color);
	}
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height , 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}

void Renderer::SwapBuffers()
{
	glFlush();
	glutSwapBuffers();

	/*int a = glGetError();
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
	a = glGetError();*/
}

///////////////////////////////////////////////////////////////////////////////

void Renderer::convolute(float* buffer, float* kernel, int kernelWidth, int kernelHeight)
{
	float* bufferResult = new float[m_width * m_height * 3];

	// gather kernel sum (may be faster if i have sum already)
	float kernelSum = 0.0;
	for (int i = 0; i < kernelHeight * kernelWidth; i++)
		kernelSum += kernel[i];

	vec3 totalPixel, color;
	int xval, yval, xToCheck, yToCheck;
	float kernelVal;

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			totalPixel = vec3(0.0, 0.0, 0.0);
			for (int kx = 0; kx < kernelWidth; kx++)
			{
				for (int ky = 0; ky < kernelHeight; ky++)
				{
					xval = kx - kernelWidth / 2;
					yval = ky - kernelHeight / 2;

					kernelVal = kernel[kx + ky * kernelWidth];

					xToCheck = x + xval; yToCheck = y + yval;
					if (xToCheck < 0 || xToCheck > m_width - 1 || yToCheck < 0 || yToCheck > m_height - 1)
					{
						color = vec3(0.0, 0.0, 0.0);
					}
					else
					{
						color.x = buffer[INDEX(m_width, xToCheck, yToCheck, 0)] * kernelVal;
						color.y = buffer[INDEX(m_width, xToCheck, yToCheck, 1)] * kernelVal;
						color.z = buffer[INDEX(m_width, xToCheck, yToCheck, 2)] * kernelVal;
					}
					totalPixel += color;
				}
			}
			totalPixel /= kernelSum;
			bufferResult[INDEX(m_width, x, y, 0)] = totalPixel.x;
			bufferResult[INDEX(m_width, x, y, 1)] = totalPixel.y;
			bufferResult[INDEX(m_width, x, y, 2)] = totalPixel.z;
		}
	}

	// swap buffer
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			buffer[INDEX(m_width, x, y, 0)] = bufferResult[INDEX(m_width, x, y, 0)];
			buffer[INDEX(m_width, x, y, 1)] = bufferResult[INDEX(m_width, x, y, 1)];
			buffer[INDEX(m_width, x, y, 2)] = bufferResult[INDEX(m_width, x, y, 2)];
		}
	}

	delete[] bufferResult;
}

void Renderer::postProccess()
{
	if (lightBloom)
	{
		// get the brightest colors
		float brightness;
		for (int y = 0; y < m_height; y++)
		{
			for (int x = 0; x < m_width; x++)
			{
				// calc brightness for pixel
				brightness = m_outBuffer[INDEX(m_width, x, y, 0)] * 0.2126;
				brightness += m_outBuffer[INDEX(m_width, x, y, 1)] * 0.7152;
				brightness += m_outBuffer[INDEX(m_width, x, y, 2)] * 0.0722;

				if (brightness > 0.7)
				{
					m_blurBuffer[INDEX(m_width, x, y, 0)] = m_outBuffer[INDEX(m_width, x, y, 0)];
					m_blurBuffer[INDEX(m_width, x, y, 1)] = m_outBuffer[INDEX(m_width, x, y, 1)];
					m_blurBuffer[INDEX(m_width, x, y, 2)] = m_outBuffer[INDEX(m_width, x, y, 2)];
				}
			}
		}

		// double pass gaussian blur
		convolute(m_blurBuffer, gaussianKernel, 15, 1);
		convolute(m_blurBuffer, gaussianKernel, 1, 15);

		// add to out buffer
		vec3 colorBlurred, color;
		for (int y = 0; y < m_height; y++)
		{
			for (int x = 0; x < m_width; x++)
			{
				colorBlurred.x = m_blurBuffer[INDEX(m_width, x, y, 0)];
				colorBlurred.y = m_blurBuffer[INDEX(m_width, x, y, 1)];
				colorBlurred.z = m_blurBuffer[INDEX(m_width, x, y, 2)];

				color.x = m_outBuffer[INDEX(m_width, x, y, 0)];
				color.y = m_outBuffer[INDEX(m_width, x, y, 1)];
				color.z = m_outBuffer[INDEX(m_width, x, y, 2)];

				m_outBuffer[INDEX(m_width, x, y, 0)] = min(1.0, color.x + colorBlurred.x);
				m_outBuffer[INDEX(m_width, x, y, 1)] = min(1.0, color.y + colorBlurred.y);
				m_outBuffer[INDEX(m_width, x, y, 2)] = min(1.0, color.z + colorBlurred.z);
			}
		}
	}


	if (SSAA)
	{
		vec3 color(0.0, 0.0, 0.0);
		float* smallerBuffer = new float[(m_width / 2) * (m_height / 2) * 3];
		for (int y = 0; y < m_height/2; y++)
		{
			for (int x = 0; x < m_width/2; x++)
			{
				color = vec3();

				color.x += m_outBuffer[INDEX(m_width, x * 2, y * 2, 0)];
				color.y += m_outBuffer[INDEX(m_width, x * 2, y * 2, 1)];
				color.z += m_outBuffer[INDEX(m_width, x * 2, y * 2, 2)];
				
				color.x += m_outBuffer[INDEX(m_width, x * 2 + 1, y * 2, 0)];
				color.y += m_outBuffer[INDEX(m_width, x * 2 + 1, y * 2, 1)];
				color.z += m_outBuffer[INDEX(m_width, x * 2 + 1, y * 2, 2)];
				
				color.x += m_outBuffer[INDEX(m_width, x * 2 + 1, y * 2 + 1, 0)];
				color.y += m_outBuffer[INDEX(m_width, x * 2 + 1, y * 2 + 1, 1)];
				color.z += m_outBuffer[INDEX(m_width, x * 2 + 1, y * 2 + 1, 2)];
				
				color.x += m_outBuffer[INDEX(m_width, x * 2, y * 2 + 1, 0)];
				color.y += m_outBuffer[INDEX(m_width, x * 2, y * 2 + 1, 1)];
				color.z += m_outBuffer[INDEX(m_width, x * 2, y * 2 + 1, 2)];

				color *= 0.25;

				smallerBuffer[INDEX(m_width / 2, x, y, 0)] = color.x;
				smallerBuffer[INDEX(m_width / 2, x, y, 1)] = color.y;
				smallerBuffer[INDEX(m_width / 2, x, y, 2)] = color.z;
			}
		}

		for (int y = 0; y < m_height / 2; y++)
		{
			for (int x = 0; x < m_width / 2; x++)
			{
				color.x = smallerBuffer[INDEX(m_width / 2, x, y, 0)];
				color.y = smallerBuffer[INDEX(m_width / 2, x, y, 1)];
				color.z = smallerBuffer[INDEX(m_width / 2, x, y, 2)];

				m_outBuffer[INDEX(m_width, x, y, 0)] = color.x;
				m_outBuffer[INDEX(m_width, x, y, 1)] = color.y;
				m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
			}
		}
		delete[] smallerBuffer;

	}

}