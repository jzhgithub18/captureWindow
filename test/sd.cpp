#include "sd.h"
#include <stdio.h>
//#include <>
////#include "DGIWPlus.h"
//#include <d3dx9.h>
#define ALL_APP TRUE
ScreenCapture9::ScreenCapture9()
{
	CURSORINFO globalCursor;
	globalCursor.cbSize = sizeof(CURSORINFO); // could cache I guess...
	::GetCursorInfo(&globalCursor);
	_hcur = globalCursor.hCursor;
	//GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
}


ScreenCapture9::~ScreenCapture9()
{
	//GdiplusShutdown(m_gdiplusToken);
}

int ScreenCapture9::InitDisplayCount()
{
	if (g_pD3D == NULL)
	{
		if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		{
			printf_s("Unable to Create Direct3D\n");
			return E_FAIL;
		}
	}
	_DisplayCount = g_pD3D->GetAdapterCount();
	return _DisplayCount;
}


HRESULT	ScreenCapture9::InitD3D(HWND hWnd, int nDisplay)
{

	D3DDISPLAYMODE	ddm;
	D3DPRESENT_PARAMETERS	d3dpp;
	if (g_pD3D == NULL)
	{
		if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		{
			printf_s("Unable to Create Direct3D\n");
			return E_FAIL;
		}
		_DisplayCount = g_pD3D->GetAdapterCount();
	}

	if (nDisplay > _DisplayCount - 1)
	{
		nDisplay = D3DADAPTER_DEFAULT;
	}

	if (FAILED(g_pD3D->GetAdapterDisplayMode(nDisplay, &ddm)))
	{
		printf_s("Unable to Get Adapter Display Mode\n");
		return E_FAIL;
	}

	//g_pD3D->setA
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	d3dpp.Windowed = true;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.BackBufferFormat = ddm.Format;
	
#if ALL_APP
    int width = ddm.Width; // rect.right - rect.left;
    int height = ddm.Height; // rect.bottom - rect.top;
	d3dpp.BackBufferHeight = gScreenRect.bottom = ddm.Height;
	d3dpp.BackBufferWidth = gScreenRect.right = ddm.Width;
#else
	GetClientRect(hWnd, &gScreenRect);
	//gScreenRect = rect;
	d3dpp.BackBufferHeight = GetHeight();// rect.bottom - rect.top;
	d3dpp.BackBufferWidth = GetWidth();  // rect.right - rect.left;
#endif

	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.BackBufferCount = 1;

	if (FAILED(g_pD3D->CreateDevice(nDisplay,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		//D3DCREATE_HARDWARE_VERTEXPROCESSING , 
		&d3dpp, &g_pd3dDevice)))
	{
		printf_s("Unable to Create Device\n");
		return E_FAIL;
	}

	ghWnd = hWnd;

#if ALL_APP
	if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(GetWidth(),GetHeight(), D3DFMT_A8R8G8B8,
		D3DPOOL_SYSTEMMEM/*D3DPOOL_SCRATCH*/, &g_pSurface, NULL)))
	{
		printf_s("Unable to Create Surface\n");
		return E_FAIL;
	}

	D3DLOCKED_RECT	lockedRect;

	if (FAILED(g_pSurface->LockRect(&lockedRect, NULL, 0)))					// compute the required buffer size
	{
		printf_s("Unable to Lock Surface\n");
		return E_FAIL;
	}
	gPitch = lockedRect.Pitch;
	if (FAILED(g_pSurface->UnlockRect()))
	{
		printf_s("Unable to Unlock Surface\n");
		return E_FAIL;
	}
#endif
	return S_OK;
}

UINT ScreenCapture9::GetPitch()
{
	return gPitch;
}

UINT ScreenCapture9::GetHeight()
{
	return gScreenRect.bottom - gScreenRect.top;
}

UINT ScreenCapture9::GetWidth()
{
	return gScreenRect.right - gScreenRect.left;
}


void ScreenCapture9::AddMouse(HDC hMemDC) {
	//	__int64 start = StartCounter();
	POINT p;

	GetCursorPos(&p);
	ICONINFO iconinfo;
	BOOL ret = ::GetIconInfo(_hcur, &iconinfo); // 0.09ms

	if (ret) {
		p.x -= iconinfo.xHotspot; // align mouse, I guess...
		p.y -= iconinfo.yHotspot;

		// avoid some memory leak or other...
		if (iconinfo.hbmMask) {
			::DeleteObject(iconinfo.hbmMask);
		}
		if (iconinfo.hbmColor) {
			::DeleteObject(iconinfo.hbmColor);
		}
	}

	DrawIcon(hMemDC, p.x, p.y, _hcur); // 0.042ms
									   //if (show_performance)
									   //	LocalOutput("add mouse took %.02f ms", GetCounterSinceStartMillis(start)); // sum takes around 0.125 ms
}

HRESULT ScreenCapture9::GrabImage(void* pBits)
{
	if (!g_pd3dDevice) {
		return E_FAIL;
	}

#if ALL_APP
	if (FAILED(g_pd3dDevice->GetFrontBufferData(0, g_pSurface)))
	{
		printf_s("Unable to get Buffer Surface Data\n");
		return E_FAIL;
	}
#else
    if (FAILED(g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_pSurface)))
    {
        printf_s("Unable to get Buffer Surface Data\n");
        return E_FAIL;
    }
#endif
	/*static int i = 0;
	TCHAR buffer[256];
	wsprintf(buffer, L"CaptureScreenByDirectDraw%d.bmp",i);*/
	//D3DXSaveSurfaceToFile(buffer, D3DXIFF_BMP, g_pSurface, NULL, NULL);
	//HDC hdc;
	//HRESULT hok = g_pSurface->GetDC(&hdc);
	//if (hok == S_OK)
	//{
		//AddMouse(hdc);
	//}

	D3DLOCKED_RECT	lockedRect;
    /*if (FAILED(g_pSurface->LockRect(&lockedRect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY)))
    {
        printf_s("Unable to Lock Front Buffer Surface\n");
        return E_FAIL;
    }*/
    if (FAILED(g_pSurface->LockRect(&lockedRect, NULL, 0)))
    {
        printf_s("Unable to Lock Front Buffer Surface\n");
        return E_FAIL;
    }
    gPitch = lockedRect.Pitch;
    if (_initbitmap == 0)
    {
        int biBitCountchar = 32;

        HDC hDC = GetDC(ghWnd);
        _initbitmap = 1;
        ZeroMemory(&_bitmapinfo, sizeof(BITMAPINFO));
        BITMAPINFOHEADER& bmiHeader = _bitmapinfo.bmiHeader;
        bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmiHeader.biWidth = lockedRect.Pitch/4;
        bmiHeader.biHeight = 0 - GetHeight();
        bmiHeader.biPlanes = 1;
        bmiHeader.biBitCount = biBitCountchar; // 32  24; 
        bmiHeader.biXPelsPerMeter = 0;
        bmiHeader.biYPelsPerMeter = 0;
        bmiHeader.biSizeImage = GetPitch() * GetHeight();
        bmiHeader.biClrUsed = 0;
        bmiHeader.biClrImportant = 0;

        _hMemDC = CreateCompatibleDC(hDC);
        _hMemDCBitmap = CreateDIBSection(_hMemDC, &_bitmapinfo, DIB_RGB_COLORS
            , (void**)&_pPixel, NULL, 0);

        SelectObject(_hMemDC, _hMemDCBitmap);
        DeleteDC(hDC);
    }
	errno_t err;
	int height =GetHeight() /*gScreenRect.bottom - gScreenRect.top*/;
	err = memcpy_s((BYTE*)_pPixel, lockedRect.Pitch * height, (BYTE*)lockedRect.pBits, lockedRect.Pitch * height);
	if (err)
	{
		printf("Error executing memcpy_s.\n");
	}

	if (FAILED(g_pSurface->UnlockRect()))
	{
		printf("Unable to Unlock Front Buffer Surface\n");
		return E_FAIL;
	}
	//g_pSurface->ReleaseDC(hdc);
	return S_OK;
}
#if 0
HRESULT ScreenCapture9::Reset()
{
	D3DDISPLAYMODE	ddm;
	D3DPRESENT_PARAMETERS	d3dpp;

	if (g_pSurface)															//Release the Surface - we need to get the latest surface
	{
		g_pSurface->Release();
		g_pSurface = NULL;
	}

	if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm)))	//User might have changed the mode - Get it afresh
	{
		printf_s("Unable to Get Adapter Display Mode");
		return E_FAIL;
	}
	//if (FAILED(g_pD3D->GetAdapterDisplayMode(1, &ddm)))	//User might have changed the mode - Get it afresh
	//{
	//	printf_s("Unable to Get Adapter Display Mode");
	//	return E_FAIL;
	//}

	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	d3dpp.Windowed = true;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.BackBufferFormat = ddm.Format;
	d3dpp.BackBufferHeight = gScreenRect.bottom = ddm.Height;
	d3dpp.BackBufferWidth = gScreenRect.right = ddm.Width;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = ghWnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (FAILED(g_pd3dDevice->Reset(&d3dpp)))
	{
		printf_s("Unable to Reset Device");
		return E_FAIL;
	}

	if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(ddm.Width, ddm.Height, /*D3DFMT_R8G8B8*/ D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &g_pSurface, NULL)))
	{
		printf_s("Unable to Create Surface");
		return E_FAIL;
	}

	return S_OK;
}
#endif
void ScreenCapture9::Cleanup()
{
	if (g_pSurface)
	{
		g_pSurface->Release();
		g_pSurface = NULL;
	}
	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = NULL;
	}
	if (g_pD3D)
	{
		g_pD3D->Release();
		g_pD3D = NULL;
	}
	//if (_pPixel != NULL)
	//	delete[]_pPixel;
	if (_hMemDC != NULL)
		DeleteDC(_hMemDC);
	if (_hMemDCBitmap != NULL)
		DeleteObject(_hMemDCBitmap);
}



BYTE* ScreenCapture9::Grab2Hwnd()
{

	int width = GetWidth();
	int height = GetHeight();




	GrabImage(_pPixel);
	AddMouse(_hMemDC);
	//memcpy(_pPixel, pData, width*height*biBitCountchar / 8);
	//if (hWnd != NULL)
	//{
		//BitBlt(hDC, 0, 0, width, height, _hMemDC, 0, 0, SRCCOPY);
	//}

	return _pPixel;
	//pDestBuffer->Release();
}
