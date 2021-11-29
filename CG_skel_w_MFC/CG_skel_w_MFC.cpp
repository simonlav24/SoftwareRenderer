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

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#define MODEL_OPEN 0 
#define MODEL_CUBE 1
#define MODEL_SWITCH 2 
#define MODEL_DELETE 3 

#define FILE_OPEN 1
#define MAIN_DEMO 1
#define MAIN_ABOUT 2
#define MAIN_MODEL_SWITCH 3
#define MAIN_CAMERA_SWITCH 4

#define SHOW_TOGGLE_BOUNDING_BOX 0
#define SHOW_TOGGLE_FACE_NORMALS 1
#define SHOW_TOGGLE_VERTEX_NORMALS 2
#define SHOW_TOGGLE_GRID 3 

#define TRANSFORM_MODEL 0
#define TRANSFORM_WORLD 1
#define TRANSFORM_CAMERA 2
#define TRANSFORM_LIGHT 3

#define MODE_POSITION 0
#define MODE_SCALE 1
#define MODE_ROTATION 2

#define CAMERA_ADD_ORTHO 0 
#define CAMERA_ADD_PERSP 1
#define CAMERA_SWITCH 2
#define CAMERA_RESET 3
#define CAMERA_EDIT_ORTHO 4
#define CAMERA_EDIT_FRUS 5
#define CAMERA_EDIT_FOVY 6
#define CAMERA_LOOK_MODEL 7

#define LIGHT_ADD 0
#define LIGHT_DELETE 1
#define LIGHT_SWITCH 2

#define SHADING_WIRE 0
#define SHADING_FLAT 1
#define SHADING_PHONG 2
#define SHADING_GOURAUD 3

Scene* scene;
Renderer* renderer;

int last_x, last_y;
bool lb_down, rb_down, mb_down;

// more stuff
int framesPerSecond{ 10 };
GLfloat step{ 0.5 };

//----------------------------------------------------------------------------
// Callbacks
static GLfloat t = 0;
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
	t += 0.25;
	// Call the scene and ask it to draw itself
	scene->draw();
}

void reshape(int width, int height)
{
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
			scene->transformActiveModel(RotateX(-rotStep));
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
			scene->transformActiveModel(RotateX(rotStep));
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
			scene->transformActiveModel(RotateZ(rotStep));
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
			scene->transformActiveModel(RotateZ(-rotStep));
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
			scene->transformActiveModel(RotateY(rotStep));
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
			scene->transformActiveModel(RotateY(-rotStep));
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
		renderer->ambientIntensity -= 0.1;
		cout << "ambientIntensity " << renderer->ambientIntensity << endl;
		break;
	case ']':
		renderer->ambientIntensity += 0.1;
		cout << "ambientIntensity " << renderer->ambientIntensity << endl;
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	//button = {GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON}
	//state = {GLUT_DOWN,GLUT_UP}
	if (button == 3)
		scene->rotateZoomCamera(0, 0, 1);
	if (button == 4)
		scene->rotateZoomCamera(0, 0, -1);
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
			scene->rotateZoomCamera(dx, dy, 0);
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

float userInput()
{
	float input;
	cin >> input;
	return input;
}

void timer(int id)
{
	glutPostRedisplay();
	glutTimerFunc(1000 / framesPerSecond, timer, 0);
}

void modelMenu(int id)
{
	switch (id)
	{
	case MODEL_CUBE:
		scene->createPrimitive();
		break;
	case MODEL_SWITCH:
		scene->switchActiveModel();
		break;
	case MODEL_DELETE:
		scene->deleteActiveModel();
		break;
	case MODEL_OPEN:
		CFileDialog dlg(TRUE, _T(".obj"), NULL, NULL, _T("*.obj|*.*"));
		if (dlg.DoModal() == IDOK)
		{
			std::string s((LPCTSTR)dlg.GetPathName());
			scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
		}
		break;
	}
}

// main menu switcher
void mainMenu(int id)
{
	switch (id)
	{
	case MAIN_DEMO:
		scene->drawDemo();
		break;
	case MAIN_ABOUT:
		AfxMessageBox(_T("Computer Graphics"));
		break;
	case MAIN_CAMERA_SWITCH:
		scene->switchActiveCamera();
		break;
	}
}

// frame switcher submenu
void frameStateMenu(int id)
{
	switch (id)
	{
	case TRANSFORM_MODEL:
		scene->tState = model;
		break;
	case TRANSFORM_WORLD:
		scene->tState = world;
		break;
	case TRANSFORM_CAMERA:
		scene->tState = camera;
		break;
	case TRANSFORM_LIGHT:
		scene->tState = light;
		break;
	}
}

// Light mode switcher submenu
void shadingSetupMenu(int id)
{
	switch (id)
	{
	case SHADING_WIRE:
		renderer->shadingSetup = WireFrame;
		break;
	case SHADING_FLAT:
		renderer->shadingSetup = Flat;
		break;
	case SHADING_GOURAUD:
		renderer->shadingSetup = Gouraud;
		break;
	case SHADING_PHONG:
		renderer->shadingSetup = Phong;
		break;
	}
}

// mode switcher submenu
void modeStateMenu(int id)
{
	switch (id)
	{
	case MODE_POSITION:
		scene->tMode = position;
		break;
	case MODE_SCALE:
		scene->tMode = scale;
		break;
	case MODE_ROTATION:
		scene->tMode = rotation;
		break;
	}
}

// show extra features submenu
void showMenu(int id)
{
	switch (id)
	{
	case SHOW_TOGGLE_BOUNDING_BOX:
		scene->showBoundingBox = !scene->showBoundingBox;
		break;
	case SHOW_TOGGLE_FACE_NORMALS:
		scene->showFaceNormals = !scene->showFaceNormals;
		break;
	case SHOW_TOGGLE_VERTEX_NORMALS:
		scene->showVertexNormals = !scene->showVertexNormals;
		break;
	case SHOW_TOGGLE_GRID:
		scene->showGrid = !scene->showGrid;
		break;
	}
}

void lightMenu(int id)
{
	switch (id)
	{
	case LIGHT_ADD:
		scene->addLight();
		break;
	case LIGHT_DELETE:
		scene->deleteActiveLight();
		break;
	case LIGHT_SWITCH:
		scene->switchActiveLight();
	}
}

void cameraMenu(int id)
{
	float multiInput[6];
	switch (id)
	{
	case CAMERA_ADD_ORTHO:
		scene->addCamera();
		scene->currentCamera().LookAt(vec4(0.0, 0.0, 10.0, 1), vec4(0, 0, 0, 1), vec4(0, 1, 0, 1));
		renderer->viewerPos = scene->currentCamera().Eye;
		scene->currentCamera().Ortho(-5.0, 5.0, -5.0, 5.0, 5.0, 14.0);
		break;
	case CAMERA_ADD_PERSP:
		scene->addCamera();
		scene->currentCamera().LookAt(vec4(8, 8, -8.0, 1), vec4(0, 0, 0, 1), vec4(0, 1, 0, 1));
		renderer->viewerPos = scene->currentCamera().Eye;
		scene->currentCamera().Frustum(-5.0, 5.0, -5.0, 5.0, 5.0, 14.0);
		break;
	case CAMERA_SWITCH:
		scene->switchActiveCamera();
		break;
	case CAMERA_LOOK_MODEL:
		scene->lookAtModel();
		break;
	case CAMERA_EDIT_ORTHO:
		AfxMessageBox(_T("Insert values in the console"));
		cout << "-- Enter values: left, right, bottom, top, near, far --" << endl;
		for(int i = 0; i < 6; i++)
			multiInput[i] = userInput();
		scene->currentCamera().Ortho(multiInput[0], multiInput[1], multiInput[2], multiInput[3], multiInput[4], multiInput[5]);
		break;
	case CAMERA_EDIT_FRUS:
		AfxMessageBox(_T("Insert values in the console"));
		cout << "-- Enter values: left, right, bottom, top, near, far --" << endl;
		for (int i = 0; i < 6; i++)
			multiInput[i] = userInput();
		scene->currentCamera().Frustum(multiInput[0], multiInput[1], multiInput[2], multiInput[3], multiInput[4], multiInput[5]);
		break;
	case CAMERA_EDIT_FOVY:
		AfxMessageBox(_T("Insert values in the console"));
		cout << "-- Enter values: fovy, aspect ratio, near, far --" << endl;
		for (int i = 0; i < 4; i++)
			multiInput[i] = userInput();
		scene->currentCamera().Perspective(multiInput[0], multiInput[1], multiInput[2], multiInput[3]);
		break;
		break;
	case CAMERA_RESET:
		scene->resetCameraPosition();
		break;
	}
}

// menu initialization function
void initMenu()
{
	int menuModel = glutCreateMenu(modelMenu);
	glutAddMenuEntry("Open obj file", MODEL_OPEN);
	glutAddMenuEntry("Load Primitive Cube", MODEL_CUBE);
	glutAddMenuEntry("Switch Model", MODEL_SWITCH);
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
	glutAddMenuEntry("Add Light", LIGHT_ADD);
	glutAddMenuEntry("Switch Light", LIGHT_SWITCH);
	glutAddMenuEntry("Delete Light", LIGHT_DELETE);

	int menuLightSetup = glutCreateMenu(shadingSetupMenu);
	glutAddMenuEntry("WireFrame", SHADING_WIRE);
	glutAddMenuEntry("Flat Shading", SHADING_FLAT);
	glutAddMenuEntry("Phong Shading", SHADING_PHONG);
	glutAddMenuEntry("Gouraud Shading", SHADING_GOURAUD);

	glutCreateMenu(mainMenu);
	glutAddSubMenu("Model", menuModel);
	glutAddSubMenu("Switch Frame", menuFrame);
	glutAddSubMenu("Transformation Mode", menuMode);
	glutAddSubMenu("Camera", menuCamera);
	glutAddSubMenu("Light", menuLight);
	glutAddSubMenu("Shading mode", menuLightSetup);
	glutAddSubMenu("Show", menuShow);
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
	//scene->addCamera();
	//scene->currentCamera().LookAt(vec4(0.0, 0.0, 10.0, 1), vec4(0, 0, 0, 1), vec4(0, 1, 0, 1));
	//scene->currentCamera().Ortho(-5.0, 5.0, -5.0, 5.0, 5.0, 14.0);

	scene->addCamera();
	scene->currentCamera().LookAt(vec4(8, 8, -8.0, 1), vec4(0, 0, 0, 1), vec4(0, 1, 0, 1));
	renderer->viewerPos = scene->currentCamera().Eye;
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