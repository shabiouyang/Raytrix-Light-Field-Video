#include "IOUtil.h"

IOUtil::IOUtil()
{
}

IOUtil::~IOUtil()
{
}

void IOUtil::writeLabelToCSV(int n, ...)
{
	va_list list;
	int i;
	char *str;

	va_start(list, n);

	ofstream ofs(va_arg(list, char*));

	for (i = 1; i < n+1; i++)
	{
		str = va_arg(list, char*);
		ofs << str << ",";
	}

	if (n != 0)
	{
		ofs << endl;
	}

	va_end(list);
}

void IOUtil::writeToCSV(char *dataType, ...)
{
	va_list list;
	char *str;

	va_start(list, dataType);

	ofstream ofs(va_arg(list, char*), ios_base::app);

	for (str = (char *)dataType; *str != '\0'; str++)
	{
		if (*str == 's')
		{
			ofs << va_arg(list, char *) << ",";
		}
		else if (*str == 'c')
		{
			ofs << (char)va_arg(list, int) << ",";
		}
		else if (*str == 'd')
		{
			ofs << va_arg(list, int) << ",";
		}
		else if (*str == 'f')
		{
			ofs << va_arg(list, double) << ",";
		}
		else if (*str == 'n')
		{
			ofs << endl;
		}
	}

	va_end(list);
}

void IOUtil::readFromCSV(char *fileName, vector<CvPoint> &points)
{
	char fname[256] = { '\0' };
	char *extension = ".csv";
	sprintf_s(fname, "%s%s", fileName, extension);
	ifstream ifs(fname);

	string oneLine, token;
	stringstream ss1, ss2;
	CvPoint point;
	bool one;

	if (!ifs)
	{
		cout << "Error:Input data file not found" << endl;
	}
	else
	{
		while (getline(ifs, oneLine))
		{
			istringstream stream(oneLine);

			one = true;
			while (getline(stream, token, ','))
			{
				if (one)
				{
					point.x = atoi(token.c_str());
					one = false;
				}
				else
				{
					point.y = atoi(token.c_str());
					points.push_back(point);
				}
			}
		}
	}
}

void IOUtil::saveImg(string fileName, Mat &img, int fileNum)
{
	static int loopCnt = 0;

	if (loopCnt < fileNum)
	{
		stringstream fullFileName;
		string extension = ".jpg";
		int digit = (int)log10((double)fileNum) + 1;
		fullFileName.str("");
		fullFileName << fileName << setw(digit) << setfill('0') << loopCnt << extension;
		imwrite(fullFileName.str(), img);
		loopCnt++;
	}
}

bool IOUtil::makeDirectory(char *path, int folderNum)
{
	//Check PathName Length
	if (strlen(path) > 100)
	{
		cout << "Path Name is too Long" << endl;
		return false;
	}

	//Add Number to FolerName
	char fullPath[128];
	if (folderNum > 0)
	{
		sprintf_s(fullPath, path, folderNum);
	}
	else
	{
		strcpy_s(fullPath, path);
	}

	//Make Multi-Directory
	while (!PathIsDirectory(fullPath))
	{
		char popBackFullPath[128];
		strcpy_s(popBackFullPath, fullPath);
		while (_mkdir(popBackFullPath) == -1)
		{
			int len = strlen(popBackFullPath);
			if (len == 0)
			{
				return false;
			}
			popBackFullPath[len - 1] = '\0';
		}
	}

	return true;
}
