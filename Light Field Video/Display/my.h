/*======================================================================*
* Class for layered 3D display simulator
*
* @file		Simulator > DisplaySimulator.h
* @author	S. Mikawa
* @date		2018/02/dd
*======================================================================*/

#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <vector>
#include <direct.h>
#include <opencv2/opencv.hpp>
#include <GL/glut.h>

using namespace std;
using namespace cv;
#define FPS 30  
enum viewModels { Orthographic, Perspective };

class DisplaySimulator
{
private:
	int initial_time = time(NULL), final_time, frame_count = 0;
	int frame;
	Size layerSize;
	vector<vector<Mat>> layerImages;
	int nowTimeDivision;
	GLuint **textureID;
	bool isMouseLeftButtonDown;
	bool isMouseRightButtonDown;
	bool saveFlag;
	bool recordFlag;
	bool autoMoveFlag;
	bool gridDrawFlag;
	bool menuDrawFlag;
	bool controlflag;
	char saveFolderName[128];
	Point mouseLeftButtonClickPos;
	Point3d viewPoint;
	Point3d viewAngle;

	//Simulation Parameter
	Size windowSize;
	int timeDivision;
	int layerNum;
	double cameraInterval;
	double layerDis;
	double layerInterval;
	double dotPitch;
	viewModels viewModel;

	//Draw Material
	void drawLayer(Size2d layerSize, double z);
	void drawBacklight(Size2d backlightSize, double z);
	void drawGrid(Size2d groundSize, double interval);
	void drawText(string text, Point2f textPos);
	void drawParameter();

	double world2screen(double worldScale);
	double calcFPS();
	bool getWindowImage(Mat &windowImage);
	void setInitParameter();
	void autoMoveViewPoint();
	void recordWindow(Mat &windowImage, bool autoMoveFlag, char *fileName);

public:
	DisplaySimulator(const char *fileName, int layerNum, int timeDivision, Size windowSize);
	~DisplaySimulator();

	void load_texture();
	//Callback Function
	void displayCallback();
	void keyboardCallback(unsigned char key, int x, int y);
	void specialCallback(int key, int x, int y);
	void motionCallback(int x, int y);
	void mouseCallback(int button, int state, int x, int y);
	void reshapeCallback(int x, int y);
	//void idleCallback(int);

	void init(double cameraInterval, double layerDis, double layerInterval, double dotPitch, viewModels viewModel = Orthographic);
	void showParameter();
	void run();
};
void idleCallback(int);