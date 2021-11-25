#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

using namespace std;

class MeshModel : public Model
{
protected :
	MeshModel() {}
	vector<vec3> vertices;
	//useful arrays contaning info about the models
	vec3* vertex_positions;
	vec3* vertexNormals;
	vec3* faceNormals;
	vec3* centerPoints;
	//transform matrix
	mat4 _model_transform;
	mat4 _world_transform;
	mat4 _normal_transform;
	// sizes of arrays
	int vertexCount = 0; //num faces * 3, not actualy count of vertices
	int vertexNormalsCount = 0;
	int faceCount = 0;
	// 2 vertices for bounding box definition
	vec3 bounding_box[2];

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	//draw funcs
	void draw(Renderer* r, mat4& cTransform, mat4& projection, vec3& color);
	void drawBoundingBox(Renderer* r, mat4& cTransform, mat4& projection);
	void drawWorldAxis(Renderer* r, mat4& cTransform, mat4& projection);
	void drawFaceNormals(Renderer* r, mat4& cTransform, mat4& projection);
	void drawVertexNormals(Renderer* r, mat4& cTransform, mat4& projection);
	// calculate bounding box
	void calculateBoundingBox(vector<vec3>& vertex);	
	// multiply by Transform matrix
	void transformModel(const mat4& transform, bool scalling=false);
	void transformWorld(const mat4& transform);
	
	vec3 getPosition() override;
};

class PrimMeshModel : public MeshModel
{
public:
	PrimMeshModel();
};