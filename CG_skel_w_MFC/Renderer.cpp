#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) ((x)+(y)*(width))*3+(c)
#define INDEX_ZB(width,x,y) ((x)+(y)*(width))
#define DEFAULT_DIMS 512

#define det(p2, p0, p1)  0.5*((p0.x-p2.x)*(p1.y-p2.y)-(p1.x-p2.x)*(p0.y-p2.y))
#define area(p1, p2, p3) abs(0.5*(p1.x*(p2.y-p3.y)+p2.x*(p3.y-p1.y)+p3.x*(p1.y-p2.y)))

#define viewPort(a) vec3((m_width / 2.0) * (a.x + 1), (m_height / 2.0) * (a.y + 1), a.z)

Renderer::Renderer() :m_width(DEFAULT_DIMS), m_height(DEFAULT_DIMS)
{
	InitOpenGLRendering();
	CreateBuffers(DEFAULT_DIMS, DEFAULT_DIMS);
	Init();
}
Renderer::Renderer(int width, int height) :m_width(width), m_height(height)
{
	InitOpenGLRendering();
	CreateBuffers(width,height);
	Init();
}

void Renderer::Init()
{
	ambientIntensity = 0.1;
	shadingSetup = Flat;
}

Renderer::~Renderer(void)
{
	DestroyBuffers();
}



void Renderer::CreateBuffers(int width, int height)
{
	m_width=width;
	m_height=height;	
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3*m_width*m_height];
	m_zbuffer = new float[m_width * m_height];
}

void Renderer::clearBuffer()
{
	GLfloat bgColor[2] = { 0.050 , 0.215 };
	GLfloat color;
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			color = (bgColor[1] - bgColor[0]) * ((GLfloat)y / (GLfloat)m_height) + bgColor[0];
			m_outBuffer[INDEX(m_width, x, y, 0)] = color; m_outBuffer[INDEX(m_width, x, y, 1)] = color; m_outBuffer[INDEX(m_width, x, y, 2)] = color;
			m_zbuffer[INDEX_ZB(m_width, x, y)] = 101;
		}
	}
}

void Renderer::DestroyBuffers()
{
	delete[] m_outBuffer;
	delete[] m_zbuffer;
}

void Renderer::SetDemoBuffer()
{
	//vertical line
	for(int i=0; i<m_width/3; i++)
	{
		m_outBuffer[INDEX(m_width,256,i,0)]=1;	m_outBuffer[INDEX(m_width,256,i,1)]=0;	m_outBuffer[INDEX(m_width,256,i,2)]=0;

	}
	//horizontal line
	for(int i=0; i<m_width/3; i++)
	{
		m_outBuffer[INDEX(m_width,i,256,0)]=1;	m_outBuffer[INDEX(m_width,i,256,1)]=0;	m_outBuffer[INDEX(m_width,i,256,2)]=1;

	}
}

// method for changing the color of a single pixel
void Renderer::drawPixel(int x, int y, const vec3& color)
{
	if (x < 0 || x > m_width - 1 || y < 0 || y > m_height - 1)
		return;
	m_outBuffer[INDEX(m_width, x, y, 0)] = color.x; m_outBuffer[INDEX(m_width, x, y, 1)] = color.y; m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
}

void Renderer::drawLine(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, const vec3& color)
{
	drawLine((int)x0, (int)y0, (int)x1, (int)y1, color);
}

void Renderer::drawLine(vec3 a, vec3 b, const vec3& color)
{
	if (a.x > m_width + border || a.x < -border || b.x > m_width + border || b.x < -border || a.y > m_height + border || a.y < -border || b.y > m_height + border || b.y < -border)
		return;
	vec3 start, end;
	bool steep;
	if (abs(b.y - a.y) < abs(b.x - a.x))
	{
		if (a.x < b.x)
		{
			start = a;
			end = b;
		}
		else
		{
			start = b;
			end = a;
		}
		steep = false;
	}
	else
	{
		if (a.y < b.y)
		{
			start = a;
			end = b;
		}
		else
		{
			start = b;
			end = a;
		}
		steep = true;
	}
	int dx = end.x - start.x;
	int dy = end.y - start.y;
	if (steep)
	{
		int xdir = 1;
		if (dx < 0)
		{
			xdir *= -1;
			dx *= -1;
		}
		int d = 2 * dx - dy;
		int x = start.x;
		for (int y = start.y; y < end.y; y++)
		{
			if (d > 0)
			{
				x += xdir;
				d += 2 * (dx - dy);
			}
			else
			{
				d += 2 * dx;
			}
			drawPixel(x, y, color);
		}
	}
	else
	{
		int ydir = 1;
		if (dy < 0)
		{
			ydir *= -1;
			dy *= -1;
		}
		int d = 2 * dy - dx;
		int y = start.y;
		for (int x = start.x; x < end.x; x++)
		{
			if (d > 0)
			{
				y += ydir;
				d += 2 * (dy - dx);
			}
			else
			{
				d += 2 * dy;
			}
			drawPixel(x, y, color);
		}
	}
}

void Renderer::drawLine(int x0, int y0, int x1, int y1, const vec3& color)
{
	drawLine(vec3(x0, y0, 0), vec3(x1, y1, 0), color);
}

void Renderer::DrawTriangles(const std::vector<vec3>& vertices, Material& mat, vector<vec4>& faceNormals)
{
	for (int i = 0; i < vertices.size(); i+=3)
	{
		//drawTriangleFlat(vertices[i + 0], vertices[i + 1], vertices[i + 2], color);
		//DrawTrianglePhong(vertices[i + 0], vertices[i + 1], vertices[i + 2], mat, faceNormals[i / 3]);

		drawTriangleWire(vertices[i + 0], vertices[i + 1], vertices[i + 2], mat.color);
	}
}

vec3 findCoeficients(vec3 point, vec3 p0, vec3 p1, vec3 p2)
{
	// find area using determinant
	GLfloat A = area(p0, p1, p2);
	GLfloat A0 = area(point, p1, p2);
	GLfloat A1 = area(point, p0, p2);
	GLfloat A2 = area(point, p0, p1);
	//GLfloat A2 = 1 - A0 - A1;
	//cout << A0 + A1 + A2 << " " << A << endl;

	return vec3(A0 / A, A1 / A, A2 / A);

}

float sdot(vec3 p1, vec3 p2, vec3 p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool isInside(vec3 pt, vec3 v1, vec3 v2, vec3 v3)
{
	float d1, d2, d3;
	bool has_neg, has_pos;

	d1 = sdot(pt, v1, v2);
	d2 = sdot(pt, v2, v3);
	d3 = sdot(pt, v3, v1);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

GLfloat Renderer::calculateAmbient(Material& mat)
{
	GLfloat Iambient = 0.0;
	for (int l = 0; l < sceneLights->size(); l++)
	{
		Iambient += max(0.0, mat.ambientCoeficient * sceneLights->at(l)->ambientIntensity);
	}
	return Iambient;
}

GLfloat Renderer::calculateDiffusion(vec3& pointInWorld, vec3& normalInWorld, Material& mat)
{
	GLfloat Idiffuse = 0.0;
	vec3 dirToLight;
	for (int l = 0; l < sceneLights->size(); l++)
	{
		dirToLight = sceneLights->at(l)->position - pointInWorld;
		GLfloat dotProd = dot(normalize(normalInWorld), normalize(dirToLight));
		Idiffuse += max(0.0, mat.diffuseCoeficient * dotProd * sceneLights->at(l)->diffuseIntensity);
	}
	return Idiffuse;
}

GLfloat Renderer::calculateSpecular(vec3& pointInWorld, vec3& normalInWorld, Material& mat)
{
	GLfloat Ispecular = 0.0;
	vec3 dirToLight, reflected;
	for (int l = 0; l < sceneLights->size(); l++)
	{
		dirToLight = normalize(pointInWorld - sceneLights->at(l)->position);
		//if (dot(dirToLight, normalInWorld) > 0)
		//	continue;
		reflected = dirToLight - 2.0 * dot(dirToLight, normalInWorld) * normalInWorld;
		vec3 dirToViewer = vec3(viewerPos.x, viewerPos.y, viewerPos.z) - pointInWorld;
		dirToViewer = dirToViewer * -1;
		GLfloat dotProd = dot(normalize(reflected), normalize(dirToViewer));
		dotProd = pow(dotProd, mat.shininessCoeficient);
		Ispecular += max(0.0, mat.specularCoeficient * dotProd * sceneLights->at(l)->specularIntensity);
	}
	return Ispecular;
}

void Renderer::drawTriangleFlat(vec3 p0, vec3 p1, vec3 p2, Material& mat)
{
	int yMax = max(p0.y, max(p1.y, p2.y));
	int yMin = min(p0.y, min(p1.y, p2.y));

	int xMax = max(p0.x, max(p1.x, p2.x));
	int xMin = min(p0.x, min(p1.x, p2.x));
	
	if (yMax >= m_height || yMin < 0 || xMax >= m_width || xMin < 0)
		return;

	vec3 c1(1.0, 0, 0);
	vec3 c2(0, 1.0, 0);
	vec3 c3(0, 0, 1.0);

	vec3 alpha;
	vec3 Color;

	GLfloat zValue;

	for (int y = yMin; y <= yMax; y++)
	{
		for (int x = xMin; x <= xMax; x++)
		{
			if (isInside(vec3(x, y, 0), p0, p1, p2))
			{
				alpha = findCoeficients(vec3(x, y, 0), p0, p1, p2);
				zValue = alpha.x * p0.z + alpha.y * p1.z + alpha.z * p2.z;
				//if(rand() > RAND_MAX - 100)
				//	cout << zValue << " " << m_zbuffer[INDEX_ZB(m_width, x, y)] << endl;
				vec3 Color = alpha.x * c1 + alpha.y * c2 + alpha.z * c3;
				if (zValue < m_zbuffer[INDEX_ZB(m_width, x, y)])
				{
					m_zbuffer[INDEX_ZB(m_width, x, y)] = zValue;
					drawPixel(x, y, Color);
				}

			}
		}
	}
}

void Renderer::drawModel(vector<vec4>& modelVertices, vector<vec4>& modelFaceNormals, vector<vec4>& modelVertexNormals, mat4& ProjCam, Material& mat)
{
	if (shadingSetup == WireFrame)
	{
		vector<vec3> triangles;
		for (int i = 0; i < modelVertices.size(); i++)
		{
			vec3 screenPoint = viewPort(homo2noHomo(ProjCam * modelVertices[i]));
			triangles.push_back(screenPoint);
		}
		drawTrianglesWire(triangles, mat);
		return;
	}

	else if (shadingSetup == Flat)
	{
		vec3 eyeInWorld = homo2noHomo(viewerPos);
		GLfloat Idiffuse = 0.0;
		GLfloat Itot = 0.0;

		for (int i = 0; i < modelVertices.size(); i++)
		{
			vec4 p0 = modelVertices[i];
			vec4 p1 = modelVertices[i + 1];
			vec4 p2 = modelVertices[i + 2];

			// calculate screen points
			vec3 sp0 = viewPort(homo2noHomo(ProjCam * p0));
			vec3 sp1 = viewPort(homo2noHomo(ProjCam * p1));
			vec3 sp2 = viewPort(homo2noHomo(ProjCam * p2));

			vec3 np0 = homo2noHomo(p0);
			vec3 np1 = homo2noHomo(p1);
			vec3 np2 = homo2noHomo(p2);

			int yMax = max(sp0.y, max(sp1.y, sp2.y));
			int yMin = min(sp0.y, min(sp1.y, sp2.y));

			int xMax = max(sp0.x, max(sp1.x, sp2.x));
			int xMin = min(sp0.x, min(sp1.x, sp2.x));

			if (yMax >= m_height || yMin < 0 || xMax >= m_width || xMin < 0)
			{
				i += 2;
				continue;
			}

			GLfloat zValue;
			vec3 alpha;
			vec3 pointInWorld;
			vec3 normalInWorld;
			vec3 Color;

			// find color
			// point in world
			pointInWorld = (np0 + np1 + np2)/3.0;
			// normal in world
			normalInWorld = vec3(modelFaceNormals[i].x, modelFaceNormals[i].y, modelFaceNormals[i].z);
			// lights in world
			//Idiffuse = calculateDiffusion(pointInWorld, normalInWorld, mat);
			Idiffuse = calculateSpecular(pointInWorld, normalInWorld, mat);
			
			Itot = Idiffuse;
			Color = mat.color * Idiffuse;

			for (int y = yMin; y <= yMax; y++)
			{
				for (int x = xMin; x <= xMax; x++)
				{
					if (isInside(vec3(x, y, 0), sp0, sp1, sp2))
					{
						alpha = findCoeficients(vec3(x, y, 0), sp0, sp1, sp2);
						zValue = alpha.x * sp0.z + alpha.y * sp1.z + alpha.z * sp2.z;
						if (zValue < m_zbuffer[INDEX_ZB(m_width, x, y)])
						{
							m_zbuffer[INDEX_ZB(m_width, x, y)] = zValue;
							drawPixel(x, y, Color);
						}

					}
				}
			}
			i += 2;
		}
	}

	else if (shadingSetup == Gouraud)
	{
		vec3 eyeInWorld = homo2noHomo(viewerPos);
		GLfloat Iambient = 0.0;
		GLfloat Idiffuse = 0.0;
		GLfloat Ispeculat = 0.0;
		GLfloat Itot = 0.0;

		for (int i = 0; i < modelVertices.size(); i++)
		{
			vec4 p0 = modelVertices[i];
			vec4 p1 = modelVertices[i + 1];
			vec4 p2 = modelVertices[i + 2];

			// calculate screen points
			vec3 sp0 = viewPort(homo2noHomo(ProjCam * p0));
			vec3 sp1 = viewPort(homo2noHomo(ProjCam * p1));
			vec3 sp2 = viewPort(homo2noHomo(ProjCam * p2));

			vec3 np0 = homo2noHomo(p0);
			vec3 np1 = homo2noHomo(p1);
			vec3 np2 = homo2noHomo(p2);

			int yMax = max(sp0.y, max(sp1.y, sp2.y));
			int yMin = min(sp0.y, min(sp1.y, sp2.y));

			int xMax = max(sp0.x, max(sp1.x, sp2.x));
			int xMin = min(sp0.x, min(sp1.x, sp2.x));

			if (yMax >= m_height || yMin < 0 || xMax >= m_width || xMin < 0)
			{
				i += 2;
				continue;
			}

			GLfloat zValue;
			vec3 alpha;
			vec3 pointInWorld;
			vec3 normalInWorld;
			vec3 Color;

			vec3 Colors[3];
			// find colors of points around
			for (int c = 0; c < 3; c++)
			{
				pointInWorld = homo2noHomo(modelVertices[i + c]);
				normalInWorld = vec3(modelVertexNormals[i + c].x, modelVertexNormals[i + c].y, modelVertexNormals[i + c].z);
				GLfloat Iambient = 0*calculateAmbient(mat);
				GLfloat Idiffuse = 0*calculateDiffusion(pointInWorld, normalInWorld, mat);
				GLfloat Ispecular = calculateSpecular(pointInWorld, normalInWorld, mat);
				//cout << "light: " << Iambient <<" "<< Idiffuse <<" "<< Ispecular << endl;
				Colors[c] = mat.color * (Iambient + Idiffuse + Ispecular);
			}

			for (int y = yMin; y <= yMax; y++)
			{
				for (int x = xMin; x <= xMax; x++)
				{
					if (isInside(vec3(x, y, 0), sp0, sp1, sp2))
					{
						alpha = findCoeficients(vec3(x, y, 0), sp0, sp1, sp2);
						zValue = alpha.x * sp0.z + alpha.y * sp1.z + alpha.z * sp2.z;

						Color = alpha.x * Colors[0] + alpha.y * Colors[1] + alpha.z * Colors[2];

						if (zValue < m_zbuffer[INDEX_ZB(m_width, x, y)])
						{
							m_zbuffer[INDEX_ZB(m_width, x, y)] = zValue;
							drawPixel(x, y, Color);
						}

					}
				}
			}
			i += 2;
		}
	}

	else if (shadingSetup == Phong)
	{
		vec3 eyeInWorld = homo2noHomo(viewerPos);
		GLfloat Iambient = 0.0;
		GLfloat Idiffuse = 0.0;
		GLfloat Ispeculat = 0.0;
		GLfloat Itot = 0.0;

		for (int i = 0; i < modelVertices.size(); i++)
		{
			vec4 p0 = modelVertices[i];
			vec4 p1 = modelVertices[i + 1];
			vec4 p2 = modelVertices[i + 2];

			// calculate screen points
			vec3 sp0 = viewPort(homo2noHomo(ProjCam * p0));
			vec3 sp1 = viewPort(homo2noHomo(ProjCam * p1));
			vec3 sp2 = viewPort(homo2noHomo(ProjCam * p2));

			vec3 np0 = homo2noHomo(p0);
			vec3 np1 = homo2noHomo(p1);
			vec3 np2 = homo2noHomo(p2);

			int yMax = max(sp0.y, max(sp1.y, sp2.y));
			int yMin = min(sp0.y, min(sp1.y, sp2.y));

			int xMax = max(sp0.x, max(sp1.x, sp2.x));
			int xMin = min(sp0.x, min(sp1.x, sp2.x));

			if (yMax >= m_height || yMin < 0 || xMax >= m_width || xMin < 0)
			{
				i += 2;
				continue;
			}

			GLfloat zValue;
			vec3 alpha;
			vec3 pointInWorld;
			vec3 normalInWorld;
			vec3 Color;

			vec3 normals[3];
			for (int n = 0; n < 3; n++)
			{
				normals[n] = vec3(modelVertexNormals[i + n].x, modelVertexNormals[i + n].y, modelVertexNormals[i + n].z);
			}

			for (int y = yMin; y <= yMax; y++)
			{
				for (int x = xMin; x <= xMax; x++)
				{
					if (isInside(vec3(x, y, 0), sp0, sp1, sp2))
					{
						alpha = findCoeficients(vec3(x, y, 0), sp0, sp1, sp2);
						zValue = alpha.x * sp0.z + alpha.y * sp1.z + alpha.z * sp2.z;

						pointInWorld = alpha.x * np0 + alpha.y * np1 + alpha.z * np2;
						normalInWorld = alpha.x * normals[0] + alpha.y * normals[1] + alpha.z * normals[2];

						GLfloat Iambient = calculateAmbient(mat);
						GLfloat Idiffuse = calculateDiffusion(pointInWorld, normalInWorld, mat);
						GLfloat Ispecular = calculateSpecular(pointInWorld, normalInWorld, mat);

						Color = mat.color * (Iambient + Idiffuse + 0*Ispecular);

						if (zValue < m_zbuffer[INDEX_ZB(m_width, x, y)])
						{
							m_zbuffer[INDEX_ZB(m_width, x, y)] = zValue;
							drawPixel(x, y, Color);
						}

					}
				}
			}
			i += 2;
		}
	}
}

void Renderer::DrawTrianglePhong(vec3 p0, vec3 p1, vec3 p2, Material& mat, vec4& faceNormal)
{
	int yMax = max(p0.y, max(p1.y, p2.y));
	int yMin = min(p0.y, min(p1.y, p2.y));

	int xMax = max(p0.x, max(p1.x, p2.x));
	int xMin = min(p0.x, min(p1.x, p2.x));

	if (yMax >= m_height || yMin < 0 || xMax >= m_width || xMin < 0)
		return;

	vec3 c1(1.0, 0, 0);
	vec3 c2(0, 1.0, 0);
	vec3 c3(0, 0, 1.0);

	vec3 alpha;
	vec3 Color;

	GLfloat zValue;

	for (int y = yMin; y <= yMax; y++)
	{
		for (int x = xMin; x <= xMax; x++)
		{
			if (isInside(vec3(x, y, 0), p0, p1, p2))
			{
				alpha = findCoeficients(vec3(x, y, 0), p0, p1, p2);
				zValue = alpha.x * p0.z + alpha.y * p1.z + alpha.z * p2.z;
				
				if (zValue > m_zbuffer[INDEX_ZB(m_width, x, y)])
					continue;
				/*vec4 point = vec4(x, y, zValue, 0);
				viewerPos.w = 0;
				vec4 dirToViewer = viewerPos - point;
				
				// ambient
				GLfloat Ia = mat.ambientCoeficient * ambientIntensity; //--------------> L_a (ambient light intensity) needs a place to be

				// diffuse
				GLfloat Id = 0.0;
				for (int i = 0; i < sceneLights->size(); i++)
				{
					vec4 lightPos = vec4(sceneLights->at(i)->position); // multiply by camera and projection and screen view port
					lightPos = camProj * lightPos;
					lightPos = homo2noHomo(lightPos);
					lightPos = viewPort(lightPos);
					lightPos.w = 0;
					// light pos is correct on screen

					vec3 light = vec3(lightPos.x, lightPos.y, lightPos.z);
					vec3 point3 = vec3(point.x, point.y, point.z);
					vec3 n = normalize(cross(p2 - p0, p1 - p0));
					//drawPixel(point3.x, point3.y, vec3(1.0, 0.0, 0.0));
					//drawLine(point3, point3 + n, vec3(1.0, 0.0, 0.0));
					//SwapBuffers();
					vec3 dirToLight3 = light - point3;
					vec4 dirToLight = lightPos - point;
					faceNormal.w = 0;

					
					vec4 ngot = normalize(faceNormal);

					GLfloat dotProd = max(dot(normalize(dirToLight3), n), 0.0);
					//cout << dotProd << endl;
					Id += mat.diffuseCoeficient * dotProd * 100.0;
				}
				





				Color = mat.color * Id;*/
				
				m_zbuffer[INDEX_ZB(m_width, x, y)] = zValue;
				drawPixel(x, y, Color);
				

			}
		}
	}
}
void Renderer::drawTrianglesWire(const std::vector<vec3>& vertices, Material& mat)
{
	for (int i = 0; i < vertices.size(); i += 3)
	{
		drawTriangleWire(vertices[i + 0], vertices[i + 1], vertices[i + 2], mat.color);
	}
}
void Renderer::drawTriangleWire(vec3 p0, vec3 p1, vec3 p2, const vec3& color)
{
	drawLine(p0, p1, color);
	drawLine(p1, p2, color);
	drawLine(p2, p0, color);
}

vec2 Renderer::getDims()
{
	return vec2(m_width, m_height);
}

void Renderer::drawPlusSign(vec4 pos, vec3 color)
{
	//vec3 color = vec3(1.0, 0.0, 0.0);
	drawLine((int)pos.x, (int)pos.y - 2, (int)pos.x, (int)pos.y + 4, color);
	drawLine((int)pos.x + 1, (int)pos.y - 2, (int)pos.x + 1, (int)pos.y + 4, color);

	drawLine((int)pos.x - 2, (int)pos.y, (int)pos.x + 4, (int)pos.y, color);
	drawLine((int)pos.x - 2, (int)pos.y + 1, (int)pos.x + 4, (int)pos.y + 1, color);
}

void Renderer::drawLightIndicator(vec4 pos, vec3 color)
{
	GLfloat size = 15.0;
	GLfloat size2 = 10.6;
	vec3 Pos(pos.x, pos.y, pos.z);
	drawLine(Pos + vec3(-size, 0.0, 0.0), Pos + vec3(size, 0.0, 0.0), color);
	drawLine(Pos + vec3(0.0, -size, 0.0), Pos + vec3(0.0, size, 0.0), color);
	drawLine(Pos + vec3(-size2, -size2, 0.0), Pos + vec3(size2, size2, 0.0), color);
	drawLine(Pos + vec3(-size2, size2, 0.0), Pos + vec3(size2, -size2, 0.0), color);
}

void Renderer::reshape(int width, int height)
{
	DestroyBuffers();
	CreateBuffers(width, height);
}

/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[]={
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[]={
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1};
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc)+sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );
	GLint  vPosition = glGetAttribLocation( program, "vPosition" );

	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0 );

	GLint  vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid *) sizeof(vtc) );
	glProgramUniform1i( program, glGetUniformLocation(program, "texture"), 0 );
	a = glGetError();
}

void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}

void Renderer::SwapBuffers()
{

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glutSwapBuffers();
	a = glGetError();
}