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
	glProgramArray.toon = InitShader("Shaders/toon_vs.glsl", "Shaders/toon_fs.glsl");
	glProgramArray.toon_silhouette = InitShader("Shaders/toon_silhouette_vs.glsl", "Shaders/toon_silhouette_fs.glsl");

	timeStep = 0.0f;
	quantizationNum = 3;
	isVertexAnimating = 0;
	isColorAnimating = 0;

	for(int y = 0; y < 64; y++)
		for (int x = 0; x < 64; x++)
		{
			noiseKernel[y * 64 + x] = rand() % 256;
		}

	glGenTextures(1, &noiseId);

	glBindTexture(GL_TEXTURE_2D, noiseId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 64, 64, 0, GL_RED, GL_UNSIGNED_BYTE, noiseKernel);

	glBindTexture(GL_TEXTURE_2D, noiseId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDrawLines(vertices, colors, 6, transform, GL_LINES);
	
}

void Renderer::drawGrid()
{
	vec4 vertices[84];
	int k = 0;
	vec4 color[1] = { vec4(0.5f, 0.5f, 0.5f, 1.0f) };
	for (int j = 0; j < 21; j++)
	{
		vertices[k++] = vec4(-10.0f + (float)j, 0.0f, -10.0f, 1.0f);
		vertices[k++] = vec4(-10.0f + (float)j, 0.0f, 10.0f, 1.0f);
	}
	for (int j = 0; j < 21; j++)
	{
		vertices[k++] = vec4(-10.0f, 0.0f, -10.0f + (float)j, 1.0f);
		vertices[k++] = vec4(10.0f, 0.0f, -10.0f + (float)j, 1.0f);
	}
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDrawLines(vertices, color, 84, Proj * lookAt, GL_LINES, true);

}

void Renderer::glDrawLines(vec4* vertices, vec4* colors, int size, mat4 transform, GLuint lineMode, bool singleColor)
{
	glUseProgram(glProgramArray.line);

	// set transform
	glUniformMatrix4fv(glGetUniformLocation(glProgramArray.line, "transform"), 1, GL_TRUE, transform);

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
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
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

	glUseProgram(glProgramArray.line);

	mat4 identity;
	vec4 first(0.2f, 0.2f, 0.2f, 1.0f);
	vec4 second(0.5f, 0.5f, 0.5f, 1.0f);
	vec4 vertices[] = { vec4(-1.0f, -1.0f, 0.0f, 1.0f), vec4(-1.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, -1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f) };
	vec4 colors[] = { first, second, first, second };

	// set transform identity for background
	glUniformMatrix4fv(glGetUniformLocation(glProgramArray.line, "transform"), 1, GL_TRUE, identity);

	// draw background
	GLuint buffers[2];
	GLuint vao;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, buffers);

	GLuint vPositionLoc = glGetAttribLocation(glProgramArray.line, "vPosition");
	GLuint vColorLoc = glGetAttribLocation(glProgramArray.line, "vColor");

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * 4, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vPositionLoc);
	glVertexAttribPointer(vPositionLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * 4, colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vColorLoc);
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBindVertexArray(vao);
	glLineWidth(1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glUseProgram(0);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(2, buffers);
}

// hsv, h:0->360, s:0->1, v:0->1 to rgb: 0->1 
vec3 hsv2rgb(vec3 hsv)
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

void Renderer::DrawModel(vaoData vData, Material mat, mat4 worldModel, mat4 normalMat, int shading)
{
	vec3 Wirecolor = vec3(1.0, 0.5, 0.7);

	GLuint currentShading;
	if (shading == 0)
	{
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
		case Toon:
			// first, draw silhouette
			DrawModel(vData, mat, worldModel, normalMat, 1);
			// secondly draw toon shading
			currentShading = glProgramArray.toon;
			break;
		}
	}
	else if (shading == 1)
	{
		currentShading = glProgramArray.toon_silhouette;
	}

	glUseProgram(currentShading);

	// make uniform: camtransform, projection
	glUniformMatrix4fv(glGetUniformLocation(currentShading, "lookAt"), 1, GL_TRUE, lookAt);
	glUniformMatrix4fv(glGetUniformLocation(currentShading, "proj"), 1, GL_TRUE, Proj);

	// make uniform: model, world
	glUniformMatrix4fv(glGetUniformLocation(currentShading, "worldModelMat"), 1, GL_TRUE, worldModel);

	// make uniform normal mat
	glUniformMatrix4fv(glGetUniformLocation(currentShading, "normalMat"), 1, GL_TRUE, normalMat);

	// make uniform material
	glUniform3fv(glGetUniformLocation(currentShading, "matAmbient"), 1, mat.ambientColor);
	glUniform3fv(glGetUniformLocation(currentShading, "matDiffuse"), 1, mat.diffuseColor);
	glUniform3fv(glGetUniformLocation(currentShading, "matSpecular"), 1, mat.specularColor);
	glUniform3fv(glGetUniformLocation(currentShading, "matEmissive"), 1, mat.emissiveColor);
	glUniform1f(glGetUniformLocation(currentShading, "matShininess"), mat.shininessCoeficient);
	glUniform1f(glGetUniformLocation(currentShading, "environmentStrength"), mat.environmentStrength);
	glUniform1f(glGetUniformLocation(currentShading, "normalMapStrength"), mat.normalStrength);

	// bind animation
	glUniform1f(glGetUniformLocation(currentShading, "timeStep"), timeStep);
	glUniform1i(glGetUniformLocation(currentShading, "isVertexAnimating"), isVertexAnimating);
	glUniform1i(glGetUniformLocation(currentShading, "isColorAnimating"), isColorAnimating);

	glUniform1i(glGetUniformLocation(currentShading, "isNoiseTexture"), mat.isNoiseTexture);

	// make uniform color wireframe
	glUniform3fv(glGetUniformLocation(currentShading, "wireColor"), 1, Wirecolor);

	// make uniform viewer
	glUniform3fv(glGetUniformLocation(currentShading, "viewerPos"), 1, viewerPos[0]);

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
	glUniform1i(glGetUniformLocation(currentShading, "lightCount"), min(6, sceneLights->size()));
	glUniform1i(glGetUniformLocation(currentShading, "quantizationNum"), quantizationNum);

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
	else if (shadingSetup == Flat || shadingSetup == Gouraud || shadingSetup == Phong || shadingSetup == Toon)
	{
		GLuint vNormalLoc = glGetAttribLocation(currentShading, "vNormal");
		glBindBuffer(GL_ARRAY_BUFFER, vData.buffers[shadingSetup == Flat ? 1 : 2]);
		glEnableVertexAttribArray(vNormalLoc);
		glVertexAttribPointer(vNormalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	// make uniform isTexturized
	glUniform1i(glGetUniformLocation(currentShading, "isTexturized"), mat.isTexturized);
	glUniform1i(glGetUniformLocation(currentShading, "isEnvironment"), mat.isEnvironment);
	glUniform1i(glGetUniformLocation(currentShading, "isNormalMap"), mat.isNormalMap);

	// bind textures
	glUniform1i(glGetUniformLocation(currentShading, "textureMapping"), mat.textureMappingMode);
	if (mat.isTexturized || mat.isNormalMap || mat.isEnvironment || mat.isNoiseTexture)
	{
		GLuint texLoc = glGetUniformLocation(currentShading, "materialTexture");
		glUniform1i(texLoc, 0);

		GLuint envLoc = glGetUniformLocation(currentShading, "environmentTexture");
		glUniform1i(envLoc, 1);

		GLuint norLoc = glGetUniformLocation(currentShading, "normalMapTexture");
		glUniform1i(norLoc, 2);

		GLuint noiLoc = glGetUniformLocation(currentShading, "noiseTexture");
		glUniform1i(noiLoc, 3);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mat.textureImage.textureId);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mat.textureEnvironment.textureId);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mat.textureNormal.textureId);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, mat.textureNoise.textureId);
		
		// bind texture coordinates
		GLuint vTextureLoc = glGetAttribLocation(currentShading, "vTexture");
		glBindBuffer(GL_ARRAY_BUFFER, vData.buffers[3]);
		glEnableVertexAttribArray(vTextureLoc);
		glVertexAttribPointer(vTextureLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// bind tangents
		GLuint vTangentLoc = glGetAttribLocation(currentShading, "vTangent");
		glBindBuffer(GL_ARRAY_BUFFER, vData.buffers[4]);
		glEnableVertexAttribArray(vTangentLoc);
		glVertexAttribPointer(vTangentLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// bind bitangents
		GLuint vBitangentLoc = glGetAttribLocation(currentShading, "vBitangent");
		glBindBuffer(GL_ARRAY_BUFFER, vData.buffers[5]);
		glEnableVertexAttribArray(vBitangentLoc);
		glVertexAttribPointer(vBitangentLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	// draw
	if (shading == 0)
	{
		glLineWidth(1);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else
	{
		// silhoueete
		glLineWidth(5);
		glEnable(GL_DEPTH_TEST);
		drawingMode = GL_LINE;
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}
	
	glPolygonMode(GL_FRONT_AND_BACK, drawingMode);
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
	//DestroyBuffers();
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