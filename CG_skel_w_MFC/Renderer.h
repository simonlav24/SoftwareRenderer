#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"

using namespace std;
class Renderer
{
	float *m_outBuffer; // 3*width*height
	float *m_zbuffer; // width*height
	int m_width, m_height;
	int border = 200;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();

	//////////////////////////////
	// openGL stuff. Don't touch.

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
	void DrawTriangles(const std::vector<vec3>& vertices, const int count, const vec3& color);
	void SetCameraTransform(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	
	// extra stuff:

	// draw single pixel (0 < RGB < 1)
	void drawPixel(int x, int y, const vec3& color);
	// clear m_out buffer
	void clearBuffer();
	void DestroyBuffers();

	void drawLine(int x0, int y0, int x1, int y1, const vec3& color);
	void lineSteep(int x0, int y0, int x1, int y1, const vec3& color);
	void lineflat(int x0, int y0, int x1, int y1, const vec3& color);

	void drawCameraIndicator(vec4 pos);
	void reshape(int width, int height);

	vec2 getDims();
};
