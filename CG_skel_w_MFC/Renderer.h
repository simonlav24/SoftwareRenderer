#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"
#include "Light.h"

using namespace std;

enum ShadingSetup { WireFrame, Flat, Phong, Gouraud, Toon };

struct GLProgramsArray {
	GLuint line;
	GLuint wireFrame;
	GLuint flat_gouraud;
	GLuint phong;
	GLuint toon;
	GLuint toon_silhouette;
};

struct vaoData {
	GLuint vao;
	int size;
	GLuint* buffers;
	vec3* vertexPos;
	vec3* faceNormals;
	vec3* vertexNormals;
	vec2* vertexTexture;
	vec3* tangents;
	vec3* bitangents;
};

class Renderer
{
	int m_width, m_height;

	void CreateBuffers(int width, int height);

	//////////////////////////////
	// openGL stuff. Don't touch.
	int border = 100;
	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////

	GLProgramsArray glProgramArray;

public:
	Renderer();
	Renderer(int width, int height);
	~Renderer(void);
	void Init();

	mat4 lookAt;
	mat4 Proj;
	mat4 ProjCam;
	bool orthogonal;

	int isVertexAnimating;
	int isColorAnimating;
	float timeStep;

	void SwapBuffers();
	void DestroyBuffers();
	
	unsigned char noiseKernel[64*64];
	GLuint noiseId;

	// viewer eye, at vec:
	vec4 viewerPos[2];
	vector<Light*> *sceneLights;

	// clear buffer
	void clearBuffer();

	void drawPlusSign(vec4 pos, vec3 color);
	void drawLightIndicator(vec4 pos, vec3 color, vec4 direction);

	void reshape(int width, int height);

	vec2 getDims();
	ShadingSetup shadingSetup;
	int quantizationNum;

	void glDrawLines(vec4* vertices, vec4* colors, int size, mat4 transform, GLuint lineMode = GL_LINE_STRIP, bool singleColor = false);
	//void glDrawLines(vec4* vertices, int size, vec4 color, mat4 transform, GLuint lineMode = GL_LINE_STRIP);

	void DrawModel(vaoData vData, Material mat, mat4 worldModel, mat4 normalMat, int shading=0);
	void drawOriginAxis();
	void drawGrid();

	void loadEnvironmentTexture(char* fileName);
};
