/*======================================================================*
* Layer pattern optimization by perspective view model
* Input: Multi-view image
*
* @file		Simulator > Source.cpp
* @author	S. Mikawa (Originally Mr.Kondo's program)
* @date		2018/02/dd
*======================================================================*/

#include "opencv2\opencv.hpp"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <omp.h>
#include <direct.h>
#include <Windows.h>
#include <fstream>
#include <direct.h>
#include <stdio.h>
#include <time.h>
#include "../Timer.h"
#include "../IOUtil.h"

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

#pragma CV_LIBRARY(core)
#pragma CV_LIBRARY(highgui)
#pragma CV_LIBRARY(imgcodecs)
#pragma CV_LIBRARY(imgproc)

using namespace cv;
using namespace std;

int focus = 200;	
int zoom = 136;		
double anglex = 33 / 2;	

double angley = anglex * 880.0 / 912.0;	
double camera2vcamera = -0.16*focus + 0.055*zoom + 127;
double camera2layer = focus;
double layerDis = camera2layer - camera2vcamera;	
double cameraInterval = 0.0013*focus - 0.0015*zoom - 0.0027;				
double Vwx = cameraInterval * 625 / (2 * tan(3.1415*anglex / 180)*layerDis);	
double Vwy = cameraInterval * 433 / (2 * tan(3.1415*angley / 180)*layerDis);	

int experimentNum = 1;
char pathName_mark[128] = "../LayerPattern/video/Scene_06/Perspective/Result_%03d";
//char fileName[128] = "../../Dataset/MultiView/Lytro/Scene_01/image_%03d.png";
char fileName_mark[128] = "../../RenderingRxMV/RenderingRxMV/frog_MV_modified/tree_frame%03d_%03d.png";
int INPUTh = 5;
int INPUTv = INPUTh;
int INPUTvh = INPUTh * INPUTv;
int layerNum = 3;				
int ITER = 5;					
int frame = 1;					
double layerInterval = 20;		
Mat input[25];					
Mat Y[300];
int x = 0;						
int y = 0;						
int ch = 0;						
double smallNumber = 0.00001;	
int exh = 1;					
int exv = 1;					
int he = exh * (INPUTh - 1);	
int ve = exv * (INPUTv - 1);	
enum viewModels { Orthographic, Perspective };
double processTime = 0;

void ImageRead(int j)
{
	//_%03d.png
	char fileName[128];

	char fullFileName_mark[128];
	for (int i = 0; i < INPUTvh; i++)
	{
		sprintf(fileName, fileName_mark, j, i+1);
		//string fullFileName = fullFileName_mark;
		//cout << fileName << endl;
		input[i] = imread(fileName, 1);
	}

	x = input[0].cols;
	y = input[0].rows;
	ch = input[0].channels();
}

Mat ReapY(Mat X)
{
	for (int j = 0; j < X.rows; j++)
	{
		for (int k = 0; k < X.cols; k++)
		{
			for (int CH = 0; CH < ch; CH++)
			{
				if (X.at<Vec3d>(j, k)[CH] < 0.001)
				{
					X.at<Vec3d>(j, k)[CH] = 0.001;
				}
			}
		}
	}

	return X;
}

Mat ReapABC(Mat X)
{
	for (int j = 0; j < X.rows; j++)
	{
		for (int k = 0; k < X.cols; k++)
		{
			for (int CH = 0; CH < ch; CH++)
			{
				if (X.at<Vec3d>(j, k)[CH] > 1.0)
				{
					X.at<Vec3d>(j, k)[CH] = 1.0;
				}
			}
		}
	}

	return X;
}

Mat AddSmallNumber(Mat X)
{
	for (int i = 0; i < X.channels(); i++)
	{
		for (int j = 0; j < X.rows; j++)
		{
			for (int k = 0; k < X.cols; k++)
			{
				X.at<Vec3d>(j, k)[i] += smallNumber;
			}
		}
	}

	return X;
}

void LayImage(Mat A, Mat B, Mat C, int j)
{
	char pathName[128];
	sprintf(pathName, pathName_mark, j);

	Mat_<unsigned char>uchrA(A.rows, A.cols, 3);
	Mat_<unsigned char>uchrB(B.rows, B.cols, 3);
	Mat_<unsigned char>uchrC(C.rows, C.cols, 3);

	uchrA = A * 255;
	uchrB = B * 255;
	uchrC = C * 255;

	for (int i = 0; i < frame; i++)
	{
		char fullFileNameA[128];
		char fullFileNameB[128];
		char fullFileNameC[128];
		char fileNameA[128];
		char fileNameB[128];
		char fileNameC[128];
		sprintf(fullFileNameA, pathName, experimentNum);
		sprintf(fullFileNameB, pathName, experimentNum);
		sprintf(fullFileNameC, pathName, experimentNum);
		sprintf(fileNameA, "/layer_%d.png", i + 1);
		sprintf(fileNameB, "/layer_%d.png", frame + i + 1);
		sprintf(fileNameC, "/layer_%d.png", 2 * frame + i + 1);
		strcat_s(fullFileNameA, fileNameA);
		strcat_s(fullFileNameB, fileNameB);
		strcat_s(fullFileNameC, fileNameC);
		imwrite(fullFileNameA, uchrA.rowRange(i*y, (i + 1)*y));
		imwrite(fullFileNameB, uchrB.rowRange(i*y, (i + 1)*y));
		imwrite(fullFileNameC, uchrC.rowRange(i*y, (i + 1)*y));
	}
}

double Interpolation(Mat X, int xI, int yI, double xd, double yd, int CH)
{
	double v1 = X.at<Vec3d>(yI, xI)[CH] * (1 - yd) * (1 - xd);
	double v2 = X.at<Vec3d>((yI + 1), xI)[CH] * yd * (1 - xd);
	double v3 = X.at<Vec3d>(yI, (xI + 1))[CH] * (1 - yd) * xd;
	double v4 = X.at<Vec3d>((yI + 1), (xI + 1))[CH] * yd * xd;
	double v = v1 + v2 + v3 + v4;

	return v;
}

Mat Anumerator(Mat B, Mat C)
{
	Mat X = Mat::zeros(y*frame, x, CV_64FC3);

	double az = layerDis - layerInterval;		
	double bz = layerDis;			
	double cz = layerDis + layerInterval;		

	for (int i = 0; i < INPUTv; i++)
	{
		
		double campositonY = y / 2 + Vwy * ((INPUTv - 1) / 2 - i);

		for (int j = 0; j < INPUTh; j++)
		{
			double campositonX = x / 2 - Vwx * ((INPUTh - 1) / 2 - j);
			Mat Y2 = Mat::zeros(y*frame, x, CV_64FC3);

			for (int k = 0; k < frame; k++)
			{
				for (int CH = 0; CH < 3; CH++)
				{
					for (int q = 0; q < y; q++)
					{
						double slopey = -az / (campositonY - q);

						double byi;	
						double byd = modf(bz / slopey + campositonY, &byi);	
						int byI = (int)byi + y*k;

						double cyi;	
						double cyd = modf(cz / slopey + campositonY, &cyi);	
						int cyI = (int)cyi + y*k;

						double inyi;
						double inyd = modf(layerDis / slopey + y / 2 - layerDis / (layerDis / (y / 2 - campositonY)), &inyi);
						int inyI = (int)inyi;

						if (cyI >= 0 + y*k&& cyI < y - 1 + y*k && inyI >= 0 && inyI < y - 1)
						{
							for (int p = 0; p < x; p++)
							{
								double slopex = -az / (campositonX - p);

								double bxi;
								double bxd = modf(bz / slopex + campositonX, &bxi);
								int bxI = (int)bxi;

								double cxi;
								double cxd = modf(cz / slopex + campositonX, &cxi);
								int cxI = (int)cxi;

								double inxi;
								double inxd = modf(layerDis / slopex + x / 2 - layerDis / (layerDis / (x / 2 - campositonX)), &inxi);
								int inxI = (int)inxi;

								if (cxI >= 0 && cxI < x - 1 && inxI >= 0 && inxI < x - 1)
								{
									double bv = Interpolation(B, bxI, byI, bxd, byd, CH);
									double cv = Interpolation(C, cxI, cyI, cxd, cyd, CH);
									double inv = Interpolation(Y[j + INPUTh*i], inxI, inyI, inxd, inyd, CH);

									Y2.at<Vec3d>(q + y*k, p)[CH] = inv * bv * cv;
								}
							}
						}
					}
				}
			}
			X = Y2 + X;
		}
	}

	return X;
}

Mat Bnumerator(Mat A, Mat C)
{
	Mat X = Mat::zeros(y*frame, x, CV_64FC3);

	double az = layerDis - layerInterval;
	double bz = layerDis;
	double cz = layerDis + layerInterval;


	for (int i = 0; i < INPUTv; i++) {

		double campositonY = y / 2 + Vwy * ((INPUTv - 1) / 2 - i);

		for (int j = 0; j < INPUTh; j++) {

			double campositonX = x / 2 - Vwx * ((INPUTh - 1) / 2 - j);

			//printf("camX=%f, camY=%f , j=%d, i=%d\n", campositonX, campositonY, j, i);

			Mat Y2 = Mat::zeros(y*frame, x, CV_64FC3);

			for (int k = 0; k < frame; k++) {
				for (int CH = 0; CH < 3; CH++) {
					for (int q = 0; q < y - 1; q++) {

						double slopey = -bz / (campositonY - q);

						double ayi;
						double ayd = modf(az / slopey + campositonY, &ayi);
						int ayI = (int)ayi + y*k;

						double cyi;
						double cyd = modf(cz / slopey + campositonY, &cyi);
						int cyI = (int)cyi + y*k;

						double inyi;
						double inyd = modf(layerDis / slopey + y / 2 - layerDis / (layerDis / (y / 2 - campositonY)), &inyi);

						int inyI = (int)inyi;

						if (cyI >= 0 + y*k && cyI < y - 1 + y*k && inyI >= 0 && inyI < y - 1) {

							for (int p = 0; p < x - 1; p++) {

								double slopex = -bz / (campositonX - p);

								double axi;
								double axd = modf(az / slopex + campositonX, &axi);
								int axI = (int)axi;

								double cxi;
								double cxd = modf(cz / slopex + campositonX, &cxi);
								int cxI = (int)cxi;

								double inxi;
								double inxd = modf(layerDis / slopex + x / 2 - layerDis / (layerDis / (x / 2 - campositonX)), &inxi);
								int inxI = (int)inxi;

								if (cxI >= 0 && cxI < x - 1 && inxI >= 0 && inxI < x - 1) {
									//if(i==6&&j==6)printf("axI=%d,ayI=%d,", axI, ayI);
									double av = Interpolation(A, axI, ayI, axd, ayd, CH);
									//if (i == 6 && j == 6)printf("cxI=%d,cyI=%d,", cxI, cyI);
									double cv = Interpolation(C, cxI, cyI, cxd, cyd, CH);
									//if (i == 6 && j == 6)printf("inxI=%d,inyI=%d\n", inxI, inyI);
									double inv = Interpolation(Y[j + INPUTh*i], inxI, inyI, inxd, inyd, CH);
									//if (i == 6 && j == 6)printf("Y2xI=%d,Y2yI=%d\n", q + ExtentionHeight / 2, p + ExtentionWidth / 2);
									//Y2.at<Vec3d>(q + i, p + j)[CH] = Y[i*INPUTh + j].at<Vec3d>(q, p)[CH] * B.at<Vec3d>(q + ve - i, p + he - j)[CH];
									Y2.at<Vec3d>(q + y*k, p)[CH] = inv * av * cv;
								}
							}
						}
					}
				}
			}

			X = Y2 + X;

		}
	}
	return X;
}

Mat Cnumerator(Mat A, Mat B)
{
	Mat X = Mat::zeros(y*frame, x, CV_64FC3);

	double az = layerDis - layerInterval;
	double bz = layerDis;
	double cz = layerDis + layerInterval;


	for (int i = 0; i < INPUTv; i++) {

		double campositonY = y / 2 + Vwy * ((INPUTv - 1) / 2 - i);

		for (int j = 0; j < INPUTh; j++) {

			double campositonX = x / 2 - Vwx * ((INPUTh - 1) / 2 - j);

			//printf("camX=%f, camY=%f , j=%d, i=%d\n", campositonX, campositonY, j, i);

			Mat Y2 = Mat::zeros(y*frame, x, CV_64FC3);

			for (int k = 0; k < frame; k++) {
				for (int CH = 0; CH < 3; CH++) {
					for (int q = 0; q < y - 1; q++) {

						double slopey = -cz / (campositonY - q);

						double ayi;
						double ayd = modf(az / slopey + campositonY, &ayi);
						int ayI = (int)ayi + y*k;

						double byi;
						double byd = modf(bz / slopey + campositonY, &byi);
						int byI = (int)byi + y*k;

						double inyi;
						double inyd = modf(layerDis / slopey + y / 2 - layerDis / (layerDis / (y / 2 - campositonY)), &inyi);
						int inyI = (int)inyi;


						if (inyI >= 0 && inyI < y - 1) {

							for (int p = 0; p < x - 1; p++) {


								double slopex = -cz / (campositonX - p);

								double axi;
								double axd = modf(az / slopex + campositonX, &axi);
								int axI = (int)axi;

								double bxi;
								double bxd = modf(bz / slopex + campositonX, &bxi);
								int bxI = (int)bxi;

								double inxi;
								double inxd = modf(layerDis / slopex + x / 2 - layerDis / (layerDis / (x / 2 - campositonX)), &inxi);
								int inxI = (int)inxi;


								if (inxI >= 0 && inxI < x - 1) {

									double av = Interpolation(A, axI, ayI, axd, ayd, CH);
									double bv = Interpolation(B, bxI, byI, bxd, byd, CH);
									double inv = Interpolation(Y[j + INPUTh*i], inxI, inyI, inxd, inyd, CH);

									//Y2.at<Vec3d>(q + i, p + j)[CH] = Y[i*INPUTh + j].at<Vec3d>(q, p)[CH] * B.at<Vec3d>(q + ve - i, p + he - j)[CH];
									Y2.at<Vec3d>(q + y*k, p)[CH] = inv * av * bv;
								}
							}
						}
					}
				}
			}

			X = Y2 + X;

		}
	}

	return X;
}

Mat Adenominato(Mat A, Mat B, Mat C)
{
	Mat X = Mat::zeros(y*frame, x, CV_64FC3);

	double az = layerDis - layerInterval;
	double bz = layerDis;
	double cz = layerDis + layerInterval;


	for (int i = 0; i < INPUTv; i++) {

		double campositonY = y / 2 + Vwy * ((INPUTv - 1) / 2 - i);

		for (int j = 0; j < INPUTh; j++) {

			double campositonX = x / 2 - Vwx * ((INPUTh - 1) / 2 - j);
			//printf("ABCcamX=%f, camY=%f , j=%d, i=%d\n", campositonX, campositonY, j, i);
			Mat Y2 = Mat::zeros(y*frame, x, CV_64FC3);
			Mat emit2 = Mat::zeros(y, x, CV_64FC3);

			for (int k = 0; k < frame; k++) {
				for (int CH = 0; CH < 3; CH++) {
					for (int q = 0; q < y; q++) {

						double slopey = -az / (campositonY - q);

						double byi;
						double byd = modf(bz / slopey + campositonY, &byi);
						int byI = (int)byi + y*k;

						double cyi;
						double cyd = modf(cz / slopey + campositonY, &cyi);
						int cyI = (int)cyi + y*k;

						if (cyI >= 0 + y*k&& cyI < y - 1 + y*k) {

							for (int p = 0; p < x; p++) {

								double slopex = -az / (campositonX - p);

								double bxi;
								double bxd = modf(bz / slopex + campositonX, &bxi);
								int bxI = (int)bxi;

								double cxi;
								double cxd = modf(cz / slopex + campositonX, &cxi);
								int cxI = (int)cxi;


								if (cxI >= 0 && cxI < x - 1) {
									double bv = Interpolation(B, bxI, byI, bxd, byd, CH);
									double cv = Interpolation(C, cxI, cyI, cxd, cyd, CH);

									emit2.at<Vec3d>(q, p)[CH] += A.at<Vec3d>(q + y*k, p)[CH] * bv * cv;
								}
								//Y2.at<Vec3d>(q + i, p + he - j)[CH] = A.at<Vec3d>(q + i, p + he - j)[CH] * B.at<Vec3d>(q + ve - i, p + j)[CH] * B.at<Vec3d>(q + ve - i, p + j)[CH];
							}
						}
					}
				}
			}

			for (int k = 0; k < frame; k++) {
				for (int CH = 0; CH < 3; CH++) {
					for (int q = 0; q < y; q++) {

						double slopey = -az / (campositonY - q);

						double byi;
						double byd = modf(bz / slopey + campositonY, &byi);
						int byI = (int)byi + y*k;

						double cyi;
						double cyd = modf(cz / slopey + campositonY, &cyi);
						int cyI = (int)cyi + y*k;

						if (cyI >= 0 + y*k&& cyI < y - 1 + y*k) {

							for (int p = 0; p < x; p++) {

								double slopex = -az / (campositonX - p);

								double bxi;
								double bxd = modf(bz / slopex + campositonX, &bxi);
								int bxI = (int)bxi;

								double cxi;
								double cxd = modf(cz / slopex + campositonX, &cxi);
								int cxI = (int)cxi;


								if (cxI >= 0 && cxI < x - 1) {
									double bv = Interpolation(B, bxI, byI, bxd, byd, CH);
									double cv = Interpolation(C, cxI, cyI, cxd, cyd, CH);

									Y2.at<Vec3d>(q + y*k, p)[CH] += emit2.at<Vec3d>(q, p)[CH] * bv * cv;
								}
								//Y2.at<Vec3d>(q + i, p + he - j)[CH] = A.at<Vec3d>(q + i, p + he - j)[CH] * B.at<Vec3d>(q + ve - i, p + j)[CH] * B.at<Vec3d>(q + ve - i, p + j)[CH];
							}
						}
					}
				}
			}





			X = Y2 + X;
		}
	}

	AddSmallNumber(X);

	return X;
}

Mat Bdenominato(Mat A, Mat B, Mat C)
{
	Mat X = Mat::zeros(y*frame, x, CV_64FC3);

	double az = layerDis - layerInterval;
	double bz = layerDis;
	double cz = layerDis + layerInterval;


	for (int i = 0; i < INPUTv; i++) {

		double campositonY = y / 2 + Vwy * ((INPUTv - 1) / 2 - i);

		for (int j = 0; j < INPUTh; j++) {

			double campositonX = x / 2 - Vwx * ((INPUTh - 1) / 2 - j);

			Mat Y2 = Mat::zeros(y*frame, x, CV_64FC3);
			Mat emit2 = Mat::zeros(y, x, CV_64FC3);

			for (int k = 0; k < frame; k++) {
				for (int CH = 0; CH < 3; CH++) {
					for (int q = 0; q < y - 1; q++) {

						double slopey = -bz / (campositonY - q);

						double ayi;
						double ayd = modf(az / slopey + campositonY, &ayi);
						int ayI = (int)ayi + y*k;

						double cyi;
						double cyd = modf(cz / slopey + campositonY, &cyi);
						int cyI = (int)cyi + y*k;

						if (cyI >= 0 + y*k&& cyI < y - 1 + y*k) {

							for (int p = 0; p < x - 1; p++) {

								double slopex = -bz / (campositonX - p);


								double axi;
								double axd = modf(az / slopex + campositonX, &axi);
								int axI = (int)axi;

								double cxi;
								double cxd = modf(cz / slopex + campositonX, &cxi);
								int cxI = (int)cxi;



								if (cxI >= 0 && cxI < x - 1) {

									double av = Interpolation(A, axI, ayI, axd, ayd, CH);
									double cv = Interpolation(C, cxI, cyI, cxd, cyd, CH);

									//Y2.at<Vec3d>(q + i, p + he - j)[CH] = A.at<Vec3d>(q + i, p + he - j)[CH] * B.at<Vec3d>(q + ve - i, p + j)[CH] * B.at<Vec3d>(q + ve - i, p + j)[CH];
									emit2.at<Vec3d>(q, p)[CH] += B.at<Vec3d>(q + y*k, p)[CH] * av * cv;
								}
							}
						}
					}
				}
			}

			for (int k = 0; k < frame; k++) {
				for (int CH = 0; CH < 3; CH++) {
					for (int q = 0; q < y - 1; q++) {

						double slopey = -bz / (campositonY - q);

						double ayi;
						double ayd = modf(az / slopey + campositonY, &ayi);
						int ayI = (int)ayi + y*k;

						double cyi;
						double cyd = modf(cz / slopey + campositonY, &cyi);
						int cyI = (int)cyi + y*k;

						if (cyI >= 0 + y*k&& cyI < y - 1 + y*k) {

							for (int p = 0; p < x - 1; p++) {

								double slopex = -bz / (campositonX - p);


								double axi;
								double axd = modf(az / slopex + campositonX, &axi);
								int axI = (int)axi;

								double cxi;
								double cxd = modf(cz / slopex + campositonX, &cxi);
								int cxI = (int)cxi;



								if (cxI >= 0 && cxI < x - 1) {

									double av = Interpolation(A, axI, ayI, axd, ayd, CH);
									double cv = Interpolation(C, cxI, cyI, cxd, cyd, CH);

									Y2.at<Vec3d>(q + y*k, p)[CH] += emit2.at<Vec3d>(q, p)[CH] * av * cv;
								}
								//Y2.at<Vec3d>(q + i, p + he - j)[CH] = A.at<Vec3d>(q + i, p + he - j)[CH] * B.at<Vec3d>(q + ve - i, p + j)[CH] * B.at<Vec3d>(q + ve - i, p + j)[CH];
							}
						}
					}
				}
			}

			X = Y2 + X;
		}
	}

	AddSmallNumber(X);

	return X;
}

Mat Cdenominato(Mat A, Mat B, Mat C)
{
	Mat X = Mat::zeros(y*frame, x, CV_64FC3);

	double az = layerDis - layerInterval;
	double bz = layerDis;
	double cz = layerDis + layerInterval;


	for (int i = 0; i < INPUTv; i++) {

		double campositonY = y / 2 + Vwy * ((INPUTv - 1) / 2 - i);

		for (int j = 0; j < INPUTh; j++) {

			double campositonX = x / 2 - Vwx * ((INPUTh - 1) / 2 - j);

			Mat Y2 = Mat::zeros(y*frame, x, CV_64FC3);
			Mat emit2 = Mat::zeros(y, x, CV_64FC3);


			for (int k = 0; k < frame; k++) {
				for (int CH = 0; CH < 3; CH++) {
					for (int q = 0; q < y - 1; q++) {

						double slopey = -cz / (campositonY - q);

						double ayi;
						double ayd = modf(az / slopey + campositonY, &ayi);
						int ayI = (int)ayi + y*k;

						double byi;
						double byd = modf(bz / slopey + campositonY, &byi);
						int byI = (int)byi + y*k;

						for (int p = 0; p < x - 1; p++) {

							double slopex = -cz / (campositonX - p);

							double axi;
							double axd = modf(az / slopex + campositonX, &axi);
							int axI = (int)axi;

							double bxi;
							double bxd = modf(bz / slopex + campositonX, &bxi);
							int bxI = (int)bxi;

							if (bxI >= 0 && bxI < x - 1) {

								double av = Interpolation(A, axI, ayI, axd, ayd, CH);
								double bv = Interpolation(B, bxI, byI, bxd, byd, CH);

								//Y2.at<Vec3d>(q + i, p + he - j)[CH] = A.at<Vec3d>(q + i, p + he - j)[CH] * B.at<Vec3d>(q + ve - i, p + j)[CH] * B.at<Vec3d>(q + ve - i, p + j)[CH];
								emit2.at<Vec3d>(q, p)[CH] += C.at<Vec3d>(q + y*k, p)[CH] * av * bv;

							}
						}
					}
				}
			}
			for (int k = 0; k < frame; k++) {
				for (int CH = 0; CH < 3; CH++) {
					for (int q = 0; q < y - 1; q++) {

						double slopey = -cz / (campositonY - q);

						double ayi;
						double ayd = modf(az / slopey + campositonY, &ayi);
						int ayI = (int)ayi + y*k;

						double byi;
						double byd = modf(bz / slopey + campositonY, &byi);
						int byI = (int)byi + y*k;

						for (int p = 0; p < x - 1; p++) {

							double slopex = -cz / (campositonX - p);

							double axi;
							double axd = modf(az / slopex + campositonX, &axi);
							int axI = (int)axi;

							double bxi;
							double bxd = modf(bz / slopex + campositonX, &bxi);
							int bxI = (int)bxi;

							if (bxI >= 0 && bxI < x - 1) {

								double av = Interpolation(A, axI, ayI, axd, ayd, CH);
								double bv = Interpolation(B, bxI, byI, bxd, byd, CH);

								Y2.at<Vec3d>(q + y*k, p)[CH] += emit2.at<Vec3d>(q, p)[CH] * av * bv;
							}

						}
					}
				}
			}


			X = Y2 + X;
		}
	}

	AddSmallNumber(X);

	return X;
}

void Solve(int j)
{
	Mat A = Mat::zeros(y*frame, x, CV_64FC3);
	Mat B = Mat::zeros(y*frame, x, CV_64FC3);
	Mat C = Mat::zeros(y*frame, x, CV_64FC3);

	Mat a[3];
	Mat cameraInterval[3];
	Mat c[3];

	for (int i = 0; i < 3; i++)
	{
		a[i] = Mat::zeros(A.rows, A.cols, CV_64FC1);
		cameraInterval[i] = Mat::zeros(B.rows, B.cols, CV_64FC1);
		c[i] = Mat::zeros(C.rows, C.cols, CV_64FC1);
	}

	vector<Mat>mrgA;
	vector<Mat>mrgB;
	vector<Mat>mrgC;

	for (int i = 0; i < INPUTvh; i++)
	{
		input[i].convertTo(Y[i], CV_64FC3);
		Y[i] = (Y[i] * frame) / 255.0;
		ReapY(Y[i]);
	}

	//ƒ‰ƒ“ƒ_ƒ€ƒmƒCƒY‚Å‰Šú‰»
	for (int i = 0; i < ch; i++) {
		randu(a[i], Scalar(1.0), Scalar(0.001));
		randu(cameraInterval[i], Scalar(1.0), Scalar(0.001));
		randu(c[i], Scalar(1.0), Scalar(0.001));

		mrgA.push_back(a[i]);
		mrgB.push_back(cameraInterval[i]);
		mrgC.push_back(c[i]);
	}

	merge(mrgA, A);
	merge(mrgB, B);
	merge(mrgC, C);

	for (int iter = 0; iter < ITER; iter++)
	{
		printf("ITER=%d\t", iter);

		A = A.mul(Anumerator(B, C) / (Adenominato(A, B, C)));
		ReapABC(A);
		printf("layerA finish\t");

		B = B.mul(Bnumerator(A, C) / (Bdenominato(A, B, C)));
		ReapABC(B);
		printf("layerB finish\t");

		C = C.mul(Cnumerator(A, B) / (Cdenominato(A, B, C)));
		ReapABC(C);
		printf("layerC finish\n");
	}

	LayImage(A, B, C, j);
}

void writeConfig(char *fileName)
{
	char buf[128];
	char *sectionName[] = { "Layer" , "Optimization", "Camera" };
	ofstream ofs(fileName);

	sprintf(buf, "%d", layerNum);
	WritePrivateProfileString(sectionName[0], "layerNum", buf, fileName);
	sprintf(buf, "%f", layerInterval);
	WritePrivateProfileString(sectionName[0], "layerInterval", buf, fileName);

	sprintf(buf, "%d", Perspective);
	WritePrivateProfileString(sectionName[1], "viewModel", buf, fileName);
	sprintf(buf, "%d", frame);
	WritePrivateProfileString(sectionName[1], "timeDivision", buf, fileName);
	sprintf(buf, "%d", INPUTvh);
	WritePrivateProfileString(sectionName[1], "imageNum", buf, fileName);
	sprintf(buf, "%d", x);
	WritePrivateProfileString(sectionName[1], "imageSize.width", buf, fileName);
	sprintf(buf, "%d", y);
	WritePrivateProfileString(sectionName[1], "imageSize.height", buf, fileName);
	sprintf(buf, "%d", ITER);
	WritePrivateProfileString(sectionName[1], "iteration", buf, fileName);
	sprintf(buf, "%f", processTime);
	WritePrivateProfileString(sectionName[1], "processTime", buf, fileName);

	sprintf(buf, "%d", focus);
	WritePrivateProfileString(sectionName[2], "focus", buf, fileName);
	sprintf(buf, "%d", zoom);
	WritePrivateProfileString(sectionName[2], "zoom", buf, fileName);
	sprintf(buf, "%f", cameraInterval);
	WritePrivateProfileString(sectionName[2], "cameraInterval", buf, fileName);
	sprintf(buf, "%f", camera2vcamera);
	WritePrivateProfileString(sectionName[2], "camera2vcamera", buf, fileName);
	sprintf(buf, "%f", Vwx);
	WritePrivateProfileString(sectionName[2], "vcamera2vcamera.x", buf, fileName);
	sprintf(buf, "%f", Vwy);
	WritePrivateProfileString(sectionName[2], "vcamera2vcamera.y", buf, fileName);
	sprintf(buf, "%f", camera2layer);
	WritePrivateProfileString(sectionName[2], "camera2layer", buf, fileName);
	sprintf(buf, "%f", layerDis);
	WritePrivateProfileString(sectionName[2], "vcamera2layer", buf, fileName);
	sprintf(buf, "%f", anglex * 2);
	WritePrivateProfileString(sectionName[2], "fieldOfViewAngle.x", buf, fileName);
	sprintf(buf, "%f", angley * 2);
	WritePrivateProfileString(sectionName[2], "fieldOfViewAngle.y", buf, fileName);
}

int main(void)
{
	
	for (int j = 1; j <= 1; ++j)
	{
		cout << "Processing Frame " << j << endl;
		char pathName[128];
		sprintf(pathName, pathName_mark, j);

		ImageRead(j);
		IOUtil::makeDirectory(pathName, experimentNum);

		Timer TIMER(sec, false);
		Solve(j);
		processTime = TIMER.getTime();

		char confFileName[128];
		sprintf(confFileName, pathName, experimentNum);
		strcat_s(confFileName, "/config.ini");
		writeConfig(confFileName);
	}
	
	return 0;
}