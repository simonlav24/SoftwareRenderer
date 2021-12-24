#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
#include "Light.h"

using namespace std;

enum transformFrame { world, model, camera, light };
enum transformMode { position, scale, rotation };
enum materialProperty {color, ambient, diffuse, specular, shine, special, emission};

class Model {
protected:
	virtual ~Model() {}
	
	
public:
	bool showIndicators;
	void virtual draw(Renderer* r)=0;
	// multiply by modelTransform matrix
	//void virtual transformModel(const mat4& trans, bool scalling=false)=0;
	//void virtual transformWorld(const mat4& trans)=0;
	void virtual transform(const mat4& transform, bool world, bool scalling = false)=0;
	vec3 virtual getPosition() = 0;
	void virtual setPosition(vec3 pos) = 0;
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

	transformFrame tState;
	transformMode tMode;

	Scene(Renderer *renderer) : m_renderer(renderer), activeModel(-1), activeCamera(-1), tState(world), tMode(position),
		showBoundingBox(false), showFaceNormals(false), showVertexNormals(false), showGrid(false), showIndicators(true)
		{
		m_renderer->sceneLights = &(this->lights);
	};

	void draw();
	void drawOriginPoint();
	void drawGrid();
	void drawDemo();
	void toggleIndicators();

	// models
	void createPrimitive();
	void switchActiveModel();
	void deleteActiveModel();
	void transformActiveModel(const mat4& transform, bool scalling = false, bool rotation = false);
	void loadOBJModel(string fileName);
	void changeMaterial(materialProperty prop, vec3 values);
	vec3 getMaterial(materialProperty prop);

	// cameras
	void addCamera();
	void rotateZoomCamera(int dx, int dy, int scroll, GLfloat step);
	void translateCamera(int dx, int dy);
	void lookAtModel();
	void switchActiveCamera();
	void resetCameraPosition();
	void reshapeCamera(int width, int height);
	Camera& currentCamera();
	void setProjCam();

	// lights
	void addLight(LightType type=point);
	void moveLight(vec3 pos);
	void deleteActiveLight();
	void switchActiveLight();
	void changeLightColor(vec3 color);
	void changeLightPosition(vec3 pos);
	void changeLightDirection(vec3 dir);

	int activeModel;
	int activeLight;
	int activeCamera;
	
	bool showBoundingBox;
	bool showFaceNormals;
	bool showVertexNormals;
	bool showIndicators;
	bool showGrid;
};