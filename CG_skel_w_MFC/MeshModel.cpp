#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
#define screen(temp) vec3((temp.x) * r->getDims().x, (temp.y) * r->getDims().y, 0.0)
#define viewPort(dims, a) vec3((dims.x / 2.0) * (a.x + 1), (dims.y / 2.0) * (a.y + 1), a.z)

// Polygon object
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

vec3 vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

vec2 vec2fFromStream(std::istream & aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}

MeshModel::MeshModel(string fileName)
{
	// initializations
	bounding_box[0] = vec3(0, 0, 0);
	bounding_box[1] = vec3(0, 0, 0);

	loadFile(fileName);
}

MeshModel::~MeshModel(void)
{
	delete[] vertex_positions;
}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
	vector<vec3> vertexNormals;
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
		if (lineType == "v") /*vector pos*/
		{
			vertices.push_back(vec3fFromStream(issLine));

			// calculate bounding box
			calculateBoundingBox(vertices.back());
		}
		else if (lineType == "f") /*face data*/
			faces.push_back(issLine);
		else if (lineType == "vn") /*face data*/
		{ 
			vertexNormals.push_back(vec3fFromStream(issLine));
			// vertex normals
		}
		else if (lineType == "#" || lineType == "")
		{
			// comment / empty line
		}
		else
		{
			cout<< "Found unknown line Type \"" << lineType << "\"";
		}
	}

	// scale models to match general size (if its way too big or way too small):
	if (2.0 > bounding_box[0].y || bounding_box[0].y > 6.0)
	{
		GLfloat scaleFactor = 4.0 / bounding_box[0].y;
		bounding_box[0] = vec3();
		bounding_box[1] = vec3();
		for (int i = 0; i < vertices.size(); i++)
		{
			vertices[i] = vertices[i] * scaleFactor;
			calculateBoundingBox(vertices[i]);
		}
	}

	// save number of points, 3 points per face
	vertexCount = faces.size() * 3;
	vertex_positions = new vec3[vertexCount];
	// iterate through all stored faces and create triangles

	int k = 0;
	for (int i = 0; i < faces.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			int index = faces[i].v[j] - 1;
			vertex_positions[k++] = vertices[index]; 
		}
	}

	faceNormals = new vec3[faces.size()];
	centerPoints = new vec3[faces.size()];
	// calculate center points & normals
	k = 0;
	for (int i = 0; i < vertexCount; i+=3)
	{
		vec3 p1 = vertex_positions[i];
		vec3 p2 = vertex_positions[i + 1];
		vec3 p3 = vertex_positions[i + 2];

		faceNormals[k] = normalize(cross(p2 - p1, p3 - p1));
		//cout << "normal: " << normalize(cross(p2 - p1, p3 - p1)) << endl;
		//cout << "center point: " << (p1 + p2 + p3) / 3.0 << endl;
		centerPoints[k] = (p1 + p2 + p3) / 3.0;
		k++;
	}

	//actualVertexCount = vertices.size();

	// calculate vertexNormals
	/*vertexNormals = new vec3[vertexCount];
	vertexCenter = new vec3[vertexCount];
	vec3 normal;
	for (int vertex = 0; vertex < vertices.size(); vertex++)
	{
		vertexCenter[vertex] = vertices[vertex];
		normal = vec3(0.0, 0.0, 0.0);

		for (int polygon = 0; polygon < faces.size(); polygon++)
		{
			vec3 p1 = vertex_positions[3 * polygon];
			vec3 p2 = vertex_positions[3 * polygon + 1];
			vec3 p3 = vertex_positions[3 * polygon + 2];

			cout << vertexCenter[vertex] << "    " << p1 << p2 << p3 << endl;
			if (vertex == 3 * polygon || vertex == 3 * polygon + 1 || vertex == 3 * polygon + 2)
			{
				cout << "y" << endl;
				normal += faceNormals[polygon];
			}
		}
		cout << "normal:" << normal << endl;
		vertexNormals[vertex] = normalize(normal);
	}*/
}

vec3 transformPoint(vec4 point, mat4 m, vec2 rendererDims)
{
	point = m * point;
	point = homo2noHomo(point);
	vec3 out = viewPort(rendererDims, point);
	return out;
}

void MeshModel::drawWorldAxis(Renderer* r, mat4& cTransform, mat4& projection)
{
	mat4 Mw = projection * cTransform * _world_transform;
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

void MeshModel::draw(Renderer* r, mat4& cTransform, mat4& projection, vec3& color)
{
	vec2 rendererDims = r->getDims();
	//mat4 M = projection * transpose(cTransform) * _world_t ransform * _model_transform;
	mat4 M = projection * cTransform * _world_transform * _model_transform;

	
	vector<vec3> triangles;
	for (int i = 0; i < vertexCount; i++)
	{
		vec3 point = vertex_positions[i];

		// convert point to homogeneous
		vec4 point4(point.x, point.y, point.z, 1);

		// multiply by model transformations
		point4 = M * point4;
		
		// convert to non-homogeneous
		vec4 nonHomogene = homo2noHomo(point4);

		// view port transform
		vec3 screenPoint = viewPort(rendererDims, nonHomogene);

		triangles.push_back(screenPoint);
	}

	r->DrawTriangles(triangles, triangles.size(), color);

	// origin point for debugging
	drawWorldAxis(r, cTransform, projection);
}

vec3 MeshModel::getPosition()
{
	vec3 pos;
	pos.x = _model_transform[0][3];
	pos.y = _model_transform[1][3];
	pos.z = _model_transform[2][3];
	return pos; // might need to homogene with [3][3]
}

void MeshModel::calculateBoundingBox(vec3 vertex)
{
	// positive direction
	bounding_box[0].x = max(bounding_box[0].x, vertex.x);
	bounding_box[0].y = max(bounding_box[0].y, vertex.y);
	bounding_box[0].z = max(bounding_box[0].z, vertex.z);
	// negative direction direction
	bounding_box[1].x = min(bounding_box[1].x, vertex.x);
	bounding_box[1].y = min(bounding_box[1].y, vertex.y);
	bounding_box[1].z = min(bounding_box[1].z, vertex.z);
}

void MeshModel::drawBoundingBox(Renderer* r, mat4& cTransform, mat4& projection)
{
	vec2 rendererDims = r->getDims();
	//mat4 M = projection * transpose(cTransform) * _world_t ransform * _model_transform;
	mat4 M = projection * cTransform * _world_transform * _model_transform;

	vec3 line[2];

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

}

void MeshModel::darwFaceNormals(Renderer* r, mat4& cTransform, mat4& projection)
{
	vec2 rendererDims = r->getDims();
	mat4 proj = projection * cTransform;
	mat4 transform = proj * _world_transform * _model_transform;
	mat4 normalTransform = _world_transform * _normal_transform;
	

	int faceCount = vertexCount / 3;
	for (int i = 0; i < faceCount; i++)
	{
		vec4 center = homo2noHomo(transform * vec4(centerPoints[i]));

		vec4 direction = vec4(faceNormals[i], 0); // might be 1 instead of 0
		direction = (normalTransform * direction);
		direction.w = 0;
		direction = normalize(direction);
		direction.w = 1;

		direction = proj * direction;
		direction = homo2noHomo(direction);

		vec4 dir = center + direction;
		
		vec3 point1 = viewPort(rendererDims, center);
		vec3 point2 = viewPort(rendererDims, dir);

		r->drawLine(point1.x, point1.y, point2.x, point2.y, vec3(1.0, 1.0, 0.0));

	}
}

void MeshModel::darwVertexNormals(Renderer* r, mat4& cTransform, mat4& projection)
{
	//cout << "asd" << endl;
	vec2 rendererDims = r->getDims();
	mat4 proj = projection * cTransform;
	mat4 transform = proj * _world_transform * _model_transform;
	mat4 normalTransform = _world_transform * _normal_transform;

	int vertices = actualVertexCount;
	//cout << vertices << endl;
	for (int i = 0; i < vertices; i++)
	{
		//cout << i << endl;
		vec4 center = homo2noHomo(transform * vec4(vertexCenter[i]));

		vec4 direction = vec4(vertexNormals[i], 0); // might be 1 instead of 0

		//cout << direction << endl;

		direction = (normalTransform * direction);
		direction.w = 0;
		direction = normalize(direction);
		direction.w = 1;

		direction = proj * direction;
		direction = homo2noHomo(direction);

		vec4 dir = center + direction;

		//cout << center << "here" << direction << endl;

		vec3 point1 = viewPort(rendererDims, center);
		vec3 point2 = viewPort(rendererDims, dir);

		//cout << point1 << "here" << point2 << endl;

		r->drawLine(point1.x, point1.y, point2.x, point2.y, vec3(1.0, 1.0, 0.0));

	}
}

void MeshModel::transformModel(const mat4& transform, bool scalling)
{
	// multiply by trans model from Left
	_model_transform = transform * _model_transform;
	
	// check if scaling
	mat4 normalScale;
	if (scalling)
	{
		normalScale = transform;
		normalScale[0][0] = 1 / normalScale[0][0];
		normalScale[1][1] = 1 / normalScale[1][1];
		normalScale[2][2] = 1 / normalScale[2][2];
	}
	_normal_transform = normalScale * _normal_transform;
}

void MeshModel::transformWorld(const mat4& transform)
{
	// multiply by trans model from Left
	_world_transform = transform * _world_transform;
}

PrimMeshModel::PrimMeshModel()
{
	vertex_positions = new vec3[36];
	vector<FaceIdcs> faces;
	vector<vec3> vertices;

	vertices.push_back(vec3(0.0, 0.0, 0.0));
	vertices.push_back(vec3(0.0, 0.0, 1.0));
	vertices.push_back(vec3(0.0, 1.0, 0.0));
	vertices.push_back(vec3(0.0, 1.0, 1.0));
	vertices.push_back(vec3(1.0, 0.0, 0.0));
	vertices.push_back(vec3(1.0, 0.0, 1.0));
	vertices.push_back(vec3(1.0, 1.0, 0.0));
	vertices.push_back(vec3(1.0, 1.0, 1.0));

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

	//...

}