#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <atlbase.h> 
#include "my.h"
#define CV_LIB_PREFIX comment(lib, "opencv_"

#define CV_LIB_VERSION CVAUX_STR(CV_MAJOR_VERSION)\
    CVAUX_STR(CV_MINOR_VERSION)\
    CVAUX_STR(CV_SUBMINOR_VERSION)

#ifdef _DEBUG
#define CV_LIB_SUFFIX CV_LIB_VERSION "d.lib")
#else
#define CV_LIB_SUFFIX CV_LIB_VERSION ".lib")
#endif

#define CV_LIBRARY(lib_name) CV_LIB_PREFIX CVAUX_STR(lib_name) CV_LIB_SUFFIX
#pragma warning(disable:4996)
#pragma CV_LIBRARY(core)
#pragma CV_LIBRARY(highgui)
#pragma CV_LIBRARY(imgcodecs)
#pragma CV_LIBRARY(imgproc)
using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
	wchar_t cameraInterval[128], layerDis[128], layerInterval[128];
	char layerFileName[128] = "../LayerPattern/video/Scene_06/Perspective/Result_%03d/layer_%d.png";
	int layerNum = 3;
	int timeDivision = 1;
	DisplaySimulator DS(layerFileName, layerNum, timeDivision, Size(920, 920));
	DS.init(0.690300/3, 665.920000*0+200, 6.000000, 0.25, (viewModels)1);
	DS.showParameter();
	//DS.showParameter();
	//Loop
	DS.run();
	return 0;
}