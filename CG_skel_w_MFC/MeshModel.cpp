#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
//todo: understand better
#define screen(temp) vec3((temp.x) * r->getDims().x, (temp.y) * r->getDims().y, 0.0)
#define viewPort(dims, a) vec3((dims.x / 2.0) * (a.x + 1), (dims.y / 2.0) * (a.y + 1), a.z)

//triangle object
struct FaceIdcs
{
	int v[4];
	int vn[4];
	int vt[4];

	FaceIdcs()
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;
	}

	FaceIdcs(std::istream & aStream)
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;

		char c;
		for(int i = 0; i < 3; i++)
		{
			aStream >> std::ws >> v[i] >> std::ws;
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> std::ws;
			if (aStream.peek() == '/')
			{
				aStream >> c >> std::ws >> vn[i];
				continue;
			}
			else
				aStream >> vt[i];
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> vn[i];
		}
	}

	FaceIdcs(int a, int b, int c)
	{
		v[0] = a;
		v[1] = b;
		v[2] = c;
	}
};

//parse vec3 from input
vec3 vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

//parse vec2 from input
vec2 vec2fFromStream(std::istream & aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}

GLfloat random(GLfloat lower, GLfloat upper) {
	return lower + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (upper - lower)));
}

MeshModel::MeshModel(string fileName)
{
	// initializations
	bounding_box[0] = vec3(0, 0, 0);
	bounding_box[1] = vec3(0, 0, 0);

	loadFile(fileName);
	//mat.color = vec3(0.7, 0.8, 0.7);
	mat.special = false;
	showIndicators = true;
}

MeshModel::~MeshModel(void)
{
	delete[] vertex_positions;
	delete[] vertexNormals;
	delete[] faceNormals;
	delete[] centerPoints;
	delete[] normal_positions;
}

void MeshModel::loadFile(string fileName)
{
	//loading file buffer
	ifstream ifile(fileName.c_str());
	//saving info about v, vn and f
	vector<vec3> vertices_normals;
	vector<FaceIdcs> faces;
	// while not end of file
	while (!ifile.eof())
	{
		// get line
		string curLine;
		getline(ifile, curLine);

		// read type of the line
		istringstream issLine(curLine);
		string lineType;
		issLine >> std::ws >> lineType;

		// based on the type parse data (v, vn or f)
		if (lineType == "v") /*point pos*/
		{
			vertices.push_back(vec3fFromStream(issLine));
		}
		else if (lineType == "f") { /*face data*/
			faces.push_back(issLine);
		}
		else if (lineType == "vn") /*face data*/
		{
			vertices_normals.push_back(vec3fFromStream(issLine));
		}
		else if (lineType == "#" || lineType == "")
		{
			// comment / empty line
		}
		else
		{
			cout << "Found unknown line Type \"" << lineType << "\"" << endl;
		}
	}

	// calculate bounding box
	calculateBoundingBox(vertices);

	// scale models to match general size (if its way too big or way too small):
	if (2.0 > bounding_box[0].y || bounding_box[0].y > 6.0)
	{
		GLfloat scaleFactor = 4.0 / bounding_box[0].y;
		bounding_box[0] = vec3();
		bounding_box[1] = vec3();
		for (int i = 0; i < vertices.size(); i++)
		{
			vertices[i] = vertices[i] * scaleFactor;
		}
	}


	// calculate bounding box yet again
	calculateBoundingBox(vertices);

	//init vertex_positions array
	vertexCount = faces.size() * 3;
	vertex_positions = new vec3[vertexCount];
	normal_positions = new vec3[vertexCount];
	// iterate through all stored faces and create triangles
	for (int i = 0, k = 0; i < faces.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			vertex_positions[k++] = vertices[faces[i].v[j] - 1];
		}
	}

	//calculate center points & normals
	faceCount = faces.size();
	vec3* faceNorms = new vec3[faceCount];
	centerPoints = new vec3[faceCount];
	for (int i = 0, k = 0; i < vertexCount; i += 3)
	{
		vec3 p1 = vertex_positions[i], p2 = vertex_positions[i + 1], p3 = vertex_positions[i + 2];
		faceNorms[k] = normalize(cross(p2 - p1, p3 - p1)); //normal according to formula
		centerPoints[k] = (p1 + p2 + p3) / 3.0;//center point according to formula
		k++;
	}

	faceNormals = new vec3[vertexCount];
	for (int i = 0; i < vertexCount; i++)
	{
		faceNormals[i] = faceNorms[i / 3];
	}


	delete[] faceNorms;

	//calculating vertex normals
	vertexNormalsCount = vertices.size();
	vertexNormals = new vec3[vertexNormalsCount];
	if (vertices_normals.empty()) { //if no given vn-s, calculating manualy (according to formula)
		for (int i = 0; i < faceCount; i++) {
			vec3 p1 = vertex_positions[3 * i], p2 = vertex_positions[3 * i + 1], p3 = vertex_positions[3 * i + 2];
			vec3 addition = length(cross(p2 - p1, p3 - p1)) * faceNormals[i];
			for (int j = 0; j < 3; j++) {
				vertexNormals[faces[i].v[j] - 1] += addition;
			}
		}
	}
	else {
		for (int i = 0; i < faceCount; i++) {
			for (int j = 0; j < 3; j++) {
				vertexNormals[faces[i].v[j] - 1] += vertices_normals[faces[i].vn[j] - 1];
			}
		}
	}

	//normalize
	for (int i = 0; i < vertexNormalsCount; i++) {
		vertexNormals[i] = normalize(vertexNormals[i]);
	}

	//
	for (int i = 0, k = 0; i < faces.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			normal_positions[k++] = vertexNormals[faces[i].v[j] - 1];
		}
	}
	
}

vec3 transformPoint(vec4 point, mat4 m, vec2 rendererDims)
{
	return viewPort(rendererDims, homo2noHomo(m * point));
}

void MeshModel::drawWorldAxis(Renderer* r)
{
	mat4 Mw = r->ProjCam * _world_transform;
	vec2 rendererDims = r->getDims();

	vec4 zero = vec4(0, 0, 0, 1);
	zero = Mw * zero;
	zero = homo2noHomo(zero);
	vec3 zeroS = viewPort(rendererDims, zero);

	vec4 xAxis = vec4(1, 0, 0, 1);
	xAxis = Mw * xAxis;
	xAxis = homo2noHomo(xAxis);
	vec3 xS = viewPort(rendererDims, xAxis);
	r->drawLine((int)zeroS.x, (int)zeroS.y, (int)xS.x, (int)xS.y, vec3(1, 0, 0));

	vec4 yAxis = vec4(0, 1, 0, 1);
	yAxis = Mw * yAxis;
	yAxis = homo2noHomo(yAxis);
	vec3 yS = viewPort(rendererDims, yAxis);
	r->drawLine((int)zeroS.x, (int)zeroS.y, (int)yS.x, (int)yS.y, vec3(0, 1, 0));

	vec4 zAxis = vec4(0, 0, 1, 1);
	zAxis = Mw * zAxis;
	zAxis = homo2noHomo(zAxis);
	vec3 zS = viewPort(rendererDims, zAxis);
	r->drawLine((int)zeroS.x, (int)zeroS.y, (int)zS.x, (int)zS.y, vec3(0, 0, 1));
}

void MeshModel::draw(Renderer* r)
{
	mat4 normalTransform = _normal_world_transform * _normal_transform;
	mat4 worldModel = _world_transform * _model_transform;

	/*for (int i = 0; i < vertexCount; i++)
	{
		cout << "point: " << vertex_positions[i] << endl;
		cout << "normal: " << faceNormals[i] << endl;
	}*/
	r->DrawModel(vertex_positions, faceNormals, normal_positions, vertexCount, mat, worldModel, normalTransform);

}

vec3 MeshModel::getPosition()
{
	vec4 pos(0.0, 0.0, 0.0, 1.0);
	return homo2noHomo(_world_transform * _model_transform * pos);
}

void MeshModel::setPosition(vec3 pos)
{
	_world_transform = Translate(-getPosition()) * _world_transform;
	_world_transform = Translate(pos) * _world_transform;
}

void MeshModel::calculateBoundingBox(vector<vec3>& vertices)
{
	for (int i = 0; i < vertices.size(); i++) {
		// positive direction
		bounding_box[0].x = max(bounding_box[0].x, vertices[i].x);
		bounding_box[0].y = max(bounding_box[0].y, vertices[i].y);
		bounding_box[0].z = max(bounding_box[0].z, vertices[i].z);
		// negative direction direction
		bounding_box[1].x = min(bounding_box[1].x, vertices[i].x);
		bounding_box[1].y = min(bounding_box[1].y, vertices[i].y);
		bounding_box[1].z = min(bounding_box[1].z, vertices[i].z);
	}
}

void MeshModel::drawBoundingBox(Renderer* r, mat4& cTransform, mat4& projection)
{
	vec4 vertices[24];
	vec3 line[2];

	int k = 0;
	for (int i = 0; i < 2; i++)
	{
		line[0] = bounding_box[i];
		line[1] = bounding_box[i]; line[1].x = bounding_box[1 - i].x;
		vertices[k++] = vec4(line[0]);
		vertices[k++] = vec4(line[1]);

		line[0] = bounding_box[i];
		line[1] = bounding_box[i]; line[1].y = bounding_box[1 - i].y;
		vertices[k++] = vec4(line[0]);
		vertices[k++] = vec4(line[1]);

		line[0] = bounding_box[i];
		line[1] = bounding_box[i]; line[1].z = bounding_box[1 - i].z;
		vertices[k++] = vec4(line[0]);
		vertices[k++] = vec4(line[1]);
	}

	line[0] = bounding_box[0]; line[0].y = bounding_box[1].y;
	line[1] = bounding_box[1]; line[1].x = bounding_box[0].x;
	vertices[k++] = vec4(line[0]);
	vertices[k++] = vec4(line[1]);
	line[0] = bounding_box[0]; line[0].z = bounding_box[1].z;
	line[1] = bounding_box[1]; line[1].x = bounding_box[0].x;
	vertices[k++] = vec4(line[0]);
	vertices[k++] = vec4(line[1]);
	line[0] = bounding_box[0]; line[0].y = bounding_box[1].y;
	line[1] = bounding_box[1]; line[1].z = bounding_box[0].z;
	vertices[k++] = vec4(line[0]);
	vertices[k++] = vec4(line[1]);
	line[0] = bounding_box[0]; line[0].x = bounding_box[1].x;
	line[1] = bounding_box[1]; line[1].z = bounding_box[0].z;
	vertices[k++] = vec4(line[0]);
	vertices[k++] = vec4(line[1]);
	line[0] = bounding_box[0]; line[0].x = bounding_box[1].x;
	line[1] = bounding_box[1]; line[1].y = bounding_box[0].y;
	vertices[k++] = vec4(line[0]);
	vertices[k++] = vec4(line[1]);
	line[0] = bounding_box[0]; line[0].z = bounding_box[1].z;
	line[1] = bounding_box[1]; line[1].y = bounding_box[0].y;
	vertices[k++] = vec4(line[0]);
	vertices[k++] = vec4(line[1]);
	r->glDrawLines(vertices, 24, vec4(1.0, 1.0, 0.0, 1.0));
	return;
	/*
	vec2 rendererDims = r->getDims();
	//mat4 M = projection * transpose(cTransform) * _world_t ransform * _model_transform;
	mat4 M = projection * cTransform * _world_transform * _model_transform;

	//vec3 line[2];

	for (int i = 0; i < 2; i++)
	{
		line[0] = bounding_box[i];
		line[1] = bounding_box[i]; line[1].x = bounding_box[1-i].x;
		line[0] = transformPoint(line[0], M, rendererDims);
		line[1] = transformPoint(line[1], M, rendererDims);
		r->drawLine((int)line[0].x, (int)line[0].y, (int)line[1].x, (int)line[1].y, vec3(0.8, 0.8, 0.2));

		line[0] = bounding_box[i];
		line[1] = bounding_box[i]; line[1].y = bounding_box[1-i].y;
		line[0] = transformPoint(line[0], M, rendererDims);
		line[1] = transformPoint(line[1], M, rendererDims);
		r->drawLine((int)line[0].x, (int)line[0].y, (int)line[1].x, (int)line[1].y, vec3(0.8, 0.8, 0.2));

		line[0] = bounding_box[i];
		line[1] = bounding_box[i]; line[1].z = bounding_box[1-i].z;
		line[0] = transformPoint(line[0], M, rendererDims);
		line[1] = transformPoint(line[1], M, rendererDims);
		r->drawLine((int)line[0].x, (int)line[0].y, (int)line[1].x, (int)line[1].y, vec3(0.8, 0.8, 0.2));
	}

	line[0] = bounding_box[0]; line[0].y = bounding_box[1].y;
	line[1] = bounding_box[1]; line[1].x = bounding_box[0].x;
	line[0] = transformPoint(line[0], M, rendererDims);
	line[1] = transformPoint(line[1], M, rendererDims);
	r->drawLine((int)line[0].x, (int)line[0].y, (int)line[1].x, (int)line[1].y, vec3(0.8, 0.8, 0.2));

	line[0] = bounding_box[0]; line[0].z = bounding_box[1].z;
	line[1] = bounding_box[1]; line[1].x = bounding_box[0].x;
	line[0] = transformPoint(line[0], M, rendererDims);
	line[1] = transformPoint(line[1], M, rendererDims);
	r->drawLine((int)line[0].x, (int)line[0].y, (int)line[1].x, (int)line[1].y, vec3(0.8, 0.8, 0.2));

	line[0] = bounding_box[0]; line[0].y = bounding_box[1].y;
	line[1] = bounding_box[1]; line[1].z = bounding_box[0].z;
	line[0] = transformPoint(line[0], M, rendererDims);
	line[1] = transformPoint(line[1], M, rendererDims);
	r->drawLine((int)line[0].x, (int)line[0].y, (int)line[1].x, (int)line[1].y, vec3(0.8, 0.8, 0.2));

	line[0] = bounding_box[0]; line[0].x = bounding_box[1].x;
	line[1] = bounding_box[1]; line[1].z = bounding_box[0].z;
	line[0] = transformPoint(line[0], M, rendererDims);
	line[1] = transformPoint(line[1], M, rendererDims);
	r->drawLine((int)line[0].x, (int)line[0].y, (int)line[1].x, (int)line[1].y, vec3(0.8, 0.8, 0.2));

	line[0] = bounding_box[0]; line[0].x = bounding_box[1].x;
	line[1] = bounding_box[1]; line[1].y = bounding_box[0].y;
	line[0] = transformPoint(line[0], M, rendererDims);
	line[1] = transformPoint(line[1], M, rendererDims);
	r->drawLine((int)line[0].x, (int)line[0].y, (int)line[1].x, (int)line[1].y, vec3(0.8, 0.8, 0.2));

	line[0] = bounding_box[0]; line[0].z = bounding_box[1].z;
	line[1] = bounding_box[1]; line[1].y = bounding_box[0].y;
	line[0] = transformPoint(line[0], M, rendererDims);
	line[1] = transformPoint(line[1], M, rendererDims);
	r->drawLine((int)line[0].x, (int)line[0].y, (int)line[1].x, (int)line[1].y, vec3(0.8, 0.8, 0.2));
	*/
}

void MeshModel::drawFaceNormals(Renderer* r, mat4& cTransform, mat4& projection)
{
	vec2 rendererDims = r->getDims();
	mat4 projWorld = projection * cTransform;
	mat4 normalWorld = _normal_world_transform * _normal_transform;
	mat4 modelWorld = _world_transform * _model_transform;

	for (int i = 0; i < faceCount; i++)
	{
		vec4 origin = centerPoints[i];
		vec4 normal = faceNormals[i];
		normal.w = 0;

		origin = modelWorld * origin;
		normal = normalWorld * normal;
		normal.w = 0;
		normal = normalize(normal);

		vec4 point = origin + normal;
		normal =  origin + normal;
		origin = homo2noHomo(projWorld * origin);
		point = homo2noHomo(projWorld * point);
		
		vec3 point1 = viewPort(rendererDims, origin);
		vec3 point2 = viewPort(rendererDims, point);
		r->drawLine(point1.x, point1.y, point2.x, point2.y, vec3(1.0, 1.0, 0.0));
	}
}

void MeshModel::drawVertexNormals(Renderer* r, mat4& cTransform, mat4& projection)
{
	vec2 rendererDims = r->getDims();
	mat4 projWorld = projection * cTransform;
	mat4 normalWorld = _normal_world_transform * _normal_transform;
	mat4 modelWorld = _world_transform * _model_transform;

	for (int i = 0; i < vertexNormalsCount; i++)
	{
		vec4 origin = vertices[i];
		vec4 normal = vertexNormals[i];
		origin = modelWorld * origin;
		normal = normalWorld * normal;
		normal.w = 0;
		normal = normalize(normal);

		vec4 point = origin + normal;
		normal = origin + normal;
		origin = homo2noHomo(projWorld * origin);
		point = homo2noHomo(projWorld * point);

		vec3 point1 = viewPort(rendererDims, origin);
		vec3 point2 = viewPort(rendererDims, point);
		r->drawLine(point1.x, point1.y, point2.x, point2.y, vec3(1.0, 0.0, 1.0));
	}
}

void MeshModel::transform(const mat4& transform, bool world, bool scalling)
{
	if (world)
	{
		_world_transform = transform * _world_transform;
		mat4 normalScale = transform;
		if (scalling)
		{
			normalScale[0][0] = 1 / normalScale[0][0];
			normalScale[1][1] = 1 / normalScale[1][1];
			normalScale[2][2] = 1 / normalScale[2][2];
		}
		_normal_world_transform = normalScale * _normal_world_transform;
	}
	else
	{
		_model_transform = transform * _model_transform;
		mat4 normalScale = transform;
		if (scalling)
		{
			normalScale[0][0] = 1 / normalScale[0][0];
			normalScale[1][1] = 1 / normalScale[1][1];
			normalScale[2][2] = 1 / normalScale[2][2];
		}
		_normal_transform = normalScale * _normal_transform;
	}
}

PrimMeshModel::PrimMeshModel()
{
	vertex_positions = new vec3[36];
	vector<FaceIdcs> faces;
	vector<vec3> vertices_normals;

	vec3 offset(0.5, 0.5, 0.5);
	vertices.push_back(vec3(0.0, 0.0, 0.0) - offset);
	vertices.push_back(vec3(0.0, 0.0, 1.0) - offset);
	vertices.push_back(vec3(0.0, 1.0, 0.0) - offset);
	vertices.push_back(vec3(0.0, 1.0, 1.0) - offset);
	vertices.push_back(vec3(1.0, 0.0, 0.0) - offset);
	vertices.push_back(vec3(1.0, 0.0, 1.0) - offset);
	vertices.push_back(vec3(1.0, 1.0, 0.0) - offset);
	vertices.push_back(vec3(1.0, 1.0, 1.0) - offset);

	faces.push_back(FaceIdcs(1, 7, 5));
	faces.push_back(FaceIdcs(1, 3, 7));
	faces.push_back(FaceIdcs(1, 4, 3));
	faces.push_back(FaceIdcs(1, 2, 4));
	faces.push_back(FaceIdcs(3, 8, 7));
	faces.push_back(FaceIdcs(3, 4, 8));
	faces.push_back(FaceIdcs(5, 7, 8));
	faces.push_back(FaceIdcs(5, 8, 6));
	faces.push_back(FaceIdcs(1, 5, 6));
	faces.push_back(FaceIdcs(1, 6, 2));
	faces.push_back(FaceIdcs(2, 6, 8));
	faces.push_back(FaceIdcs(2, 8, 4));

	bounding_box[0] = vec3(0, 0, 0);
	bounding_box[1] = vec3(0, 0, 0);

	// calculate bounding box
	calculateBoundingBox(vertices);

	// scale models to match general size (if its way too big or way too small):
	if (2.0 > bounding_box[0].y || bounding_box[0].y > 6.0)
	{
		GLfloat scaleFactor = 4.0 / bounding_box[0].y;
		bounding_box[0] = vec3();
		bounding_box[1] = vec3();
		for (int i = 0; i < vertices.size(); i++)
		{
			vertices[i] = vertices[i] * scaleFactor;
		}
	}

	// calculate bounding box yet again
	calculateBoundingBox(vertices);

	//init vertex_positions array
	vertexCount = faces.size() * 3;
	vertex_positions = new vec3[vertexCount];
	// iterate through all stored faces and create triangles
	for (int i = 0, k = 0; i < faces.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			vertex_positions[k++] = vertices[faces[i].v[j] - 1];
		}
	}

	//calculate center points & normals
	faceCount = faces.size();
	faceNormals = new vec3[faceCount];
	centerPoints = new vec3[faceCount];
	for (int i = 0, k = 0; i < vertexCount; i += 3)
	{
		vec3 p1 = vertex_positions[i], p2 = vertex_positions[i + 1], p3 = vertex_positions[i + 2];
		faceNormals[k] = normalize(cross(p2 - p1, p3 - p1)); //normal according to formula
		centerPoints[k] = (p1 + p2 + p3) / 3.0;//center point according to formula
		k++;
	}

	//calculating vertex normals
	vertexNormalsCount = vertices.size();
	vertexNormals = new vec3[vertexNormalsCount];
	normal_positions = new vec3[vertexCount];

	if (vertices_normals.empty()) { //if no given vn-s, calculating manualy (according to formula)
		for (int i = 0; i < faceCount; i++) {
			vec3 p1 = vertex_positions[3 * i], p2 = vertex_positions[3 * i + 1], p3 = vertex_positions[3 * i + 2];
			vec3 addition = length(cross(p2 - p1, p3 - p1)) * faceNormals[i];
			for (int j = 0; j < 3; j++) {
				vertexNormals[faces[i].v[j] - 1] += addition;
			}
		}
	}
	else {
		for (int i = 0; i < faceCount; i++) {
			for (int j = 0; j < 3; j++) {
				vertexNormals[faces[i].v[j] - 1] += vertices_normals[faces[i].vn[j] - 1];
			}
		}
	}

	//normalize
	for (int i = 0; i < vertexNormalsCount; i++) {
		vertexNormals[i] = normalize(vertexNormals[i]);
	}

	for (int i = 0, k = 0; i < faces.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			normal_positions[k++] = vertexNormals[faces[i].v[j] - 1];
		}
	}

}