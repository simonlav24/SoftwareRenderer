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
	vec3 *vertex_positions;
	//add more attributes
	mat4 _model_transform;
	mat4 _world_transform;
	mat4 _normal_transform;

	vec3 *faceNormals;
	vec3 *centerPoints;

	vec3 *vertexNormals;
	vec3 *vertexCenter;

	// ---- extra attributes

	// vertex full count
	int vertexCount;
	int actualVertexCount;
	int faceCount;
	// 2 vertices for bounding box definition
	vec3 bounding_box[2];

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer* r, mat4& cTransform, mat4& projection, vec3& color);

	// extra

	// calculate bounding box
	void calculateBoundingBox(vec3 vertex);
	// draw bounding box
	void drawBoundingBox(Renderer* r, mat4& cTransform, mat4& projection);
	void darwFaceNormals(Renderer* r, mat4& cTransform, mat4& projection);
	void darwVertexNormals(Renderer* r, mat4& cTransform, mat4& projection);

	// multiply by Transform matrix
	void transformModel(const mat4& transform, bool scalling=false);
	void transformWorld(const mat4& transform);

	void drawWorldAxis(Renderer* r, mat4& cTransform, mat4& projection);
	vec3 getPosition() override;
};

class PrimMeshModel : public MeshModel
{
public:
	PrimMeshModel();
};