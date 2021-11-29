#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
#include "Light.h"

using namespace std;

enum transformFrame { world, model, camera, light };
enum transformMode { position, scale, rotation };

class Model {
protected:
	virtual ~Model() {}
public:
	void virtual draw(Renderer* r, mat4& cTransform, mat4& projection, vec3& color)=0;
	void virtual draw(Renderer* r, mat4& ProjCam)=0;
	// multiply by modelTransform matrix
	//void virtual transformModel(const mat4& trans, bool scalling=false)=0;
	//void virtual transformWorld(const mat4& trans)=0;
	void virtual transform(const mat4& transform, bool world, bool scalling = false)=0;
	vec3 virtual getPosition()=0;
};

class Camera {
public:
	mat4 cTransform;
	mat4 projection;

	vec4 Eye;
	vec4 At;
	vec4 Up;

	vec4 rectPos;
	vec2 zPos;
	bool orthogonal;

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

public:
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
	
	// cameras
	void addCamera();
	void rotateZoomCamera(int dx, int dy, int scroll);
	void translateCamera(int dx, int dy);
	void lookAtModel();
	void resetCameraPosition();
	void reshapeCamera(int width, int height);
	Camera& currentCamera();

	// lights
	void addLight();
	void moveLight(vec3 pos);
	void deleteActiveLight();
	void switchActiveLight();

	int activeModel;
	int activeLight;
	int activeCamera;
	
	bool showBoundingBox;
	bool showFaceNormals;
	bool showVertexNormals;
	bool showGrid;
};