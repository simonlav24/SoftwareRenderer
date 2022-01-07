#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"
#include "Light.h"

using namespace std;

enum ShadingSetup { WireFrame, Flat, Phong, Gouraud };

struct GLProgramsArray {
	GLuint line;
	GLuint wireFrame;
	GLuint flat_gouraud;
	GLuint phong;
};

struct GLUniformLocArray {
	GLuint lookAt;
	GLuint projection;
	GLuint worldModel;
	GLuint normals;
	GLuint ambient;
	GLuint diffuse;
	GLuint specular;
	GLuint emissive;
	GLuint shininess;
	GLuint viewer;
};

class Renderer
{
	float *m_outBuffer; // 3*width*height
	float *m_zbuffer; // width*height
	float *m_blurBuffer;
	int m_width, m_height;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();

	////// post
	bool fogMode;
	vec3 calculateFog(vec3 color, GLfloat zValue);

	float* gaussianKernel;
	void convolute(float* buffer, float* kernel, int kernelWidth, int kernelHeight);

	//////////////////////////////
	// openGL stuff. Don't touch.
	int border = 100;
	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////

	/// hw3 stuff
	GLProgramsArray glProgramArray;
	GLUniformLocArray glUniformLocArray;
	//GLuint glprogramsArray[10];
	//GLuint glUniformLocArray[10]; 

public:
	Renderer();
	Renderer(int width, int height);
	~Renderer(void);
	void Init();

	mat4 lookAt;
	mat4 Proj;
	mat4 ProjCam;
	bool orthogonal;

	bool SSAA;
	bool lightBloom;

	float fogMaxdist;
	float fogMindist;
	vec3  fogColor;

	void SetCameraTransform(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	void DestroyBuffers();
	
	// viewer eye, at vec:
	vec4 viewerPos[2];
	vector<Light*> *sceneLights;

	// draw single pixel (0 < RGB < 1)
	void drawPixel(int x, int y, const vec3& color);

	void DrawTriangles(const std::vector<vec3>& vertices, Material& mat);
	void DrawTriangles(const std::vector<vec3>& vertices, Material& mat, vector<vec4>& faceNormals);
	void drawTrianglesWire(const std::vector<vec3>& vertices, Material& mat);
	void drawTriangleWire(vec3 p0, vec3 p1, vec3 p2, const vec3& color);
	void drawTriangleFlat(vec3 p0, vec3 p1, vec3 p2, Material& mat);
	void DrawTrianglePhong(vec3 p0, vec3 p1, vec3 p2, Material& mat, vec4& faceNormal);
	// clear m_out buffer
	void clearBuffer();

	void drawLine(int x0, int y0, int x1, int y1, const vec3& color);
	void drawLine(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, const vec3& color);
	void drawLine(vec3 a, vec3 b, const vec3& color);

	void drawModel(vector<vec4>& modelVertices, vector<vec4>& modelFaceNormals, vector<vec4>& modelVertexNormals, Material& mat);

	void drawPlusSign(vec4 pos, vec3 color);
	void drawLightIndicator(vec4 pos, vec3 color, vec4 direction);

	vec3 calculateAmbient(Material& mat);
	vec3 calculateDiffusion(vec3& pointInWorld, vec3& normalInWorld, Material& mat);
	vec3 calculateSpecular(vec3& pointInWorld, vec3& normalInWorld, Material& mat);

	void reshape(int width, int height);

	vec2 getDims();
	ShadingSetup shadingSetup;

	void postProccess();
	void toggleFog();


	void glDrawLinesColors(vec4* vertices, vec4* colors, int size, mat4 transform, GLuint lineMode = GL_LINE_STRIP);
	void glDrawLines(vec4* vertices, int size, vec4 color, mat4 transform, GLuint lineMode = GL_LINE_STRIP);

	void DrawModel(vec3* vertexPositions, vec3* faceNormals, vec3* vertexNormals, int size, Material mat, mat4 worldModel, mat4 normalMat);
	void drawOriginAxis();
};
