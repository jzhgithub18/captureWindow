#pragma once
#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")
class ScreenCapture9
{
private:
    //HWND _hwnd = NULL;
    int _initbitmap = 0;
    BITMAPINFO _bitmapinfo;
    HDC _hMemDC = NULL;
    HDC _hDC = NULL;
    HBITMAP _hMemDCBitmap = NULL;
    BYTE* _pPixel = NULL;
    HCURSOR _hcur;
    //vars
    IDirect3D9* g_pD3D = NULL;
    IDirect3DDevice9* g_pd3dDevice = NULL;
    IDirect3DSurface9* g_pSurface = NULL;
    HWND				ghWnd = NULL;
    RECT				gScreenRect = { 0,0,0,0 };
    UINT				gPitch = 0;
    void AddMouse(HDC hMemDC);
    HRESULT GrabImage(void* bitmap);
    //显示器个数
    UINT _DisplayCount = 1;
public:
    ScreenCapture9();
    ~ScreenCapture9();
    //第几个显示器
    HRESULT	InitD3D(HWND hWnd, int nDisplay);
    UINT GetPitch();
    UINT GetHeight();
    UINT GetWidth();

    BYTE* Grab2Hwnd();

    void Cleanup();
    int InitDisplayCount();
    //void test();
};