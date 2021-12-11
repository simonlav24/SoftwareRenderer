// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include <string>
#include "InputDialog.h"
#include "MeshModel.h"
#include "menus.h"

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

Scene* scene;
Renderer* renderer;

int last_x, last_y;
bool lb_down, rb_down, mb_down;

// more stuff
int framesPerSecond{ 60 };
GLfloat step{ 0.5 };

//----------------------------------------------------------------------------
// Callbacks
static int t = 0;
void display(void)
{
	if (scene->activeModel != -1)
	{
		//static_cast<MeshModel*>(scene->models[scene->activeModel])->mat.shininessCoeficient = 2.5 * sin(0.1 * t) + 2.5;
		//cout << static_cast<MeshModel*>(scene->models[scene->activeModel])->mat.shininessCoeficient << endl;
	}
	if (scene->activeCamera != -1)
	{
	}
	if (t == 20)
	{
		//cout << "reshape" << endl;
		//renderer->reshape(512 * 2, 512 * 2);
		//scene->reshapeCamera(512 * 2, 512 * 2);
	}
	t += 1;
	// Call the scene and ask it to draw itself
	scene->draw();
}

void reshape(int width, int height)
{
	if (renderer->SSAA)
	{
		width *= 2;
		height *= 2;
	}
		
	renderer->reshape(width, height);
	scene->reshapeCamera(width, height);
}

void keyboard(unsigned char key, int x, int y)
{
	GLfloat posStep = exp(3.21 * (step - 0.5));
	GLfloat scaleUpStep = 1.2 * step + 1.01 * (1.0 - step);
	GLfloat scaleDownStep = 0.8 * step + 0.99 * (1.0 - step);
	GLfloat rotStep = 30.0 * step + 1 * (1.0 - step);

	switch (key) {
	case 033:
		exit(EXIT_SUCCESS);
		break;
	case 'd':
		switch (scene->tMode)
		{
		case position:
			scene->transformActiveModel(Translate(vec3(-posStep, 0.0, 0.0)));
			break;
		case scale:
			scene->transformActiveModel(Scale(vec3(scaleUpStep, 1.0, 1.0)), true);
			break;
		case rotation:
			scene->transformActiveModel(RotateX(-rotStep), false, true);
			break;
		}

		break;
	case 'a':
		switch (scene->tMode)
		{
		case position:
			scene->transformActiveModel(Translate(vec3(posStep, 0.0, 0.0)));
			break;
		case scale:
			scene->transformActiveModel(Scale(vec3(scaleDownStep, 1.0, 1.0)), true);
			break;
		case rotation:
			scene->transformActiveModel(RotateX(rotStep), false, true);
			break;
		}
		break;

	case 'w':
		switch (scene->tMode)
		{
		case position:
			scene->transformActiveModel(Translate(vec3(0.0, 0.0, posStep)));
			break;
		case scale:
			scene->transformActiveModel(Scale(vec3(1.0, 1.0, scaleDownStep)), true);
			break;
		case rotation:
			scene->transformActiveModel(RotateZ(rotStep), false, true);
			break;
		}
		break;

	case 's':
		switch (scene->tMode)
		{
		case position:
			scene->transformActiveModel(Translate(vec3(0.0, 0.0, -posStep)));
			break;
		case scale:
			scene->transformActiveModel(Scale(vec3(1.0, 1.0, scaleUpStep)), true);
			break;
		case rotation:
			scene->transformActiveModel(RotateZ(-rotStep), false, true);
			break;
		}
		break;

	case 'q':
		switch (scene->tMode)
		{
		case position:
			scene->transformActiveModel(Translate(vec3(0.0, posStep, 0.0)));
			break;
		case scale:
			scene->transformActiveModel(Scale(vec3(1.0, scaleUpStep, 1.0)), true);
			break;
		case rotation:
			scene->transformActiveModel(RotateY(rotStep), false, true);
			break;
		}
		break;

	case 'e':
	case 'z':
		switch (scene->tMode)
		{
		case position:
			scene->transformActiveModel(Translate(vec3(0.0, -posStep, 0.0)));
			break;
		case scale:
			scene->transformActiveModel(Scale(vec3(1.0, scaleDownStep, 1.0)), true);
			break;
		case rotation:
			scene->transformActiveModel(RotateY(-rotStep), false, true);
			break;
		}
		break;

	case '+':
		step = min(1.0, step + 0.1);
		cout << "incremental step size = " << step << endl;
		break;
	case '-':
		step = max(0.0, step - 0.1);
		cout << "incremental step size = " << step << endl;
		break;

	case '1':
		scene->tMode = position;
		break;
	case '2':
		scene->tMode = scale;
		break;
	case '3':
		scene->tMode = rotation;
		break;
	
	case '7':
		renderer->fogMaxdist += step * 0.25;
		cout << "fog far point = " << renderer->fogMaxdist << endl;
		break;
	case '8':
		renderer->fogMaxdist -= step * 0.25;
		cout << "fog far point = " << renderer->fogMaxdist << endl;
		break;
	case '9':
		renderer->fogMindist += step * 0.25;
		cout << "fog near point = " << renderer->fogMindist << endl;
		break;
	case '0':
		renderer->fogMindist -= step * 0.25;
		cout << "fog near point = " << renderer->fogMindist << endl;
		break;

	case 'm':
		scene->switchActiveModel();
		break;
	case 'n':
		scene->showFaceNormals = !scene->showFaceNormals;
		break;
	case 'v':
		scene->showVertexNormals = !scene->showVertexNormals;
		break;
	case 'g':
		scene->showGrid = !scene->showGrid;
		break;
	case 'b':
		scene->showBoundingBox = !scene->showBoundingBox;
		break;
	case '[':
		static_cast<MeshModel*>(scene->models[scene->activeModel])->mat.shininessCoeficient -= 1;
		break;
	case ']':
		static_cast<MeshModel*>(scene->models[scene->activeModel])->mat.shininessCoeficient += 1;
		break;
	case 'l':
		renderer->SSAA = !renderer->SSAA;
		if (renderer->SSAA)
		{
			renderer->reshape(renderer->getDims().x * 2, renderer->getDims().y * 2);
			scene->reshapeCamera(renderer->getDims().x * 2, renderer->getDims().y * 2);
		}
		else
		{
			renderer->reshape(renderer->getDims().x / 2, renderer->getDims().y / 2);
			scene->reshapeCamera(renderer->getDims().x / 2, renderer->getDims().y / 2);
		}
		
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	if (button == 3)
		scene->rotateZoomCamera(0, 0, 1, step);
	if (button == 4)
		scene->rotateZoomCamera(0, 0, -1, step);
	//set down flags
	switch (button) {
	case GLUT_LEFT_BUTTON:
		lb_down = (state == GLUT_UP) ? 0 : 1;
		break;
	case GLUT_RIGHT_BUTTON:
		rb_down = (state == GLUT_UP) ? 0 : 1;
		break;
	case GLUT_MIDDLE_BUTTON:
		mb_down = (state == GLUT_UP) ? 0 : 1;
		break;
	}

	// add your code
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx = x - last_x;
	int dy = y - last_y;
	// update last x,y
	last_x = x;
	last_y = y;
	if (abs(dx) < 20 && abs(dy) < 20)
	{
		if (lb_down)
		{
			scene->rotateZoomCamera(dx, dy, 0, step);
			last_x = last_y = dx = dy = 0;
		}
		else if (mb_down)
		{
			scene->translateCamera(dx, dy);
			last_x = last_y = dx = dy = 0;
		}
	}

	//cout << dx << " " << dy << endl;
}

void timer(int id)
{
	glutPostRedisplay();
	glutTimerFunc(1000 / framesPerSecond, timer, 0);
}

// menu initialization function
void initMenu()
{
	int menuModel = glutCreateMenu(modelMenu);
	glutAddMenuEntry("Open obj file", MODEL_OPEN);
	glutAddMenuEntry("Load Primitive Cube", MODEL_CUBE);
	glutAddMenuEntry("Switch Model", MODEL_SWITCH);
	glutAddMenuEntry("Move Model", MODEL_MOVE);
	glutAddMenuEntry("Delete Model", MODEL_DELETE);

	int menuFrame = glutCreateMenu(frameStateMenu);
	glutAddMenuEntry("Model", TRANSFORM_MODEL);
	glutAddMenuEntry("World", TRANSFORM_WORLD);
	glutAddMenuEntry("Camera", TRANSFORM_CAMERA);
	glutAddMenuEntry("Light", TRANSFORM_LIGHT);

	int menuShow = glutCreateMenu(showMenu);
	glutAddMenuEntry("Toggle Bounding Box", SHOW_TOGGLE_BOUNDING_BOX);
	glutAddMenuEntry("Toggle Face Normals", SHOW_TOGGLE_FACE_NORMALS);
	glutAddMenuEntry("Toggle Vertex Normals", SHOW_TOGGLE_VERTEX_NORMALS);
	glutAddMenuEntry("Toggle Grid", SHOW_TOGGLE_GRID);
	glutAddMenuEntry("Toggle Indicators", SHOW_TOGGLE_INDICATORS);

	int menuMode = glutCreateMenu(modeStateMenu);
	glutAddMenuEntry("Position", MODE_POSITION);
	glutAddMenuEntry("Scale", MODE_SCALE);
	glutAddMenuEntry("Rotation", MODE_ROTATION);

	int menuCamera = glutCreateMenu(cameraMenu);
	glutAddMenuEntry("Add Camera (Orthogonal)", CAMERA_ADD_ORTHO);
	glutAddMenuEntry("Add Camera (Perspective)", CAMERA_ADD_PERSP);
	glutAddMenuEntry("Switch Camera", CAMERA_SWITCH);
	glutAddMenuEntry("Look At Active Model", CAMERA_LOOK_MODEL);
	glutAddMenuEntry("Edit Projection Orthogonal", CAMERA_EDIT_ORTHO);
	glutAddMenuEntry("Edit Projection Frustum", CAMERA_EDIT_FRUS);
	glutAddMenuEntry("Edit Projection Perspective", CAMERA_EDIT_FOVY);
	glutAddMenuEntry("Reset Camera Position", CAMERA_RESET);

	int menuLight = glutCreateMenu(lightMenu);
	glutAddMenuEntry("Add Point Light", LIGHT_ADD_POINT);
	glutAddMenuEntry("Add Parallel Light", LIGHT_ADD_PARALLEL);
	glutAddMenuEntry("Add ambient Light", LIGHT_ADD_AMBIENT);
	glutAddMenuEntry("Change Position", LIGHT_CHANGE_POSITION);
	glutAddMenuEntry("Change Direction", LIGHT_CHANGE_DIRECTION);
	glutAddMenuEntry("Change Color", LIGHT_CHANGE_COLOR);
	glutAddMenuEntry("Switch Light", LIGHT_SWITCH);
	glutAddMenuEntry("Delete Light", LIGHT_DELETE);

	int menuLightSetup = glutCreateMenu(shadingSetupMenu);
	glutAddMenuEntry("WireFrame", SHADING_WIRE);
	glutAddMenuEntry("Flat Shading", SHADING_FLAT);
	glutAddMenuEntry("Phong Shading", SHADING_PHONG);
	glutAddMenuEntry("Gouraud Shading", SHADING_GOURAUD);

	int menuMaterial = glutCreateMenu(materialMenu);
	glutAddMenuEntry("Edit Color", MATERIAL_CHANGE_COLOR);
	glutAddMenuEntry("Edit Ambient Color", MATERIAL_CHANGE_AMBIENT);
	glutAddMenuEntry("Edit Diffuse Color", MATERIAL_CHANGE_DIFFUSE);
	glutAddMenuEntry("Edit Specular Color", MATERIAL_CHANGE_SPECULAR);
	glutAddMenuEntry("Edit Shininess Coeficient", MATERIAL_CHANGE_SHININESS);
	glutAddMenuEntry("Toggle Special Material", MATERIAL_SPECIAL);

	int menuPost = glutCreateMenu(postProccessMenu);
	glutAddMenuEntry("Toggle Fog", POST_FOG_TOGGLE);
	glutAddMenuEntry("Change Fog Color", POST_FOG_COLOR);
	glutAddMenuEntry("Toggle SSAA", POST_SSAA_TOGGLE);

	glutCreateMenu(mainMenu);
	glutAddSubMenu("Model", menuModel);
	glutAddSubMenu("Material", menuMaterial);
	glutAddSubMenu("Switch Frame", menuFrame);
	glutAddSubMenu("Transformation Mode", menuMode);
	glutAddSubMenu("Camera", menuCamera);
	glutAddSubMenu("Light", menuLight);
	glutAddSubMenu("Shading mode", menuLightSetup);
	glutAddSubMenu("Show", menuShow);
	glutAddSubMenu("Post Proccessing", menuPost);
	glutAddMenuEntry("About", MAIN_ABOUT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------

int my_main(int argc, char** argv)
{
	//----------------------------------------------------------------------------
	// Initialize window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("CG");
	glewExperimental = GL_TRUE;
	glewInit();
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		/*		...*/
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	renderer = new Renderer(512, 512);
	scene = new Scene(renderer);

	scene->addCamera();
	scene->currentCamera().LookAt(vec4(8, 8, -8.0, 1), vec4(0, 0, 0, 1), vec4(0, 1, 0, 1));
	scene->currentCamera().Frustum(-5.0, 5.0, -5.0, 5.0, 5.0, 14.0);

	scene->addLight();
	scene->moveLight(vec3(4.0, 4.0, 8.0));

	//----------------------------------------------------------------------------
	// Initialize Callbacks

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(0, timer, 0);
	//glutSpecialFunc(special);
	initMenu();

	glutMainLoop();
	delete scene;
	delete renderer;
	return 0;
}

CWinApp theApp;

using namespace std;

int main(int argc, char** argv)
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		my_main(argc, argv);
	}

	return nRetCode;
}