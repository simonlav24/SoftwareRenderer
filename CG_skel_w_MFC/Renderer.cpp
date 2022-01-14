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
	//InitOpenGLRendering();
	//CreateBuffers(DEFAULT_DIMS, DEFAULT_DIMS);
	Init();
}
Renderer::Renderer(int width, int height) :m_width(width), m_height(height)
{
	//InitOpenGLRendering();
	//CreateBuffers(width,height);
	Init();
}

void Renderer::Init()
{
	orthogonal = false;

	shadingSetup = Phong;
	
	// init shaders programs
	glProgramArray.line = InitShader("Shaders/line_vs.glsl", "Shaders/standart_color.glsl");
	glProgramArray.wireFrame = InitShader("Shaders/wireFrame_vs.glsl", "Shaders/standart_color.glsl");
	glProgramArray.flat_gouraud = InitShader("Shaders/flat_gouraud_vs.glsl", "Shaders/flat_gouraud_fs.glsl");
	glProgramArray.phong = InitShader("Shaders/phong_vs.glsl", "Shaders/phong_fs.glsl");
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

	mat4 transform = Proj * lookAt;

	glDrawLines(vertices, colors, 6, transform, GL_LINES);
	
}

void Renderer::glDrawLines(vec4* vertices, vec4* colors, int size, mat4 transform, GLuint lineMode, bool singleColor)
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
	if (singleColor)
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec4), colors, GL_STATIC_DRAW);
		glVertexAttrib3fv(vColorLoc, *colors);
	}
	else
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * size, colors, GL_STATIC_DRAW);
		glEnableVertexAttribArray(vColorLoc);
		glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}
	

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(vao);
	glLineWidth(1);
	glDrawArrays(lineMode, 0, size);
	glUseProgram(0);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(2, buffers);
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
}

void Renderer::DestroyBuffers()
{
}

void Renderer::clearBuffer()
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

void Renderer::DrawModel(vaoData vData, Material mat, mat4 worldModel, mat4 normalMat)
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
	glUniform3fv(glUniformLocArray.ambient, 1, mat.ambientColor);
	glUniformLocArray.diffuse = glGetUniformLocation(currentShading, "matDiffuse");
	glUniform3fv(glUniformLocArray.diffuse, 1, mat.diffuseColor);
	glUniformLocArray.specular = glGetUniformLocation(currentShading, "matSpecular");
	glUniform3fv(glUniformLocArray.specular, 1, mat.specularColor);
	glUniformLocArray.emissive = glGetUniformLocation(currentShading, "matEmissive");
	glUniform3fv(glUniformLocArray.emissive, 1, mat.emissiveColor);
	glUniformLocArray.shininess = glGetUniformLocation(currentShading, "matShininess");
	glUniform1f(glUniformLocArray.shininess, mat.shininessCoeficient);
	//glUniform1i(glGetUniformLocation(currentShading, "matEnvironment"), mat.environment);

	// make uniform color wireframe
	glUniform3fv(glGetUniformLocation(currentShading, "wireColor"), 1, Wirecolor);

	// make uniform viewer
	glUniformLocArray.viewer = glGetUniformLocation(currentShading, "viewerPos");
	glUniform3fv(glUniformLocArray.viewer, 1, viewerPos[0]);

	// make uniform Lights
	vec3 lightPositions[MAX_NUM_OF_LIGHTS];
	vec3 lightColors[MAX_NUM_OF_LIGHTS];
	GLint lightTypes[MAX_NUM_OF_LIGHTS];
	for (int i = 0; i < min(MAX_NUM_OF_LIGHTS, sceneLights->size()); i++)
	{
		if(sceneLights->at(i)->lightType == point)
			lightPositions[i] = sceneLights->at(i)->position;
		else if (sceneLights->at(i)->lightType == parallel)
			lightPositions[i] = sceneLights->at(i)->direction;
		lightColors[i] = sceneLights->at(i)->color;
		lightTypes[i] = (GLuint)sceneLights->at(i)->lightType;
	}

	for (int i = 0; i < min(MAX_NUM_OF_LIGHTS, sceneLights->size()); i++)
	{
		char variable[50];

		sprintf(variable, "lightPositions[%d]", i);
		GLuint lightPositionLoc = glGetUniformLocation(currentShading, variable);
		glUniform3fv(lightPositionLoc, 1, lightPositions[i]);

		sprintf(variable, "lightColors[%d]", i);
		GLuint lightColorLoc = glGetUniformLocation(currentShading, variable);
		glUniform3fv(lightColorLoc, 1, lightColors[i]);

		sprintf(variable, "lightTypes[%d]", i);
		GLuint lightTypeLoc = glGetUniformLocation(currentShading, variable);
		glUniform1iv(lightTypeLoc, 1, &lightTypes[i]);
	}
	GLuint lightCountLoc = glGetUniformLocation(currentShading, "lightCount");
	glUniform1i(lightCountLoc, min(6, sceneLights->size()));

	// bind data
	glBindVertexArray(vData.vao);

	// bind vertices buffer
	GLuint vPositionLoc = glGetAttribLocation(currentShading, "vPosition");
	glBindBuffer(GL_ARRAY_BUFFER, vData.buffers[0]);
	glEnableVertexAttribArray(vPositionLoc);
	glVertexAttribPointer(vPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// bind normal buffer
	GLuint drawingMode = GL_FILL;
	if (shadingSetup == WireFrame)
	{
		drawingMode = GL_LINE;
	}
	else if (shadingSetup == Flat || shadingSetup == Gouraud || shadingSetup == Phong)
	{
		GLuint vNormalLoc = glGetAttribLocation(currentShading, "vNormal");
		glBindBuffer(GL_ARRAY_BUFFER, vData.buffers[shadingSetup == Flat ? 1 : 2]);
		glEnableVertexAttribArray(vNormalLoc);
		glVertexAttribPointer(vNormalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	// make uniform isTexturized
	glUniform1i(glGetUniformLocation(currentShading, "isTexturized"), mat.isTexturized);
	glUniform1i(glGetUniformLocation(currentShading, "isEnvironment"), mat.isEnvironment);
	glUniform1f(glGetUniformLocation(currentShading, "environmentStrength"), mat.environmentStrength);

	// bind textures
	glUniform1i(glGetUniformLocation(currentShading, "textureMapping"), mat.textureMappingMode);
	if (mat.isTexturized)
	{
		GLuint texLoc = glGetUniformLocation(currentShading, "materialTexture");
		glUniform1i(texLoc, 0);

		GLuint envLoc = glGetUniformLocation(currentShading, "environmentTexture");
		glUniform1i(envLoc, 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mat.textureImage.textureId);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mat.textureEnvironment.textureId);
		
		// bind texture coordinates
		GLuint vTextureLoc = glGetAttribLocation(currentShading, "vTexture");
		glBindBuffer(GL_ARRAY_BUFFER, vData.buffers[3]);
		glEnableVertexAttribArray(vTextureLoc);
		glVertexAttribPointer(vTextureLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	// draw
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, drawingMode);
	glLineWidth(1);
	glDrawArrays(GL_TRIANGLES, 0, vData.size);
}

vec2 Renderer::getDims()
{
	return vec2(m_width, m_height);
}

void Renderer::drawPlusSign(vec4 pos, vec3 color)
{
	mat4 transform = Proj * lookAt;
	vec4 vertex[6];
	int i = 0;
	vertex[i++] = pos - vec4(0.5, 0.0, 0.0, 0.0);
	vertex[i++] = pos + vec4(0.5, 0.0, 0.0, 0.0);
	vertex[i++] = pos - vec4(0.0, 0.5, 0.0, 0.0);
	vertex[i++] = pos + vec4(0.0, 0.5, 0.0, 0.0);
	vertex[i++] = pos - vec4(0.0, 0.0, 0.5, 0.0);
	vertex[i++] = pos + vec4(0.0, 0.0, 0.5, 0.0);

	glDrawLines(vertex, &vec4(color, 1.0f), 6, transform, GL_LINES, true);
}

void Renderer::drawLightIndicator(vec4 pos, vec3 color, vec4 direction)
{
	bool parallel = !(direction.x == 0.0 && direction.y == 0.0 && direction.z == 0.0);
	mat4 transform = Proj * lookAt;
	vec4 vertex[6];
	int i = 0;
	vertex[i++] = pos - vec4(1.0, 0.0, 0.0, 0.0);
	vertex[i++] = pos + vec4(1.0, 0.0, 0.0, 0.0);
	vertex[i++] = pos - vec4(0.0, 1.0, 0.0, 0.0);
	vertex[i++] = pos + vec4(0.0, 1.0, 0.0, 0.0);
	vertex[i++] = pos - vec4(0.0, 0.0, 1.0, 0.0);
	vertex[i++] = pos + vec4(0.0, 0.0, 1.0, 0.0);

	glDrawLines(vertex, &vec4(color, 1.0f), 6, transform, GL_LINES, true);

	if (parallel)
	{
		vec4 vertexArrow[2];
		i = 0;
		vertexArrow[i++] = pos;
		vertexArrow[i++] = pos + direction * 10;
		glDrawLines(vertexArrow, &vec4(color, 1.0f), 2, transform, GL_LINES, true);
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
}

///////////////////////////////////////////////////////////////////////////////