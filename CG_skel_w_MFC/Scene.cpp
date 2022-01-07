#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>
#include "GL\freeglut.h" // might not need (?)
#include "InitShader.h"
#include <chrono>

#define viewPort(dims, a) vec3((dims.x / 2.0) * (a.x + 1), (dims.y / 2.0) * (a.y + 1), a.z)

using namespace std;
void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	models.push_back(model);

	activeModel = models.size() - 1;
}

void Scene::draw()
{
	//===============Start
	// clear buffer
	m_renderer->clearBuffer();
	
	// try line
	//m_renderer->glDrawLine(cameras[activeCamera]->cTransform, cameras[activeCamera]->projection);
	
	m_renderer->drawOriginAxis(); // can hardcode the verties to save memory



	/*if(showGrid)
		drawGrid();*/

	// for all models: draw yourself
	for (int i = 0; i < models.size(); i++)
	{
		models[i]->draw(m_renderer);
		if (i == activeModel)
		{
			if(showBoundingBox)
				static_cast<MeshModel*>(models[i])->drawBoundingBox(m_renderer, cameras[activeCamera]->cTransform, cameras[activeCamera]->projection);
			if(showFaceNormals)
				static_cast<MeshModel*>(models[i])->drawFaceNormals(m_renderer, cameras[activeCamera]->cTransform, cameras[activeCamera]->projection);
			if (showVertexNormals)
				static_cast<MeshModel*>(models[i])->drawVertexNormals(m_renderer, cameras[activeCamera]->cTransform, cameras[activeCamera]->projection);
		}
	}
	

	// for all camreras: draw indicators
	if (showIndicators) {
		mat4 m = cameras[activeCamera]->projection * cameras[activeCamera]->cTransform;
		for (int i = 0; i < cameras.size(); i++)
		{
			if (i == activeCamera)
				continue;
			vec4 camPos = viewPort(m_renderer->getDims(), homo2noHomo(m * cameras[i]->Eye));
			m_renderer->drawPlusSign(camPos, vec3(1.0, 0.0, 0.0));
		}


		// for all lights: draw indicator
		for (int i = 0; i < lights.size(); i++)
		{
			vec4 lightPos = lights[i]->position;
			vec4 lightDir = lights[i]->position + 2.0 * lights[i]->direction;
			//vec4 lightPos = viewPort(m_renderer->getDims(), homo2noHomo(m * lights[i]->position));
			//vec4 lightDir = viewPort(m_renderer->getDims(), homo2noHomo(m * (lights[i]->position + 2.0 * lights[i]->direction)));
			if(lights[i]->lightType == point)
				m_renderer->drawLightIndicator(lightPos, lights[i]->color, vec3(0.0, 0.0, 0.0));
			else if (lights[i]->lightType == parallel)
				m_renderer->drawLightIndicator(lightPos, lights[i]->color, lightDir);
			else if (lights[i]->lightType == ambience)
				m_renderer->drawPlusSign(lightPos, lights[i]->color);
		}

		
		//drawOriginPoint();
	}
	// post proccessing
	
	//m_renderer->postProccess();
	
	m_renderer->SwapBuffers();
}

void Scene::toggleIndicators()
{
	showIndicators = !showIndicators;
	for (int i = 0; i < models.size(); i++)
	{
		models[i]->showIndicators = showIndicators;
	}
}

void Scene::setProjCam()
{
	mat4 ProjCam = cameras[activeCamera]->projection * cameras[activeCamera]->cTransform;
	m_renderer->lookAt = cameras[activeCamera]->cTransform;
	m_renderer->Proj = cameras[activeCamera]->projection;
	m_renderer->ProjCam = ProjCam;
	m_renderer->viewerPos[0] = cameras[activeCamera]->Eye;
	m_renderer->viewerPos[1] = cameras[activeCamera]->At;
	m_renderer->orthogonal = cameras[activeCamera]->orthogonal;
}

void Scene::drawOriginPoint()
{
	// draw origin
	GLfloat axisSize = 2.0;
	mat4 Mw = cameras[activeCamera]->projection * cameras[activeCamera]->cTransform;
	vec2 rendererDims = m_renderer->getDims();

	vec4 zero = vec4(0, 0, 0, 1);
	zero = Mw * zero;
	zero = homo2noHomo(zero);
	vec3 zeroS = viewPort(rendererDims, zero);

	vec4 xAxis = vec4(axisSize, 0, 0, 1);
	xAxis = Mw * xAxis;
	xAxis = homo2noHomo(xAxis);
	vec3 xS = viewPort(rendererDims, xAxis);
	m_renderer->drawLine((int)zeroS.x, (int)zeroS.y, (int)xS.x, (int)xS.y, vec3(1, 0, 0));

	vec4 yAxis = vec4(0, axisSize, 0, 1);
	yAxis = Mw * yAxis;
	yAxis = homo2noHomo(yAxis);
	vec3 yS = viewPort(rendererDims, yAxis);
	m_renderer->drawLine((int)zeroS.x, (int)zeroS.y, (int)yS.x, (int)yS.y, vec3(0, 1, 0));

	vec4 zAxis = vec4(0, 0, axisSize, 1);
	zAxis = Mw * zAxis;
	zAxis = homo2noHomo(zAxis);
	vec3 zS = viewPort(rendererDims, zAxis);
	m_renderer->drawLine((int)zeroS.x, (int)zeroS.y, (int)zS.x, (int)zS.y, vec3(0, 0, 1));
}

void Scene::drawGrid()
{
	if (grid.size() == 0)
	{
		GLfloat gridSize = 5.0;
		for (int i = -gridSize; i < gridSize+1; i++)
		{
			grid.push_back(vec4(i, 0, -gridSize, 1));
			grid.push_back(vec4(i, 0, gridSize, 1));
		}
		for (int i = -gridSize; i < gridSize+1; i++)
		{
			grid.push_back(vec4(-gridSize, 0, i, 1));
			grid.push_back(vec4(gridSize, 0, i, 1));
		}
	}
	mat4 Mw = cameras[activeCamera]->projection * cameras[activeCamera]->cTransform;
	vec2 rendererDims = m_renderer->getDims();

	for (int i = 0; i < grid.size(); i++)
	{
		vec4 point1 = grid[i];
		vec4 point2 = grid[i+1];
		//cout << point1 << " " << point2 << endl;
		i ++;
		point1 = viewPort(rendererDims, homo2noHomo(Mw * point1));
		point2 = viewPort(rendererDims, homo2noHomo(Mw * point2));

		m_renderer->drawLine(point1.x, point1.y, point2.x, point2.y, vec3(0.5, 0.5, 0.5));
	}
}

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
	m_renderer->SwapBuffers();
}

void Scene::switchActiveModel()
{
	if (models.size() == 0)
	{
		return;
	}

	activeModel = (activeModel + 1) % models.size();
}

void Scene::deleteActiveModel()
{
	if (activeModel == -1)
		return;
	models.erase(models.begin() + activeModel);
	switchActiveModel();
	if (models.size() == 0)
		activeModel = -1;
}

void Scene::switchActiveCamera()
{
	activeCamera = (activeCamera + 1) % cameras.size();
	setProjCam();
}

void Scene::transformActiveModel(const mat4& transform, bool scalling, bool rotation)
{
	switch (tState)
	{
	case model:
		if (activeModel == -1) return;
		models[activeModel]->transform(transform, false, scalling);
		break;
	case world:
		if (activeModel == -1) return;
		models[activeModel]->transform(transform, true, scalling);
		break;
	case camera:
		if (activeCamera == -1) return;
		cameras[activeCamera]->At = transform * cameras[activeCamera]->At;
		cameras[activeCamera]->LookAt(cameras[activeCamera]->Eye, cameras[activeCamera]->At, cameras[activeCamera]->Up);
		setProjCam();
		break;
	case light:
		if (activeLight == -1) return;
		lights[activeLight]->transformWorld(transform, rotation);
	}
}

Camera::Camera() : cTransform(), projection()
{
	// default camera properties
	LookAt(vec4(0.0, 0.0, -15.0, 1.0), vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0));
	//Frustum(-4.0, 4.0, -4.0, 4.0, 1.0, 100.0);
	Ortho(-5.0, 5.0, -5.0, 5.0, 5.0, 15.0);
}

Camera& Scene::currentCamera()
{
	return *cameras[activeCamera];
}

void Scene::addCamera()
{
	// add a default cam
	Camera *camera = new Camera();
	cameras.push_back(camera);
	activeCamera = cameras.size() - 1;
	setProjCam();
}

void Scene::translateCamera(int dx, int dy)
{
	vec4 eye = cameras[activeCamera]->Eye;
	vec4 at = cameras[activeCamera]->At;
	vec4 up = cameras[activeCamera]->Up;

	vec4 moveDx = normalize(cross(at - eye, up));

	eye = Translate(moveDx * 0.1 * -dx) * eye;
	at = Translate(moveDx * 0.1 * -dx) * at;

	vec4 moveDy = normalize(cross(moveDx, eye - at));

	eye = Translate(moveDy * 0.1 * -dy) * eye;
	at = Translate(moveDy * 0.1 * -dy) * at;

	cameras[activeCamera]->LookAt(homo2noHomo(eye), homo2noHomo(at), up);
	setProjCam();
}

void Scene::rotateZoomCamera(int dx, int dy, int scroll, GLfloat step)
{
	// move eye negative at
	vec4 eye = cameras[activeCamera]->Eye;
	vec4 at = cameras[activeCamera]->At;
	vec4 up = cameras[activeCamera]->Up;

	vec4 axis = normalize(cross(at - eye, up));
	eye = Translate(-at) * eye;
	eye = RotateAroundAxis(-dy, axis) * eye;
	eye = Translate(at) * eye;

	eye = homo2noHomo(eye);

	eye = Translate(-at) * eye;
	eye = RotateAroundAxis(-dx, up) * eye;
	eye = Translate(at) * eye;

	eye = homo2noHomo(eye);

	if (scroll == 1)
	{
		if (cameras[activeCamera]->orthogonal)
		{
			float left = cameras[activeCamera]->rectPos.x;
			float right = cameras[activeCamera]->rectPos.y;
			float bottom = cameras[activeCamera]->rectPos.z;
			float top = cameras[activeCamera]->rectPos.w;
			float znear = cameras[activeCamera]->zPos.x;
			float zfar = cameras[activeCamera]->zPos.y;
			cameras[activeCamera]->Ortho(left * 0.9, right * 0.9, bottom * 0.9, top * 0.9, znear, zfar);
			
		}
		else
		{
			vec4 direction = normalize(eye);
			eye = Translate(-direction * step) * eye;
		}
		
	}
	else if (scroll == -1)
	{
		if (cameras[activeCamera]->orthogonal)
		{
			float left = cameras[activeCamera]->rectPos.x;
			float right = cameras[activeCamera]->rectPos.y;
			float bottom = cameras[activeCamera]->rectPos.z;
			float top = cameras[activeCamera]->rectPos.w;
			float znear = cameras[activeCamera]->zPos.x;
			float zfar = cameras[activeCamera]->zPos.y;
			cameras[activeCamera]->Ortho(left * 1.1, right * 1.1, bottom * 1.1, top * 1.1, znear, zfar);
		}
		else
		{
			vec4 direction = normalize(eye);
			eye = Translate(direction) * eye;
		}
		
	}

	cameras[activeCamera]->LookAt(eye, at, up);
	setProjCam();
}

void Scene::lookAtModel()
{
	vec4 at = cameras[activeCamera]->At;
	vec3 pos = models[activeModel]->getPosition();
	cameras[activeCamera]->LookAt(cameras[activeCamera]->Eye, pos, cameras[activeCamera]->Up);
	setProjCam();
}

void Scene::resetCameraPosition()
{
	cameras[activeCamera]->LookAt(vec4(8, 8, -8.0, 1), vec4(0, 0, 0, 1), vec4(0, 1, 0, 1));
	setProjCam();
}

void Camera::LookAt(const vec4& eye, const vec4& at, const vec4& up)
{
	Eye = eye;
	At = at;
	Up = up;
	vec4 n = normalize(eye - at);
	n.w = 0.0;
	vec4 u = normalize(cross(up, n));
	u.w = 0.0;
	vec4 v = normalize(cross(n, u));
	v.w = 0.0;
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	mat4 c = mat4(u, v, n, t);

	this->cTransform = c*Translate(-eye);
}

void Camera::Frustum(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar)
{
	mat4 c;
	c[0][0] = (2 * zNear) / (right - left);
	c[0][3] = (right + left) / (right - left);
	c[1][1] = (2 * zNear) / (top - bottom);
	c[1][3] = (top + bottom) / (top - bottom);
	c[2][2] = -(zFar + zNear) / (zFar - zNear);
	c[2][3] = -(2 * zNear * zFar) / (zFar - zNear);
	c[3][2] = -1;
	c[3][3] = 0;
	this->projection = c;
	this->rectPos.x = left;
	this->rectPos.y = right;
	this->rectPos.z = bottom;
	this->rectPos.w = top;
	this->zPos.x = zNear;
	this->zPos.y = zFar;
	this->orthogonal = false;
}

void Camera::Ortho(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar)
{
	mat4 p;
	p[0][0] = 2 / (right - left);
	p[0][3] = -(left + right) / (right - left);
	p[1][1] = 2 / (top - bottom);
	p[1][3] = -(top + bottom) / (top - bottom);
	p[2][2] = -2 / (zFar - zNear);
	p[2][3] = -(zFar + zNear) / (zFar - zNear);
	p[3][3] = 1;

	this->projection = p;
	this->rectPos.x = left;
	this->rectPos.y = right;
	this->rectPos.z = bottom;
	this->rectPos.w = top;
	this->zPos.x = zNear;
	this->zPos.y = zFar;
	this->orthogonal = true;
}

void Camera::Perspective(const float fovy, const float aspect,
	const float zNear, const float zFar)
{
	float top = std::tan((fovy) / 2.0);
	top *= zNear;
	float bottom = -top;
	float right = top * aspect;
	float left = -right;
	Frustum(left, right, bottom, top, zNear, zFar);
}

void Scene::createPrimitive()
{
	PrimMeshModel* cube = new PrimMeshModel();
	this->models.push_back(cube);
	activeModel = models.size() - 1;
}

void Scene::reshapeCamera(int width, int height)
{
	float left = cameras[activeCamera]->rectPos.x;
	float right = cameras[activeCamera]->rectPos.y;
	float bottom = cameras[activeCamera]->rectPos.z;
	float top = cameras[activeCamera]->rectPos.w;

	float znear = cameras[activeCamera]->zPos.x;
	float zfar = cameras[activeCamera]->zPos.y;

	float vertical = abs(top - bottom);

	float newRatio = (float)width / (float)height;

	float horizon = newRatio * vertical;
	
	left = -horizon / 2.0f;
	right = horizon / 2.0f;

	if (cameras[activeCamera]->orthogonal)
		cameras[activeCamera]->Ortho(left, right, bottom, top, znear, zfar);
	else
		cameras[activeCamera]->Frustum(left, right, bottom, top, znear, zfar);
	setProjCam();
}

void Scene::addLight(LightType type)
{
	Light* light = new Light(type);

	lights.push_back(light);
	activeLight = lights.size() - 1;
}

void Scene::moveLight(vec3 pos)
{
	if (activeLight == -1)
		return;
	lights[activeLight]->position = pos;
}

void Scene::deleteActiveLight()
{
	if (activeLight == -1)
		return;
	lights.erase(lights.begin() + activeLight);
	switchActiveLight();
	if (lights.size() == 0)
		activeLight = -1;
}

void Scene::switchActiveLight()
{
	if (lights.size() == 0)
	{
		return;
	}
	activeLight = (activeLight + 1) % lights.size();
}

void Scene::changeLightColor(vec3 color)
{
	if (activeLight == -1)
		return;
	lights[activeLight]->color = color;
}

void Scene::changeLightPosition(vec3 pos)
{
	if (activeLight == -1)
		return;
	lights[activeLight]->position = pos;
}

void Scene::changeLightDirection(vec3 dir)
{
	if (activeLight == -1)
		return;
	lights[activeLight]->direction = dir;
}

void Scene::changeMaterial(materialProperty prop, vec3 values)
{
	if (activeModel == -1)
		return;
	switch (prop)
	{
	case materialProperty::color:
		static_cast<MeshModel*>(models[activeModel])->mat.color = values;
		break;
	case materialProperty::ambient:
		static_cast<MeshModel*>(models[activeModel])->mat.ambientColor = values;
		break;
	case materialProperty::diffuse:
		static_cast<MeshModel*>(models[activeModel])->mat.diffuseColor = values;
		break;
	case materialProperty::specular:
		static_cast<MeshModel*>(models[activeModel])->mat.specularColor = values;
		break;
	case materialProperty::shine:
		static_cast<MeshModel*>(models[activeModel])->mat.shininessCoeficient = values.x;
		break;
	case materialProperty::special:
		static_cast<MeshModel*>(models[activeModel])->mat.special = !static_cast<MeshModel*>(models[activeModel])->mat.special;
		break;
	case materialProperty::emission:
		static_cast<MeshModel*>(models[activeModel])->mat.emissiveColor = values;
		break;
	}
}

vec3 Scene::getMaterial(materialProperty prop)
{
	if (activeModel == -1)
		return vec3(-1, -1, -1);
	vec3 result;
	switch (prop)
	{
	case materialProperty::color:
		result = static_cast<MeshModel*>(models[activeModel])->mat.color;
		break;
	case materialProperty::ambient:
		result = static_cast<MeshModel*>(models[activeModel])->mat.ambientColor;
		break;
	case materialProperty::diffuse:
		result = static_cast<MeshModel*>(models[activeModel])->mat.diffuseColor;
		break;
	case materialProperty::specular:
		result = static_cast<MeshModel*>(models[activeModel])->mat.specularColor;
		break;
	case materialProperty::shine:
		result.x = static_cast<MeshModel*>(models[activeModel])->mat.shininessCoeficient;
		break;
	case materialProperty::emission:
		result = static_cast<MeshModel*>(models[activeModel])->mat.emissiveColor;
		break;
	}
	return result;
}