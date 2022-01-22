#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <math.h>

using namespace std;
//todo: understand better
#define screen(temp) vec3((temp.x) * r->getDims().x, (temp.y) * r->getDims().y, 0.0)
#define viewPort(dims, a) vec3((dims.x / 2.0) * (a.x + 1), (dims.y / 2.0) * (a.y + 1), a.z)

#define ind(x, y, size) y * size + x
#define NOISE_SIZE 64

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
	showIndicators = true;
}

MeshModel::~MeshModel(void)
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(BUFFER_SIZE, buffers);
	if(mat.isTexturized)
		stbi_image_free(mat.textureImage.data);
	delete[] vertex_positions;
	delete[] vertexNormal_positions;
	delete[] vertexTexture_positions;
	delete[] faceNormals;
	delete[] centerPoints;

	delete[] tangents_positions;
	delete[] bitangents_positions;
	
}

void MeshModel::loadFile(string fileName)
{
	//loading file buffer
	ifstream ifile(fileName.c_str());
	//saving v, vn, vt, f
	vector<vec3> vertices_fromInput;
	vector<vec3> vertices_normals_fromInput;
	vector<vec2> vertices_texture_fromInput;
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
			vertices_fromInput.push_back(vec3fFromStream(issLine));
		}
		else if (lineType == "vn") /*face data*/
		{
			vertices_normals_fromInput.push_back(vec3fFromStream(issLine));
		}
		else if (lineType == "vt")
		{
			vertices_texture_fromInput.push_back(vec2fFromStream(issLine));
		}
		else if (lineType == "f") { /*face data*/
			faces.push_back(issLine);
		}
		else if (lineType == "#" || lineType == "")
		{
			// comment
		}
		else
		{
			cout << "Found unknown line Type \"" << lineType << "\"" << endl;
		}
	}

	// calculate bounding box
	calculateBoundingBox(vertices_fromInput);

	// scale models to match general size (if its way too big or way too small):
	bool scaleModels = true;
	if (scaleModels)
	{
		if (2.0 > bounding_box[0].y || bounding_box[0].y > 6.0)
		{
			GLfloat scaleFactor = 4.0 / bounding_box[0].y;
			bounding_box[0] = vec3();
			bounding_box[1] = vec3();
			for (int i = 0; i < vertices_fromInput.size(); i++)
			{
				vertices_fromInput[i] = vertices_fromInput[i] * scaleFactor;
			}
		}
		// calculate bounding box again
		calculateBoundingBox(vertices_fromInput);
	}

	//vertex count in triangle positions
	vertexCount = faces.size() * 3;

	// initializing model data
	vertex_positions = new vec3[vertexCount];
	vertexNormal_positions = new vec3[vertexCount];
	vertexTexture_positions = new vec2[vertexCount];

	// setup calculation for vertex normals
	faceCount = faces.size();
	vec3* vertexNormalsForCalc = new vec3[vertices_fromInput.size()];
	if (!vertices_normals_fromInput.empty())
	{
		for (int i = 0; i < faceCount; i++) {
			for (int j = 0; j < 3; j++) {
				vertexNormalsForCalc[faces[i].v[j] - 1] += vertices_normals_fromInput[faces[i].vn[j] - 1];
			}
		}
	}



	// iterate through all stored faces and create triangles
	bool printDebug = false;
	for (int i = 0, k = 0; i < faces.size(); i++)
	{
		if (printDebug) cout << i << ": ";
		for (int j = 0; j < 3; j++)
		{
			vertex_positions[k] = vertices_fromInput[faces[i].v[j] - 1];
			if (printDebug) cout << "v: " << vertex_positions[k] << " ";
			if (!vertices_normals_fromInput.empty())
			{
				vertexNormal_positions[k] = normalize(vertexNormalsForCalc[faces[i].v[j] - 1]);
				if (printDebug && false) cout << "vn: " << vertexNormal_positions[k] << " ";
			}
				
			if (!vertices_texture_fromInput.empty())
			{
				vertexTexture_positions[k] = vertices_texture_fromInput[faces[i].vt[j] - 1];
				if (printDebug) cout << "vt: " << vertexTexture_positions[k] << " ";
			}
			k++;
		}
		if (printDebug) cout << endl;
	}
	delete[] vertexNormalsForCalc;

	//calculate center points & normals
	vec3* faceNormsForCalc = new vec3[faceCount];
	centerPoints = new vec3[faceCount];
	for (int i = 0, k = 0; i < vertexCount; i += 3)
	{
		vec3 p1 = vertex_positions[i], p2 = vertex_positions[i + 1], p3 = vertex_positions[i + 2];
		faceNormsForCalc[k] = normalize(cross(p2 - p1, p3 - p1)); //normal according to formula
		centerPoints[k] = (p1 + p2 + p3) / 3.0;//center point according to formula
		k++;
	}

	faceNormals = new vec3[vertexCount];
	for (int i = 0; i < vertexCount; i++)
	{
		faceNormals[i] = faceNormsForCalc[i / 3];
	}
	

	// calculate vertex normals if none
	if (vertices_normals_fromInput.empty())
	{
		vec3* vertexNormalsForCalc = new vec3[vertices_fromInput.size()];
		for (int i = 0; i < vertices_fromInput.size(); i++)
		{
			vertexNormalsForCalc[i] = vec3(0.0f, 0.0f, 0.0f);
		}


		for (int i = 0; i < faceCount; i++)
		{
			vertexNormalsForCalc[faces[i].v[0] - 1] += faceNormsForCalc[i];
			vertexNormalsForCalc[faces[i].v[1] - 1] += faceNormsForCalc[i];
			vertexNormalsForCalc[faces[i].v[2] - 1] += faceNormsForCalc[i];
		}

		for (int i = 0, k = 0; i < faceCount; i++)
		{
			vertexNormal_positions[k++] = normalize(vertexNormalsForCalc[faces[i].v[0] - 1]);
			vertexNormal_positions[k++] = normalize(vertexNormalsForCalc[faces[i].v[1] - 1]);
			vertexNormal_positions[k++] = normalize(vertexNormalsForCalc[faces[i].v[2] - 1]);
			if (false) cout << "point: " << vertices_fromInput[faces[i].v[0] - 1] << " normal: " << normalize(vertexNormalsForCalc[faces[i].v[0] - 1]) << endl;
		}
		delete[] vertexNormalsForCalc;
	}
	delete[] faceNormsForCalc;
	
	tangents_positions = new vec3[vertexCount];
	bitangents_positions = new vec3[vertexCount];

	calculateTangents();

	// generate buffers and vao
	glGenVertexArrays(1, &vao);
	glGenBuffers(BUFFER_SIZE, buffers);

	bindData();
}

void MeshModel::bindData()
{
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]); // vertex positions
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertexCount, vertex_positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]); // face normals positions
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertexCount, faceNormals, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]); // vertex normals positions
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertexCount, vertexNormal_positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[3]); // vertex normals positions
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * vertexCount, vertexTexture_positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[4]); // tangents positions
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertexCount, tangents_positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[5]); // bitangents positions
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertexCount, bitangents_positions, GL_STATIC_DRAW);
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
	//r->drawLine((int)zeroS.x, (int)zeroS.y, (int)xS.x, (int)xS.y, vec3(1, 0, 0));

	vec4 yAxis = vec4(0, 1, 0, 1);
	yAxis = Mw * yAxis;
	yAxis = homo2noHomo(yAxis);
	vec3 yS = viewPort(rendererDims, yAxis);
	//r->drawLine((int)zeroS.x, (int)zeroS.y, (int)yS.x, (int)yS.y, vec3(0, 1, 0));

	vec4 zAxis = vec4(0, 0, 1, 1);
	zAxis = Mw * zAxis;
	zAxis = homo2noHomo(zAxis);
	vec3 zS = viewPort(rendererDims, zAxis);
	//r->drawLine((int)zeroS.x, (int)zeroS.y, (int)zS.x, (int)zS.y, vec3(0, 0, 1));
}

void MeshModel::draw(Renderer* r)
{
	mat4 normalTransform = _normal_world_transform * _normal_transform;
	mat4 worldModel = _world_transform * _model_transform;

	vaoData vData;
	vData.vao = vao;
	vData.buffers = buffers;
	vData.vertexPos = vertex_positions;
	vData.faceNormals = faceNormals;
	vData.vertexNormals = vertexNormal_positions;
	vData.vertexTexture = vertexTexture_positions;
	vData.tangents = tangents_positions;
	vData.bitangents = bitangents_positions;
	vData.size = vertexCount;

	r->DrawModel(vData, mat, worldModel, normalTransform);
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

	mat4 transform = projection * cTransform * _world_transform * _model_transform;

	r->glDrawLines(vertices, &vec4(1.0, 1.0, 0.0, 1.0), 24, transform, 3U, true);
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

	
	
}

void MeshModel::drawVertexNormals(Renderer* r, mat4& cTransform, mat4& projection)
{
	mat4 worldModel = _world_transform * _model_transform;
	mat4 normalMat = _normal_world_transform * _normal_transform;
	//r->drawNormals(vertex_positions, normal_positions, vertexCount, worldModel, normalMat);

	/*
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
		//r->drawLine(point1.x, point1.y, point2.x, point2.y, vec3(1.0, 0.0, 1.0));
	}*/
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
	return;
}

void MeshModel::loadTexture(std::string fileName, int texMode)
{
	Texture* newTex;
	bool* isTextured;
	switch (texMode)
	{
	case LOAD_TEX_COLOR:
		newTex = &mat.textureImage;
		isTextured = &mat.isTexturized;
		break;
	case LOAD_TEX_ENVIRONMENT:
		newTex = &mat.textureEnvironment;
		isTextured = &mat.isEnvironment;
		break;
	case LOAD_TEX_NORMAL:
		newTex = &mat.textureNormal;
		isTextured = &mat.isNormalMap;
		break;
	}

	if (*isTextured)
	{
		stbi_image_free(newTex->data);
	}
	*isTextured = true;
	stbi_set_flip_vertically_on_load(true);
	newTex->data = stbi_load(fileName.c_str(), &(newTex->width), &(newTex->height), &(newTex->nrChannels), 0);
	std::cout << "loaded texture " << fileName << " (" << newTex->width << ", " << newTex->height << ", " << newTex->nrChannels << ")" << endl;

	// generate texture for opengl
	glGenTextures(1, &(newTex->textureId));

	glBindTexture(GL_TEXTURE_2D, newTex->textureId);
	// TODO: channel check ?
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newTex->width, newTex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, newTex->data);

	glBindTexture(GL_TEXTURE_2D, newTex->textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	cout << "istext: " << mat.isTexturized << " isenv: " << mat.isEnvironment << " isnorm: " << mat.isNormalMap << endl;
}

void MeshModel::createNoiseTexture()
{
	Texture* newTex = &mat.textureNoise;
	bool* isTextured = &mat.isNoiseTexture;

	if (*isTextured)
	{
		delete[] newTex->data;
	}
	*isTextured = true;

	newTex->data = new unsigned char[NOISE_SIZE * NOISE_SIZE * 3];

	for (int y = 0; y < NOISE_SIZE; y ++)
		for (int x = 0; x < NOISE_SIZE; x++)
		{
			unsigned char value = rand() % 256;
			for (int c = 0; c < 3; c++)
			{
				newTex->data[3 * (y * NOISE_SIZE + x) + c] = value;
			}
		}
	
	glGenTextures(1, &(newTex->textureId));

	glBindTexture(GL_TEXTURE_2D, newTex->textureId);
	// TODO: channel check ?
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, NOISE_SIZE, NOISE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, newTex->data);

	glBindTexture(GL_TEXTURE_2D, newTex->textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

void MeshModel::calculateTangents()
{
	vec3 p1, p2, p3, nm;
	vec2 t1, t2, t3;
	vec3 edge1, edge2;
	vec2 delta1, delta2;

	for (int i = 0; i < vertexCount; i+=3)
	{
		p1 = vertex_positions[i];
		p2 = vertex_positions[i + 1];
		p3 = vertex_positions[i + 2];

		t1 = vertexTexture_positions[i];
		t2 = vertexTexture_positions[i + 1];
		t3 = vertexTexture_positions[i + 2];

		nm = faceNormals[i];

		// calc edges and deltas
		edge1 = p2 - p1;
		edge2 = p3 - p1;
		delta1 = t2 - t1;
		delta2 = t3 - t1;

		// calculate tangets, bitangents
		float f = 1.0f / (delta1.x * delta2.y - delta2.x * delta1.y);
		vec3 tangent, bitangent;
		
		tangent = (edge1 * delta2.y - edge2 * delta1.y) * f;
		bitangent = (edge2 * delta1.x - edge1 * delta2.x) * f;

		tangents_positions[i] = tangent;
		tangents_positions[i + 1] = tangent;
		tangents_positions[i + 2] = tangent;

		bitangents_positions[i] = bitangent;
		bitangents_positions[i + 1] = bitangent;
		bitangents_positions[i + 2] = bitangent;
	}
}