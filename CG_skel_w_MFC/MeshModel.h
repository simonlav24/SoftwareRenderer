#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>
#include <map>

using namespace std;

class MeshModel : public Model
{
protected :
	MeshModel() {}
	//useful arrays contaning info about the models
	vec3* vertex_positions;
	vec3* vertexNormal_positions;
	vec2* vertexTexture_positions;
	vec3* faceNormals;
	vec3* centerPoints;
	//transform matrix
	mat4 _model_transform;
	mat4 _world_transform;
	mat4 _normal_transform;
	mat4 _normal_world_transform;
	// sizes of arrays
	int vertexCount = 0; //num faces * 3, not actualy count of vertices
	int vertexNormalsCount = 0;
	int faceCount = 0;
	// 6 vertices for bounding box definition
	vec3 bounding_box[2];
	vec4 bounding_BoxFull[24];

	GLuint vao;
	GLuint buffers[4];
	
public:
	
	Material mat;

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	//draw funcs
	void draw(Renderer* r);
	void drawBoundingBox(Renderer* r, mat4& cTransform, mat4& projection);
	void drawWorldAxis(Renderer* r);
	void drawFaceNormals(Renderer* r, mat4& cTransform, mat4& projection);
	void drawVertexNormals(Renderer* r, mat4& cTransform, mat4& projection);
	// calculate bounding box
	void calculateBoundingBox(vector<vec3>& vertex);	
	// multiply by Transform matrix
	void transform(const mat4& transform, bool world, bool scalling = false);
	//void transformModel(const mat4& transform, bool scalling=false);
	//void transformWorld(const mat4& transform);
	vec3 getPosition() override;
	void setPosition(vec3 pos) override;

	void loadTexture(std::string fileName);
	void bindData();
	
};

class PrimMeshModel : public MeshModel
{
public:
	PrimMeshModel();
};