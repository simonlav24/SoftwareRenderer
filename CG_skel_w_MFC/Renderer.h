#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"
#include "Light.h"

using namespace std;

enum LightSetup { WireFrame, Flat, Phong, Gouraud };

class Renderer
{
	float *m_outBuffer; // 3*width*height
	float *m_zbuffer; // width*height
	int m_width, m_height;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();

	//////////////////////////////
	// openGL stuff. Don't touch.
	int border = 200;
	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
public:
	Renderer();
	Renderer(int width, int height);
	~Renderer(void);
	void Init();
	
	mat4 ProjCam;

	void SetCameraTransform(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	void DestroyBuffers();
	
	// viewer direction vec:
	vec4 viewerPos;
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

	void drawModel(vector<vec4>& modelVertices, vector<vec4>& modelFaceNormals, vector<vec4>& modelVertexNormals, mat4& ProjCam, Material& mat);

	void drawPlusSign(vec4 pos, vec3 color);
	void drawLightIndicator(vec4 pos, vec3 color);

	GLfloat calculateAmbient(Material& mat);
	GLfloat calculateDiffusion(vec3& pointInWorld, vec3& normalInWorld, Material& mat);
	GLfloat calculateSpecular(vec3& pointInWorld, vec3& normalInWorld, Material& mat);

	void reshape(int width, int height);

	vec2 getDims();
	LightSetup shadingSetup;
	
	// temporary light properties
	GLfloat ambientIntensity;
};
