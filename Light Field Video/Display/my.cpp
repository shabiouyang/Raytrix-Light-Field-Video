#include "my.h"

//Global Variable for Calling non static method from static method
DisplaySimulator *gds = NULL;


//Callback Function for Calling non static method from static method
void displayCallback()
{
	gds->displayCallback();
}

void keyboardCallback(unsigned char key, int x, int y)
{
	gds->keyboardCallback(key, x, y);
}

void specialCallback(int key, int x, int y)
{
	gds->specialCallback(key, x, y);
}

void motionCallback(int x, int y)
{
	gds->motionCallback(x, y);
}

void mouseCallback(int button, int state, int x, int y)
{
	gds->mouseCallback(button, state, x, y);
}

void reshapeCallback(int x, int y)
{
	gds->reshapeCallback(x, y);
}
/*
void idleCallback()
{
	gds->idleCallback;
}
*/


//Constructor
DisplaySimulator::DisplaySimulator(const char *fileName, int layerNum, int timeDivision, Size windowSize)
{
	//Layer Pattern Setting
	cout << "**************************" << layerNum * timeDivision << endl;
	cout << fileName << endl;
	frame = 0;
	int frameNum = 101;
	layerImages.resize(frameNum);
	for (int j = 0; j < frameNum; j++)
	{
		layerImages[j].resize(3);
	}
	for (int temp = 1; temp < frameNum; ++temp)
	{
		for (int i = 0; i < layerNum * timeDivision; i++)
		{
			char fileNameNum[128];
			//sprintf(fileNameNum, fileName, temp, i+1);
			sprintf_s(fileNameNum, sizeof(fileNameNum), fileName, temp, i + 1);
			Mat layerImage = imread(fileNameNum);
			if (layerImage.empty())
			{
				cout << "Cannot Load Layer Pattern: " << fileNameNum << endl;
			}
			else
			{
				cout << "Load Layer Pattern: " << fileNameNum << endl;
			}
			layerImages[temp][i] = layerImage;
		}
	}

	this->layerNum = layerNum;
	layerSize.width = layerImages[1][0].cols;
	layerSize.height = layerImages[1][0].rows;

	//Window,Mouse,View,Trackbar Setting
	this->windowSize = windowSize;
	isMouseRightButtonDown = false;
	viewPoint = Point3d(0.0, 0.0, 0.0);		//Front View
	viewAngle = Point3d(0.0, 0.0, 1.0);
	gridDrawFlag = false;
	menuDrawFlag = true;
	autoMoveFlag = false;

	controlflag = false;
	//Save Setting
	saveFlag = false;
	recordFlag = false;


	//GLUT Setting
	int argc = 0;
	glutInit(&argc, NULL);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(windowSize.width, windowSize.height);
	glutCreateWindow("Layered Display");
	glClearColor(0.5, 0.5, 0.5, 0.0);
	//glClearColor(0, 0, 0, 0.0);

	//Time Division Multiplexing Setting
	this->timeDivision = timeDivision;
	nowTimeDivision = 0;

	//VSYNC(Vertical Synchronization) Setting
	//Required library: wglew.h (and glew.h?)
	//if (timeDivision > 1)
	//{
	//	PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
	//	PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;
	//	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	//	wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
	//	wglSwapIntervalEXT(0);	//0:OFF, 1:ON (FPS = Monitor Refresh Rate / Number)
	//}
	//Texture Setting
	textureID = new GLuint*[frameNum];
	for (int i = 0; i < frameNum; ++i)
	{
		textureID[i] = new GLuint[layerNum * timeDivision];
	}
}
void DisplaySimulator::load_texture()
{
	for (int frame = 1; frame < 101; ++frame)
	{
		glGenTextures(layerNum * timeDivision, textureID[frame]);
		for (int layer = 0; layer < layerNum * timeDivision; layer++)
		{
			cvtColor(layerImages[frame][layer], layerImages[frame][layer], CV_BGR2RGB);
			glBindTexture(GL_TEXTURE_2D, textureID[frame][layer]);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, layerSize.width, layerSize.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, layerSize.width, layerSize.height, GL_RGB, GL_UNSIGNED_BYTE, layerImages[frame][layer].ptr());
			//glEnd();
		}
	}
}
		
		//frame += 1;
	
	/*
	else if (frame == 29)
	{
		glGenTextures(layerNum * timeDivision, textureID[frame]);
		for (int layer = 0; layer < layerNum * timeDivision; layer++)
		{
			cvtColor(layerImages[frame][layer], layerImages[frame][layer], CV_BGR2RGB);
			glBindTexture(GL_TEXTURE_2D, textureID[frame][layer]);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, layerSize.width, layerSize.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, layerSize.width, layerSize.height, GL_RGB, GL_UNSIGNED_BYTE, layerImages[frame][layer].ptr());
		}
		frame = 1;
	}
	*/
	

DisplaySimulator::~DisplaySimulator()
{
}

void DisplaySimulator::showParameter()
{
	cout << endl << "==========Parameter=========" << endl;
	if (viewModel == Orthographic)
		cout << "viewModel: Orthographic" << endl;
	else if (viewModel == Perspective)
	{
		cout << "viewModel: Perspective" << endl;
		cout << "cameraInterval: " << cameraInterval << endl;
	}
	cout << "layerNum: " << layerNum << endl;
	cout << "timeDivision: " << timeDivision << endl;
	cout << "layerDis: " << layerDis << endl;
	cout << "layerInterval: " << layerInterval << endl;
	cout << "dotPitch: " << dotPitch << endl;
	cout << "============================" << endl << endl;
}

void DisplaySimulator::run()
{
	gds = this;
	load_texture();
	glutDisplayFunc(::displayCallback);
	glutKeyboardFunc(::keyboardCallback);
	glutSpecialFunc(::specialCallback);
	glutMotionFunc(::motionCallback);
	glutMouseFunc(::mouseCallback);
	glutReshapeFunc(::reshapeCallback);
	//glutIdleFunc(::idleCallback);
	//int argc = 0;
	//glutInit(&argc, NULL);
	glutTimerFunc(1000 / FPS, idleCallback, 0);
	glutMainLoop();
}

void DisplaySimulator::init(double cameraInterval, double layerDis, double layerInterval, double dotPitch, viewModels viewModel)
{
	this->layerDis = layerDis;					//Distance[mm] from Camera to Middle Layer
	this->layerInterval = layerInterval;		//Interval[mm] from Layer to Layer
	this->dotPitch = dotPitch;					//Dot Pitch[mm] of Display's Pixel
	this->viewModel = viewModel;				//Light Ray Model (Orthographic or Perspective)
	this->cameraInterval = cameraInterval;		//Camera Interval[mm] from Virtual Camera to Virtual Camera

	setInitParameter();
}

void DisplaySimulator::drawLayer(Size2d layerSize, double z)
{
	layerSize.width /= (double)windowSize.width;
	layerSize.height /= (double)windowSize.height;
	z /= (double)windowSize.width * dotPitch;

	GLdouble pointA[] = { -layerSize.width, layerSize.height, z };
	GLdouble pointB[] = { layerSize.width, layerSize.height, z };
	GLdouble pointC[] = { layerSize.width, -layerSize.height, z };
	GLdouble pointD[] = { -layerSize.width, -layerSize.height, z };

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	glTexCoord2d(1.0, 0.0);
	glVertex3dv(pointA);
	glTexCoord2d(0.0, 0.0);
	glVertex3dv(pointB);
	glTexCoord2d(0.0, 1.0);
	glVertex3dv(pointC);
	glTexCoord2d(1.0, 1.0);
	glVertex3dv(pointD);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void DisplaySimulator::drawBacklight(Size2d backlightSize, double z)
{
	backlightSize.width /= (double)windowSize.width;
	backlightSize.height /= (double)windowSize.height;
	z /= (double)windowSize.width * dotPitch;

	GLdouble pointA[] = { -backlightSize.width, -backlightSize.height, z };
	GLdouble pointB[] = { backlightSize.width, -backlightSize.height, z };
	GLdouble pointC[] = { backlightSize.width, backlightSize.height, z };
	GLdouble pointD[] = { -backlightSize.width, backlightSize.height, z };

	//White
	glColor4ub(255, 255, 255, 255);
	glBegin(GL_POLYGON);
	glVertex3dv(pointA);
	glVertex3dv(pointB);
	glVertex3dv(pointC);
	glVertex3dv(pointD);
	glEnd();
}

void DisplaySimulator::drawGrid(Size2d groundSize, double interval)
{
	groundSize.width /= (double)windowSize.width;
	groundSize.height /= (double)windowSize.height;

	//Z Axis: Black
	glColor3ub(0, 0, 0);
	glLineWidth(3.0);
	glBegin(GL_LINES);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, layerDis / windowSize.width / dotPitch);
	glEnd();

	//Grid: White
	glColor3ub(255, 255, 255);
	glLineWidth(1.0);
	glBegin(GL_LINES);
	for (int i = 0; i <= layerDis / interval; i++)
	{
		glVertex3d(-groundSize.width, 0, interval / windowSize.width / dotPitch * i);
		glVertex3d(groundSize.width, 0, interval / windowSize.width / dotPitch * i);
	}
	glEnd();
}

void DisplaySimulator::drawText(string text, Point2f textPos)
{
	glColor3d(0.0, 0.0, 0.0);

	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, windowSize.width, windowSize.height, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glRasterPos2f(textPos.x, textPos.y);
	int size = (int)text.size();
	for (int i = 0; i < size; ++i)
	{
		char ic = text[i];
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ic); //GLUT_BITMAP_TIMES_ROMAN_24
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void DisplaySimulator::drawParameter()
{
	Point2f textPos(10, 18);
	if (viewModel == Orthographic)
		drawText("viewModel: Orthographic", Point2f(textPos.x, textPos.y));
	else if (viewModel == Perspective)
		drawText("viewModel: Perspective", Point2f(textPos.x, textPos.y));
	drawText("layerDis (d,f): " + to_string(layerDis), Point2f(textPos.x, textPos.y * 2));
	drawText("layerInterval (j,k): " + to_string(layerInterval), Point2f(textPos.x, textPos.y * 3));
	drawText("dotPitch (c,v): " + to_string(dotPitch), Point2f(textPos.x, textPos.y * 4));
	if (viewModel == Perspective)
		drawText("cameraInterval: " + to_string(cameraInterval), Point2f(textPos.x, textPos.y * 5));

	textPos.x = 310;	//400
	drawText("i: Init Param", Point2f(textPos.x, textPos.y));
	drawText("1: Front View", Point2f(textPos.x, textPos.y * 2));
	drawText("2: Top View", Point2f(textPos.x, textPos.y * 3));
	drawText("3: Side View", Point2f(textPos.x, textPos.y * 4));
	drawText("a: Auto Move", Point2f(textPos.x, textPos.y * 5));

	textPos.x = 450;	//580
	drawText("s: Save Image", Point2f(textPos.x, textPos.y));
	drawText("r: Record Image", Point2f(textPos.x, textPos.y * 2));
	drawText("g: Grid on/off", Point2f(textPos.x, textPos.y * 3));
	drawText("m: Menu on/off", Point2f(textPos.x, textPos.y * 4));
	drawText("q: Quit", Point2f(textPos.x, textPos.y * 5));
}

double DisplaySimulator::world2screen(double worldScale)
{
	return worldScale / windowSize.width / dotPitch;
}

double DisplaySimulator::calcFPS()
{
	static double fps = 0.0;
	static int frameNum = 0;
	static int startTime = 0;
	int nowTime = glutGet(GLUT_ELAPSED_TIME);	//Number of milliseconds since glutInit called
	frameNum++;

	//if (nowTime - startTime > 1000)
	//{
		fps = frameNum * 1000.0 / (nowTime - startTime);
		printf("FPS:%f\r", fps);
		startTime = nowTime;
		frameNum = 0;
	//}

	return fps;
}

bool DisplaySimulator::getWindowImage(Mat &windowImage)
{
	static Mat multiplexImage = Mat::zeros(windowImage.size(), CV_8UC3);

	//Convert OpenGL Window into OpenCV Mat
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, windowImage.cols, windowImage.rows, GL_RGB, GL_UNSIGNED_BYTE, windowImage.data);
	flip(windowImage, windowImage, 0);
	cvtColor(windowImage, windowImage, CV_BGR2RGB);

	//Image Multiplex for TDM (Time Division Multiplexing)
	if (timeDivision == 1)
	{
		return true;
	}
	else
	{
		multiplexImage += windowImage / timeDivision;
		if (nowTimeDivision == (timeDivision - 1))
		{
			windowImage = multiplexImage.clone();
			multiplexImage = Mat::zeros(windowSize, CV_8UC3);
			return true;
		}
	}

	return false;
}

void DisplaySimulator::setInitParameter()
{
	static Size windowSize = this->windowSize;
	static int timeDivision = this->timeDivision;
	static int layerNum = this->layerNum;
	static double layerDis = this->layerDis;
	static double layerInterval = this->layerInterval;
	static double dotPitch = this->dotPitch;
	static viewModels viewModel = this->viewModel;

	this->windowSize = windowSize;
	this->timeDivision = timeDivision;
	this->layerNum = layerNum;
	this->layerDis = layerDis;
	this->layerInterval = layerInterval;
	this->dotPitch = dotPitch;
	this->viewModel = viewModel;
}

void DisplaySimulator::autoMoveViewPoint()
{
	static int loopCnt = 0;
	double oneFrameMoveAmount = 5/2;
	double moveRadius = 10/2;

	if (loopCnt == 0)
	{
		viewPoint = Point3d(0.0, 0.0, 0.0);
		viewAngle = Point3d(0.0, 0.0, world2screen(layerDis));
	}
	if (loopCnt < moveRadius)
		viewPoint.x += world2screen(oneFrameMoveAmount);
	else if (loopCnt < moveRadius * 3)
		viewPoint.x -= world2screen(oneFrameMoveAmount);
	else if (loopCnt < moveRadius * 4)
		viewPoint.x += world2screen(oneFrameMoveAmount);
	else if (loopCnt < moveRadius * 5)
		viewPoint.y -= world2screen(oneFrameMoveAmount);
	else if (loopCnt < moveRadius * 7)
		viewPoint.y += world2screen(oneFrameMoveAmount);
	else if (loopCnt == moveRadius * 7)
		loopCnt += 450 - loopCnt % 360;
	else
	{
		viewPoint.x = world2screen(moveRadius * oneFrameMoveAmount * cos(M_PI * loopCnt / 180));
		viewPoint.y = world2screen(moveRadius * oneFrameMoveAmount * sin(M_PI * loopCnt / 180));
		loopCnt += (int)(oneFrameMoveAmount / 2);
	}

	loopCnt++;
}

void DisplaySimulator::recordWindow(Mat &windowImage, bool autoMoveFlag, char *fileName)
{
	static int fileCnt = 1;

	char fileNameNum[128];
	sprintf_s(fileNameNum, sizeof(fileNameNum), fileName, fileCnt);
	imwrite(fileNameNum, windowImage);

	if (autoMoveFlag)
	{
		autoMoveViewPoint();
		if (fileCnt == 193)
		{
			fileCnt = 1;
			recordFlag = false;
			printf("Record End                   \r");
		}
	}

	fileCnt++;
}


//Callback Function
void DisplaySimulator::displayCallback()
{
	//Display Light Ray Model Setting
	glViewport(0, 0, windowSize.width, windowSize.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double fieldOfViewAngle = atan((double)layerSize.width / layerDis * dotPitch) * 2 * 180 / M_PI;
	gluPerspective(fieldOfViewAngle, layerSize.width / layerSize.height, 0.001, 100);
	//glOrtho(-1.0, 1.0, -1.0, 1.0, -100.0, 100.0);	//Orthographic View Model for Simulator

	//View Point Setting
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(viewPoint.x, viewPoint.y, viewPoint.z, viewAngle.x, viewAngle.y, viewAngle.z, 0.0, 1.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	//Draw Grid and Backlight
	glPushMatrix();
	if (gridDrawFlag)
		drawGrid(Size2d(layerSize.width, layerSize.height), 100);
	drawBacklight(Size2d(layerSize.width, layerSize.height), layerDis + (layerInterval * 2));
	glPopMatrix();

	//Draw Layer Pattern (Draw Order: Backlight -> layer_3 -> layer_2 -> layer_1)
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);		//GL_ZERO:(0,0,0,0) + GL_SRC_COLOR:(Rs/kR,Gs/kG,Bs/kB,As/kA) = Multiplication
	//cout << frame << endl;
	if (controlflag)
	{
		for (int layer = layerNum - 1; layer >= 0; layer--)
		{
			glBindTexture(GL_TEXTURE_2D, textureID[frame][layer * timeDivision + nowTimeDivision]);
			glPushMatrix();
			double eachLayerDis = layerDis + (layer - 1) * layerInterval;
			drawLayer(Size2d(layerSize.width, layerSize.height), eachLayerDis);
			glPopMatrix();
		}
		char fileName[128] = "../LayerPattern/video/Scene_03/Perspective/Result_%03d/layer_%d.png";
		char fileNameNum[128];
		sprintf_s(fileNameNum, sizeof(fileNameNum), fileName, frame, 0);
		strcpy_s(saveFolderName, fileNameNum);
		while (saveFolderName[strlen(saveFolderName) - 1] != '/')
			saveFolderName[strlen(saveFolderName) - 1] = '\0';
		strcat_s(saveFolderName, "SimulationResult");
		_mkdir(saveFolderName);
		cout << saveFolderName << endl;
	}
	else
	{
		if (frame != 100)
		{
			for (int layer = layerNum - 1; layer >= 0; layer--)
			{
				glBindTexture(GL_TEXTURE_2D, textureID[frame][layer * timeDivision + nowTimeDivision]);
				glPushMatrix();
				double eachLayerDis = layerDis + (layer - 1) * layerInterval;
				drawLayer(Size2d(layerSize.width, layerSize.height), eachLayerDis);
				glPopMatrix();
			}
			frame += 1;
		}

		else if (frame == 100)
		{
			for (int layer = layerNum - 1; layer >= 0; layer--)
			{
				glBindTexture(GL_TEXTURE_2D, textureID[frame][layer * timeDivision + nowTimeDivision]);
				glPushMatrix();
				double eachLayerDis = layerDis + (layer - 1) * layerInterval;
				drawLayer(Size2d(layerSize.width, layerSize.height), eachLayerDis);
				glPopMatrix();
			}
			frame = 1;
		}
	}

	
	glDisable(GL_BLEND);
	
	//Draw Parameter 
	if (menuDrawFlag)
		drawParameter();

	glFlush();
	glDisable(GL_DEPTH_TEST);
	glutSwapBuffers();

	//Calculate FPS 
	frame_count++;
	final_time = time(NULL);
	if (final_time - initial_time> 0)
	{
		cout << "Current FPS: " << frame_count / (final_time - initial_time) << endl;
		frame_count = 0;
		initial_time = final_time;
	}

	//OpenCV Processing (Save and Multiplex)
	
	Mat windowImage(Size(windowSize.height / 4 * 4, windowSize.width / 4 * 4), CV_8UC3);	//Mat Size is Multiple of 4
	if (getWindowImage(windowImage))
	{
		//cout << timeDivision << endl;
		//Time Division Multiplexing Processing
		if (timeDivision > 1)
		{
			//Calculation FPS
			calcFPS();

			//Show Multiplex Image (Show OpenGL and OpenCV side by side)
			imshow("windowImage", windowImage);
			cvMoveWindow("windowImage", windowSize.width + 30, 0);
			waitKey(0);
		}

		//Save Window Image by Pushed 's'
		if (saveFlag)
		{
			char fullFileName[128];
			strcpy_s(fullFileName, saveFolderName);
			strcat_s(fullFileName, "/window.png");
			imwrite(fullFileName, windowImage);
			saveFlag = false;
		}

		//Record Window Image by Pushed 'r'
		if (recordFlag)
		{
			char fullFileName[128];
			strcpy_s(fullFileName, saveFolderName);
			strcat_s(fullFileName, "/window_%03d.png");
			recordWindow(windowImage, true, fullFileName);
		}
	}

	//Auto Moving
	if (autoMoveFlag)
		autoMoveViewPoint();
	
	//Current Frame (timeDivision = 2, nowTimeDivision = 0,1,0,1...)
	nowTimeDivision++;
	nowTimeDivision %= timeDivision;
	
}
void DisplaySimulator::keyboardCallback(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'i':	//Set Initial Parameter
		setInitParameter();
		glutReshapeWindow(windowSize.width, windowSize.height);
		//break;
	case '1':	//Front View
		autoMoveFlag = false;
		viewPoint = Point3d(0.0, 0.0, 0.0);
		viewAngle = Point3d(0.0, 0.0, world2screen(layerDis));
		printf("Front View                     \r");
		break;
	case '2':	//Top View
		autoMoveFlag = false;
		viewPoint = Point3d(0.0, world2screen(layerDis), world2screen(layerDis - layerInterval * 0.5));
		viewAngle = Point3d(0.0, 0.0, world2screen(layerDis));
		printf("Top View                       \r");
		break;
	case '3':	//Side View
		autoMoveFlag = false;
		viewPoint = Point3d(world2screen(layerDis), 0.0, world2screen(layerDis - layerInterval * 0.5));
		viewAngle = Point3d(0.0, 0.0, world2screen(layerDis));
		printf("Side View                      \r");
		break;
	case 'k':
		layerInterval = (int)layerInterval + 1.0;
		printf("layerInterval:%-20g\r", layerInterval);
		break;
	case 'j':
		if (layerInterval > 1.0)
			layerInterval = (int)layerInterval - 1.0;
		printf("layerInterval:%-20g\r", layerInterval);
		break;
	case 'v':
		dotPitch += 0.01;
		printf("dotPitch:%-20g\r", dotPitch);
		break;
	case 'c':
		if (dotPitch > 0.01)
			dotPitch -= 0.01;
		printf("dotPitch:%-20g\r", dotPitch);
		break;
	case 'f':
		layerDis = (int)layerDis + 1.0;
		printf("layerDis:%-20g\r", layerDis);
		break;
	case 'd':
		if (layerDis > 1.0)
			layerDis = (int)layerDis - 1.0;
		printf("layerDis:%-20g\r", layerDis);
		break;
	case 'g':
		if (gridDrawFlag)
		{
			gridDrawFlag = false;
			printf("Grid OFF                     \n");
		}
		else
		{
			gridDrawFlag = true;
			printf("Grid ON                      \n");
		}
		break;
	case 'm':
		if (menuDrawFlag)
		{
			menuDrawFlag = false;
			printf("Menu OFF                     \n");
		}
		else
		{
			menuDrawFlag = true;
			printf("Menu ON                      \r");
		}
		break;
	case 'a':
		if (autoMoveFlag)
		{
			autoMoveFlag = false;
			printf("Auto Moving OFF              \n");
		}
		else
		{
			autoMoveFlag = true;
			printf("Auto Moving ON               \n");
		}
		break;
	case 's':
		saveFlag = true;
		printf("Save Image                       \r");
		break;
	case 'r':
		if (recordFlag)
		{
			recordFlag = false;
			printf("Record End                   \n");
		}
		else
		{
			recordFlag = true;
			printf("Record Start                 \n");
		}
		break;
	case 'p':
		if (controlflag)
		{
			controlflag = false;
			printf("Pause                   \n");
		}
		else
		{
			controlflag = true;
			printf("Play                 \n");
		}
		break;
	case 'q':
	case 'Q':
	case '\033':
		exit(0);
	}
}

void DisplaySimulator::specialCallback(int key, int x, int y)
{
	double oneFrameMoveAmount = layerDis * dotPitch / layerInterval;
	oneFrameMoveAmount = 2;

	switch (key)
	{
	case GLUT_KEY_UP:
		viewPoint.y += world2screen(oneFrameMoveAmount);
		viewAngle = Point3d(0.0, 0.0, world2screen(layerDis));
		//cout << "viewPoint: " << viewPoint << endl;
		//cout << "viewAngle: " << viewAngle << endl;
		break;
	case GLUT_KEY_DOWN:
		viewPoint.y -= world2screen(oneFrameMoveAmount);
		viewAngle = Point3d(0.0, 0.0, world2screen(layerDis));
		//cout << "viewPoint: " << viewPoint << endl;
		//cout << "viewAngle: " << viewAngle << endl;
		break;
	case GLUT_KEY_LEFT:
		viewPoint.x += world2screen(oneFrameMoveAmount);
		viewAngle = Point3d(0.0, 0.0, world2screen(layerDis));
		//cout << "viewPoint: " << viewPoint << endl;
		//cout << "viewAngle: " << viewAngle << endl;
		break;
	case GLUT_KEY_RIGHT:
		viewPoint.x -= world2screen(oneFrameMoveAmount);
		viewAngle = Point3d(0.0, 0.0, world2screen(layerDis));
		//cout << "viewPoint: " << viewPoint << endl;
		//cout << "viewAngle: " << viewAngle << endl;
		break;
	}
}

void DisplaySimulator::motionCallback(int x, int y)
{
	double motionSpeed = 1.0 / 200.0;

	if (isMouseLeftButtonDown == true)
	{
		viewPoint.x += (double)(x - mouseLeftButtonClickPos.x) * motionSpeed;
		viewPoint.y += (double)(y - mouseLeftButtonClickPos.y) * motionSpeed;
		viewAngle = Point3d(0.0, 0.0, world2screen(layerDis));
	}
	mouseLeftButtonClickPos = Point(x, y);
}

void DisplaySimulator::mouseCallback(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_UP)
		{
			isMouseLeftButtonDown = false;
		}
		else if (state == GLUT_DOWN)
		{
			autoMoveFlag = false;
			isMouseLeftButtonDown = true;
			mouseLeftButtonClickPos.x = x;
			mouseLeftButtonClickPos.y = y;
		}
	}

	if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_UP)
		{
			isMouseRightButtonDown = false;
		}
		else if (state == GLUT_DOWN)
		{
			isMouseRightButtonDown = true;
		}
	}
}

void DisplaySimulator::reshapeCallback(int x, int y)
{
	bool windowFixFlag = false;

	if (windowFixFlag)
	{
		glutReshapeWindow(windowSize.width, windowSize.height);
	}
	else
	{
		//Resize to a Square
		if (windowSize.width == x)
		{
			glutReshapeWindow(y, y);
			windowSize = Size(y, y);
		}
		else if (windowSize.width == y)
		{
			glutReshapeWindow(x, x);
			windowSize = Size(x, x);
		}
		else
		{
			glutReshapeWindow(x, x);
			windowSize = Size(x, x);
		}
	}
}
/*
void DisplaySimulator::idleCallback(int)
{
	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, &::idleCallback, 0);
}
*/
void idleCallback(int)
{
	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, idleCallback, 0);
}