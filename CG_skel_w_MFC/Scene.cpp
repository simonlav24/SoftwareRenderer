#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>

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
	// clear buffer
	m_renderer->clearBuffer();

	if(showGrid)
		drawGrid();

	// for all models: draw yourself
	for (int i = 0; i < models.size(); i++)
	{
		vec3 color = i == activeModel ? vec3(1.0, 1.0, 1.0) : vec3(0.5, 0.5, 0.5);
		models[i]->draw(m_renderer, cameras[activeCamera]->cTransform, cameras[activeCamera]->projection, color);
		if (i == activeModel)
		{
			if(showBoundingBox)
				static_cast<MeshModel*>(models[i])->drawBoundingBox(m_renderer, cameras[activeCamera]->cTransform, cameras[activeCamera]->projection);
			if(showFaceNormals)
				static_cast<MeshModel*>(models[i])->darwFaceNormals(m_renderer, cameras[activeCamera]->cTransform, cameras[activeCamera]->projection);
			if(showVertexNormals)
				static_cast<MeshModel*>(models[i])->darwVertexNormals(m_renderer, cameras[activeCamera]->cTransform, cameras[activeCamera]->projection);
		}
	}

	// for all camreras: draw indicators
	mat4 m = cameras[activeCamera]->projection * cameras[activeCamera]->cTransform;
	for (int i = 0; i < cameras.size(); i++)
	{
		if (i == activeCamera)
			continue;
		vec4 camPos = viewPort(m_renderer->getDims(), homo2noHomo(m * cameras[i]->Eye));
		m_renderer->drawCameraIndicator(camPos);
	}

	
	drawOriginPoint();

	m_renderer->SwapBuffers();
}

void Scene::drawOriginPoint()
{
	// draw origin
	mat4 Mw = cameras[activeCamera]->projection * cameras[activeCamera]->cTransform;
	vec2 rendererDims = m_renderer->getDims();

	vec4 zero = vec4(0, 0, 0, 1);
	zero = Mw * zero;
	zero = homo2noHomo(zero);
	vec3 zeroS = viewPort(rendererDims, zero);

	vec4 xAxis = vec4(1, 0, 0, 1);
	xAxis = Mw * xAxis;
	xAxis = homo2noHomo(xAxis);
	vec3 xS = viewPort(rendererDims, xAxis);
	m_renderer->drawLine((int)zeroS.x, (int)zeroS.y, (int)xS.x, (int)xS.y, vec3(1, 0, 0));

	vec4 yAxis = vec4(0, 1, 0, 1);
	yAxis = Mw * yAxis;
	yAxis = homo2noHomo(yAxis);
	vec3 yS = viewPort(rendererDims, yAxis);
	m_renderer->drawLine((int)zeroS.x, (int)zeroS.y, (int)yS.x, (int)yS.y, vec3(0, 1, 0));

	vec4 zAxis = vec4(0, 0, 1, 1);
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
		return;
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
}

void Scene::transformActiveModel(const mat4& transform, bool scalling)
{
	if (activeModel == -1)
		return;
	switch (tState)
	{
	case model:
		models[activeModel]->transformModel(transform, scalling);
		break;
	case world:
		models[activeModel]->transformWorld(transform);
		break;
	case camera:
		cameras[activeCamera]->At = transform * cameras[activeCamera]->At;
		cameras[activeCamera]->LookAt(cameras[activeCamera]->Eye, cameras[activeCamera]->At, cameras[activeCamera]->Up);
		break;
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
}

void Scene::translateCamera(int dx, int dy)
{
	vec4 eye = cameras[activeCamera]->Eye;
	vec4 at = cameras[activeCamera]->At;
	vec4 up = cameras[activeCamera]->Up;

	vec4 moveDx = normalize(cross(at - eye, up));

	eye = Translate(moveDx * 0.1 * -dx) * eye;
	at = Translate(moveDx * 0.1 * -dx) * at;

	vec4 moveDy = normalize(cross(moveDx, up));

	eye = Translate(moveDy * 0.1 * -dy) * eye;
	at = Translate(moveDy * 0.1 * -dy) * at;

	
	cameras[activeCamera]->LookAt(homo2noHomo(eye), homo2noHomo(at), up);
}

void Scene::rotateZoomCamera(int dx, int dy, int scroll)
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
	//cout << up << endl;
	eye = RotateAroundAxis(-dx, up) * eye;
	eye = Translate(at) * eye;

	eye = homo2noHomo(eye);

	if (scroll == 1)
	{
		vec4 direction = normalize(eye);
		eye = Translate(-direction) * eye;
	}
	else if (scroll == -1)
	{
		vec4 direction = normalize(eye);
		eye = Translate(direction) * eye;
	}

	cameras[activeCamera]->LookAt(eye, at, up);
}

void Scene::lookAtModel()
{
	vec4 at = cameras[activeCamera]->At;
	vec3 pos = models[activeModel]->getPosition();
	cameras[activeCamera]->LookAt(cameras[activeCamera]->Eye, pos, cameras[activeCamera]->Up);
}

void Scene::resetCameraPosition()
{
	cameras[activeCamera]->LookAt(vec4(8, 8, -8.0, 1), vec4(0, 0, 0, 1), vec4(0, 1, 0, 1));
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
	//cout << "projection frustum" << endl;
	//cout << c << endl;
	this->projection = c;
}

void Camera::Ortho(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar)
{

	/*mat4 t = Translate(-(right + left) / 2, -(bottom + top) / 2, -(zNear + zFar) / 2);
	mat4 s = Scale(2 / (right - left), 2 / (top - bottom), 2 / -(zNear - zFar));
	this->projection = s * t;
	return;*/

	mat4 p;
	p[0][0] = 2 / (right - left);
	p[0][3] = -(left + right) / (right - left);
	p[1][1] = 2 / (top - bottom);
	p[1][3] = -(top + bottom) / (top - bottom);
	p[2][2] = -2 / (zFar - zNear);
	p[2][3] = -(zFar + zNear) / (zFar - zNear);
	p[3][3] = 1;

	/*cout << "projection ortho" << endl;
	cout << p << endl;*/
	this->projection = p;

}

void Camera::Perspective(const float fovy, const float aspect,
	const float zNear, const float zFar)
{
	float fovRad = 1.0 / tan(fovy * 0.5 / 180.0 * M_PI);
	mat4 a;
	a[0][0] = aspect * fovRad;
	a[1][1] = fovRad;
	a[2][2] = zFar / (zFar - zNear);
	a[3][2] = (-zFar * zNear) / (zFar - zNear);
	a[2][3] = 1.0;
	a[3][3] = 0.0;

	mat4 division;
	float alpha = -(zNear + zFar) / (zNear - zFar);
	float beta = -(2 * zNear * zFar) / (zNear - zFar);
	division[2][2] = alpha;
	division[2][3] = beta;
	division[3][2] = 1;
	division[3][3] = 0;

	this->projection = division * a;
}

void Scene::createPrimitive()
{
	PrimMeshModel* cube = new PrimMeshModel();
	this->models.push_back(cube);
	activeModel = models.size() - 1;
}