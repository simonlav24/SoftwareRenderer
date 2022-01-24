#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

enum transformFrame { world, model, camera };
enum transformMode { position, scale, rotation };

struct Line {
	vec3 p1;
	vec3 p2;
	Line(vec3 p1, vec3 p2) :p1(p1), p2(p2){}
};

class Model {
protected:
	virtual ~Model() {}
public:
	void virtual draw(Renderer* r, mat4& cTransform, mat4& projection, vec3& color)=0;

	// multiply by modelTransform matrix
	void virtual transformModel(const mat4& trans, bool scalling=false)=0;
	void virtual transformWorld(const mat4& trans)=0;
	vec3 virtual getPosition()=0;
};


class Light {

};

class Camera {
public:
	mat4 cTransform;
	mat4 projection;

	vec4 Eye;
	vec4 At;
	vec4 Up;

	Camera();
	void setTransformation(const mat4& transform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void Frustum( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void Perspective( const float fovy, const float aspect,
		const float zNear, const float zFar);

};

class Scene {

	vector<Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer *m_renderer;

	vector<vec4> grid;

public:
	transformFrame tState;
	transformMode tMode;
	
	Scene(Renderer *renderer) : m_renderer(renderer), activeModel(-1), activeCamera(-1), tState(model), tMode(position),
		showBoundingBox(false), showFaceNormals(false), showVertexNormals(false), showGrid(false)
		{};
	void transformActiveModel(const mat4& transform, bool scalling=false);
	void loadOBJModel(string fileName);
	void draw();
	void drawOriginPoint();
	void drawGrid();
	void drawDemo();
	
	void createPrimitive();

	void switchActiveModel();
	void deleteActiveModel();
	void switchActiveCamera();
	void addCamera();
	void rotateZoomCamera(int dx, int dy, int scroll);
	void translateCamera(int dx, int dy);
	void lookAtModel();
	void resetCameraPosition();

	int activeModel;
	int activeLight;
	int activeCamera;
	Camera& currentCamera();
	
	bool showBoundingBox;
	bool showFaceNormals;
	bool showVertexNormals;
	bool showGrid;
};