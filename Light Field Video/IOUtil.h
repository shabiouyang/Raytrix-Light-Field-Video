#pragma once
#include <opencv2\opencv.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdarg.h>
#include <Windows.h>
#include <direct.h>
#include <shlwapi.h>
#pragma comment(lib, "ShLwApi.lib")

using namespace cv;
using namespace std;

class IOUtil
{
private:

public:
	IOUtil();
	~IOUtil();

	static void writeLabelToCSV(int n, ...);

	static void writeToCSV(char *dataType, ...);

	static void readFromCSV(char *fileName, vector<CvPoint> &points);

	static void saveImg(string fileName, Mat &img, int fileNum);

	static bool makeDirectory(char *path, int folderNum = 0);
};

