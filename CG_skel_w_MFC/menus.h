#pragma once

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
#define LIGHT_CHANGE_COLOR 3

#define SHADING_WIRE 0
#define SHADING_FLAT 1
#define SHADING_PHONG 2
#define SHADING_GOURAUD 3

#define MATERIAL_CHANGE_COLOR 0
#define MATERIAL_CHANGE_AMBIENT 1
#define MATERIAL_CHANGE_DIFFUSE 2
#define MATERIAL_CHANGE_SPECULAR 3
#define MATERIAL_CHANGE_SHININESS 4


// global variables from main
extern Scene* scene;
extern Renderer* renderer;

float userInput()
{
	float input;
	cin >> input;
	return input;
}

void modelMenu(int id)
{
	CFileDialog dlgFile(TRUE, _T(".obj"), NULL, NULL, _T("*.obj|*.*"));
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
		if (dlgFile.DoModal() == IDOK)
		{
			std::string s((LPCTSTR)dlgFile.GetPathName());
			scene->loadOBJModel((LPCTSTR)dlgFile.GetPathName());
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
		AfxMessageBox(_T("Simon's 3D Software Renderer"));
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
	CRGBDialog dlg;
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
		break;
	case LIGHT_CHANGE_COLOR:
		
		if (dlg.DoModal() == IDOK) {
			vec3 v = dlg.GetXYZ();
			scene->changeLightColor(v);
		}

		break;
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
		for (int i = 0; i < 6; i++)
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

void materialMenu(int id)
{
	CRGBDialog dlg;
	CFloatDialog fdlg;

	switch (id)
	{
	case MATERIAL_CHANGE_COLOR:
		dlg.mTitle = "Pick Color RGB";
		dlg.insertData(scene->getMaterial(color));
		if (dlg.DoModal() == IDOK) {
			vec3 v = dlg.GetXYZ();
			scene->changeMaterial(color, v);
		}
		break;
	case MATERIAL_CHANGE_AMBIENT:
		dlg.mTitle = "Pick Color RGB";
		dlg.insertData(scene->getMaterial(ambient));
		if (dlg.DoModal() == IDOK) {
			vec3 v = dlg.GetXYZ();
			scene->changeMaterial(ambient, v);
		}
		break;
	case MATERIAL_CHANGE_DIFFUSE:
		dlg.mTitle = "Pick Color RGB";
		dlg.insertData(scene->getMaterial(diffuse));
		if (dlg.DoModal() == IDOK) {
			vec3 v = dlg.GetXYZ();
			scene->changeMaterial(ambient, v);
		}
		break;
	case MATERIAL_CHANGE_SPECULAR:
		dlg.mTitle = "Pick Color RGB";
		dlg.insertData(scene->getMaterial(specular));
		if (dlg.DoModal() == IDOK) {
			vec3 v = dlg.GetXYZ();
			scene->changeMaterial(specular, v);
		}
		break;
	case MATERIAL_CHANGE_SHININESS:
		fdlg.mTitle = "Pick Value";
		fdlg.insertData(scene->getMaterial(shine).x);
		if (fdlg.DoModal() == IDOK) {
			float v = fdlg.Getfloat();
			scene->changeMaterial(shine, v);
		}
		break;
	}
}