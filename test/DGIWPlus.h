#pragma once
#include <Windows.h>
#include <comdef.h>
#include <gdiplus.h>
#include <map>


using namespace std;

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

class DGIWin32Plus
{
public:
    int   EncodeRGB2Jpeg(char* RGB, int width, int height, int LineWidth, char** Jpeg, int& JpegLen);
    DGIWin32Plus(void);
    ~DGIWin32Plus(void);

};