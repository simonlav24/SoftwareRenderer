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

	shadingSetup = Phong;

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
	
	glProgramArray.line = InitShader("Shaders/line_vs.glsl", "Shaders/basic_fs.glsl");
	
}

void Renderer::drawOriginAxis()
{
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

	glDrawLinesColors(vertices, colors, 6);
	
}

void Renderer::glDrawLinesColors(vec4* vertices, vec4* colors, int size)
{
	glUseProgram(glProgramArray.line);
	glUniformLocArray.lookAt = glGetUniformLocation(glProgramArray.line, "lookAt");
	glUniformLocArray.projection = glGetUniformLocation(glProgramArray.line, "proj");

	// set camera
	glUniformMatrix4fv(glUniformLocArray.lookAt, 1, GL_TRUE, lookAt);
	glUniformMatrix4fv(glUniformLocArray.projection, 1, GL_TRUE, Proj);

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

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(vao);
	glLineWidth(1);
	glDrawArrays(GL_LINE_STRIP, 0, size);
	glUseProgram(0);
	glDeleteVertexArrays(1, &vao);

	//GLenum errCode;
	//if ((errCode = glGetError()) != GL_NO_ERROR)
	//	cout << "OpenGl error! - " << gluErrorString(errCode) << endl;;
}

void Renderer::glDrawLines(vec4* vertices, int size, vec4 color)
{
	glUseProgram(glProgramArray.line);
	glUniformLocArray.lookAt = glGetUniformLocation(glProgramArray.line, "lookAt");
	glUniformLocArray.projection = glGetUniformLocation(glProgramArray.line, "proj");

	// set camera
	glUniformMatrix4fv(glUniformLocArray.lookAt, 1, GL_TRUE, lookAt);
	glUniformMatrix4fv(glUniformLocArray.projection, 1, GL_TRUE, Proj);

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
	glDrawArrays(GL_LINE_STRIP, 0, size);
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
	m_outBuffer = new float[3 * m_width * m_height];
	m_zbuffer = new float[m_width * m_height];
	m_blurBuffer = new float[3 * m_width * m_height];
}

void Renderer::DestroyBuffers()
{
	delete[] m_outBuffer;
	delete[] m_zbuffer;
	delete[] m_blurBuffer;
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

vec3 Renderer::calculateAmbient(Material& mat)
{
	vec3 Iambient(0.0, 0.0, 0.0);
	for (int l = 0; l < sceneLights->size(); l++)
	{
		Iambient.x += sceneLights->at(l)->color.x * max(0.0, mat.ambientColor.x * sceneLights->at(l)->ambientIntensity);
		Iambient.y += sceneLights->at(l)->color.y * max(0.0, mat.ambientColor.y * sceneLights->at(l)->ambientIntensity);
		Iambient.z += sceneLights->at(l)->color.z * max(0.0, mat.ambientColor.z * sceneLights->at(l)->ambientIntensity);
	}
	return Iambient;
}

vec3 Renderer::calculateDiffusion(vec3& pointInWorld, vec3& normalInWorld, Material& mat)
{
	vec3 Idiffuse(0.0, 0.0, 0.0);
	vec3 dirToLight;
	if (!mat.special)
	{
		for (int l = 0; l < sceneLights->size(); l++)
		{
			if (sceneLights->at(l)->lightType == ambience)
				continue;
			else if (sceneLights->at(l)->lightType == point)
				dirToLight = sceneLights->at(l)->position - pointInWorld;
			else if (sceneLights->at(l)->lightType == parallel)
				dirToLight = -sceneLights->at(l)->direction;
			GLfloat dotProd = dot(normalize(normalInWorld), normalize(dirToLight));
			Idiffuse.x += sceneLights->at(l)->color.x * max(0.0, mat.diffuseColor.x * dotProd * sceneLights->at(l)->diffuseIntensity);
			Idiffuse.y += sceneLights->at(l)->color.y * max(0.0, mat.diffuseColor.y * dotProd * sceneLights->at(l)->diffuseIntensity);
			Idiffuse.z += sceneLights->at(l)->color.z * max(0.0, mat.diffuseColor.z * dotProd * sceneLights->at(l)->diffuseIntensity);
		}
	}
	else
	{
		for (int l = 0; l < sceneLights->size(); l++)
		{
			if (sceneLights->at(l)->lightType == ambience)
				continue;
			else if (sceneLights->at(l)->lightType == point)
				dirToLight = sceneLights->at(l)->position - pointInWorld;
			else if (sceneLights->at(l)->lightType == parallel)
				dirToLight = -sceneLights->at(l)->direction;
			GLfloat dotProd = dot(normalize(normalInWorld), normalize(dirToLight));
			vec3 dirToViewer = vec3(viewerPos[0].x, viewerPos[0].y, viewerPos[0].z) - pointInWorld;
			GLfloat dotProd2 = dot(normalize(normalInWorld), normalize(dirToViewer));
			dotProd2 += dotProd;
			vec3 difColor = vec3(0.5*sin(0.5*pointInWorld.x + 5* dotProd)+0.5, 0.5*sin(0.5*pointInWorld.y + 5* dotProd)+0.5, 0.5*sin(0.5*pointInWorld.z + 5* dotProd)+0.5);

			Idiffuse.x += sceneLights->at(l)->color.x * max(0.0, mat.diffuseColor.x * difColor.x * dotProd * sceneLights->at(l)->diffuseIntensity);
			Idiffuse.y += sceneLights->at(l)->color.y * max(0.0, mat.diffuseColor.y * difColor.y * dotProd * sceneLights->at(l)->diffuseIntensity);
			Idiffuse.z += sceneLights->at(l)->color.z * max(0.0, mat.diffuseColor.z * difColor.z * dotProd * sceneLights->at(l)->diffuseIntensity);
		}
	}
	return Idiffuse;
}

vec3 Renderer::calculateSpecular(vec3& pointInWorld, vec3& normalInWorld, Material& mat)
{
	vec3 Ispecular(0.0, 0.0, 0.0);
	vec3 dirFromLight, reflected;
	for (int l = 0; l < sceneLights->size(); l++)
	{
		normalInWorld = normalize(normalInWorld);
		if (sceneLights->at(l)->lightType == point)
			dirFromLight = pointInWorld - sceneLights->at(l)->position;
		else if (sceneLights->at(l)->lightType == parallel)
			dirFromLight = sceneLights->at(l)->direction;

		reflected = dirFromLight - 2.0 * dot(dirFromLight, normalInWorld) * normalInWorld;
		vec3 dirToViewer = vec3(viewerPos[0].x, viewerPos[0].y, viewerPos[0].z) - pointInWorld;
		GLfloat dotProd = dot(normalize(reflected), normalize(dirToViewer));
		//dotProd = sin(4.0 * 3.1415326 * dotProd);

		if (dotProd <= 0)
			continue;
		dotProd = pow(dotProd, mat.shininessCoeficient);
		
		Ispecular.x += sceneLights->at(l)->color.x * max(0.0, mat.specularColor.x * dotProd * sceneLights->at(l)->specularIntensity);
		Ispecular.y += sceneLights->at(l)->color.y * max(0.0, mat.specularColor.y * dotProd * sceneLights->at(l)->specularIntensity);
		Ispecular.z += sceneLights->at(l)->color.z * max(0.0, mat.specularColor.z * dotProd * sceneLights->at(l)->specularIntensity);
	}

	return Ispecular;
}

void Renderer::drawModel(vector<vec4>& modelVertices, vector<vec4>& modelFaceNormals, vector<vec4>& modelVertexNormals, Material& mat)
{
	if (shadingSetup == WireFrame)
	{
		vector<vec3> triangles;
		for (int i = 0; i < modelVertices.size(); i++)
		{
			vec3 screenPoint = viewPort(homo2noHomo(ProjCam * modelVertices[i]));
			triangles.push_back(screenPoint);
		}
		drawTrianglesWire(triangles, mat);
		return;
	}

	else if (shadingSetup == Flat)
	{
		for (int i = 0; i < modelVertices.size(); i++)
		{
			vec4 p0 = modelVertices[i];
			vec4 p1 = modelVertices[i + 1];
			vec4 p2 = modelVertices[i + 2];

			// calculate screen points
			vec3 sp0 = viewPort(homo2noHomo(ProjCam * p0));
			vec3 sp1 = viewPort(homo2noHomo(ProjCam * p1));
			vec3 sp2 = viewPort(homo2noHomo(ProjCam * p2));

			vec3 np0 = homo2noHomo(p0);
			vec3 np1 = homo2noHomo(p1);
			vec3 np2 = homo2noHomo(p2);

			int yMax = max(sp0.y, max(sp1.y, sp2.y));
			int yMin = min(sp0.y, min(sp1.y, sp2.y));

			int xMax = max(sp0.x, max(sp1.x, sp2.x));
			int xMin = min(sp0.x, min(sp1.x, sp2.x));

			if (yMax >= m_height + border || yMin < 0 - border || xMax >= m_width + border || xMin < 0 - border)
			{
				i += 2;
				continue;
			}

			GLfloat zValue;
			vec3 alpha;
			vec3 pointInWorld = vec3(np0 + np1 + np2) / 3.0;
			vec3 normalInWorld = vec3(modelFaceNormals[i].x, modelFaceNormals[i].y, modelFaceNormals[i].z);
			vec3 Color;

			vec3 Iambient = calculateAmbient(mat);
			vec3 Idiffuse = calculateDiffusion(pointInWorld, normalInWorld, mat);
			vec3 Ispecular = calculateSpecular(pointInWorld, normalInWorld, mat);
			
			vec3 Itot = Idiffuse + Idiffuse + Ispecular;
			Color = vec3(mat.color.x * Itot.x, mat.color.y * Itot.y, mat.color.z * Itot.z) + mat.emissiveColor;

			for (int y = yMin; y <= yMax; y++)
			{
				for (int x = xMin; x <= xMax; x++)
				{
					if (x < 0 || x >= m_width || y < 0 || y >= m_height)
						continue;
					if (isInside(vec3(x, y, 0), sp0, sp1, sp2))
					{
						alpha = findCoeficients(vec3(x, y, 0), sp0, sp1, sp2);
						zValue = alpha.x * sp0.z + alpha.y * sp1.z + alpha.z * sp2.z;
						if (zValue < m_zbuffer[INDEX_ZB(m_width, x, y)])
						{
							m_zbuffer[INDEX_ZB(m_width, x, y)] = zValue;
							if (fogMode) Color = calculateFog(Color, zValue);

							m_outBuffer[INDEX(m_width, x, y, 0)] = Color.x;
							m_outBuffer[INDEX(m_width, x, y, 1)] = Color.y;
							m_outBuffer[INDEX(m_width, x, y, 2)] = Color.z;
							
						}

					}
				}
			}
			i += 2;
		}
	}

	else if (shadingSetup == Gouraud)
	{
		vec3 eyeInWorld = homo2noHomo(viewerPos[0]);
		GLfloat Iambient = 0.0;
		GLfloat Idiffuse = 0.0;
		GLfloat Ispeculat = 0.0;
		GLfloat Itot = 0.0;

		for (int i = 0; i < modelVertices.size(); i++)
		{
			vec4 p0 = modelVertices[i];
			vec4 p1 = modelVertices[i + 1];
			vec4 p2 = modelVertices[i + 2];

			// calculate screen points
			vec3 sp0 = viewPort(homo2noHomo(ProjCam * p0));
			vec3 sp1 = viewPort(homo2noHomo(ProjCam * p1));
			vec3 sp2 = viewPort(homo2noHomo(ProjCam * p2));

			vec3 np0 = homo2noHomo(p0);
			vec3 np1 = homo2noHomo(p1);
			vec3 np2 = homo2noHomo(p2);

			int yMax = max(sp0.y, max(sp1.y, sp2.y));
			int yMin = min(sp0.y, min(sp1.y, sp2.y));

			int xMax = max(sp0.x, max(sp1.x, sp2.x));
			int xMin = min(sp0.x, min(sp1.x, sp2.x));

			if (yMax >= m_height + border || yMin < 0 - border || xMax >= m_width + border || xMin < 0 - border)
			{
				i += 2;
				continue;
			}

			GLfloat zValue;
			vec3 alpha;
			vec3 pointInWorld;
			vec3 normalInWorld;
			vec3 Color;

			vec3 Colors[3];
			// find colors of points around
			for (int c = 0; c < 3; c++)
			{
				pointInWorld = homo2noHomo(modelVertices[i + c]);
				normalInWorld = vec3(modelVertexNormals[i + c].x, modelVertexNormals[i + c].y, modelVertexNormals[i + c].z);
				vec3 Iambient = calculateAmbient(mat);
				vec3 Idiffuse = calculateDiffusion(pointInWorld, normalInWorld, mat);
				vec3 Ispecular = calculateSpecular(pointInWorld, normalInWorld, mat);

				vec3 Itot = Iambient + Idiffuse + Ispecular;
				Colors[c] = vec3(mat.color.x * Itot.x, mat.color.y * Itot.y, mat.color.z * Itot.z);
			}

			for (int y = yMin; y <= yMax; y++)
			{
				for (int x = xMin; x <= xMax; x++)
				{
					if (x < 0 || x >= m_width || y < 0 || y >= m_height)
						continue;
					if (isInside(vec3(x, y, 0), sp0, sp1, sp2))
					{
						alpha = findCoeficients(vec3(x, y, 0), sp0, sp1, sp2);
						zValue = alpha.x * sp0.z + alpha.y * sp1.z + alpha.z * sp2.z;

						Color = alpha.x * Colors[0] + alpha.y * Colors[1] + alpha.z * Colors[2] + mat.emissiveColor;

						if (zValue < m_zbuffer[INDEX_ZB(m_width, x, y)])
						{
							m_zbuffer[INDEX_ZB(m_width, x, y)] = zValue;
							if (fogMode) Color = calculateFog(Color, zValue);
							m_outBuffer[INDEX(m_width, x, y, 0)] = Color.x;
							m_outBuffer[INDEX(m_width, x, y, 1)] = Color.y;
							m_outBuffer[INDEX(m_width, x, y, 2)] = Color.z;
						}

					}
				}
			}
			i += 2;
		}
	}

	else if (shadingSetup == Phong)
	{
		vec3 eyeInWorld = homo2noHomo(viewerPos[0]);
		GLfloat Itot = 0.0;

		for (int i = 0; i < modelVertices.size(); i++)
		{
			vec4 p0 = modelVertices[i];
			vec4 p1 = modelVertices[i + 1];
			vec4 p2 = modelVertices[i + 2];

			// calculate screen points
			vec3 sp0 = viewPort(homo2noHomo(ProjCam * p0));
			vec3 sp1 = viewPort(homo2noHomo(ProjCam * p1));
			vec3 sp2 = viewPort(homo2noHomo(ProjCam * p2));

			vec3 np0 = homo2noHomo(p0);
			vec3 np1 = homo2noHomo(p1);
			vec3 np2 = homo2noHomo(p2);

			int yMax = max(sp0.y, max(sp1.y, sp2.y));
			int yMin = min(sp0.y, min(sp1.y, sp2.y));

			int xMax = max(sp0.x, max(sp1.x, sp2.x));
			int xMin = min(sp0.x, min(sp1.x, sp2.x));

			if (yMax >= m_height + border || yMin < 0 - border || xMax >= m_width + border || xMin < 0 - border)
			{
				i += 2;
				continue;
			}

			GLfloat zValue;
			vec3 alpha;
			vec3 pointInWorld;
			vec3 normalInWorld;
			vec3 Color;

			vec3 normals[3];
			for (int n = 0; n < 3; n++)
			{
				normals[n] = vec3(modelVertexNormals[i + n].x, modelVertexNormals[i + n].y, modelVertexNormals[i + n].z);
			}

			for (int y = yMin; y <= yMax; y++)
			{
				for (int x = xMin; x <= xMax; x++)
				{
					if (x < 0 || x >= m_width || y < 0 || y >= m_height)
						continue;
					if (isInside(vec3(x, y, 0), sp0, sp1, sp2))
					{
						alpha = findCoeficients(vec3(x, y, 0), sp0, sp1, sp2);
						zValue = alpha.x * sp0.z + alpha.y * sp1.z + alpha.z * sp2.z;

						pointInWorld = alpha.x * np0 + alpha.y * np1 + alpha.z * np2;
						normalInWorld = alpha.x * normals[0] + alpha.y * normals[1] + alpha.z * normals[2];

						vec3 Iambient = calculateAmbient(mat);
						vec3 Idiffuse = calculateDiffusion(pointInWorld, normalInWorld, mat);
						vec3 Ispecular = calculateSpecular(pointInWorld, normalInWorld, mat);

						vec3 Itot = Iambient + Idiffuse + Ispecular;
						
						Color = vec3(mat.color.x * Itot.x, mat.color.y * Itot.y, mat.color.z * Itot.z) + mat.emissiveColor;

						if (zValue < m_zbuffer[INDEX_ZB(m_width, x, y)])
						{
							m_zbuffer[INDEX_ZB(m_width, x, y)] = zValue;
							if (fogMode) Color = calculateFog(Color, zValue);
							m_outBuffer[INDEX(m_width, x, y, 0)] = Color.x;
							m_outBuffer[INDEX(m_width, x, y, 1)] = Color.y;
							m_outBuffer[INDEX(m_width, x, y, 2)] = Color.z;
						}

					}
				}
			}
			i += 2;
		}
	}
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
	drawLine(Pos + vec3(-size, 0.0, 0.0), Pos + vec3(size, 0.0, 0.0), color);
	drawLine(Pos + vec3(0.0, -size, 0.0), Pos + vec3(0.0, size, 0.0), color);
	drawLine(Pos + vec3(-size2, -size2, 0.0), Pos + vec3(size2, size2, 0.0), color);
	drawLine(Pos + vec3(-size2, size2, 0.0), Pos + vec3(size2, -size2, 0.0), color);
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