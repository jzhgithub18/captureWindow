//
// Created by Administrator on 2019/8/29.
//

#ifndef ZYBMEDIASDK_D3D9VIDEORENDERS_H
#define ZYBMEDIASDK_D3D9VIDEORENDERS_H
#ifdef WIN32
#include "ID3D9VideoRenders.h"
#include <stdio.h>
#include <tchar.h>
#include <d3d9.h>
#include <thread>
#include <mutex>
#include <atomic>
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"winmm.lib")
//#pragma comment(lib,"d3dx9.lib")
using namespace std;

class D3D9VideoRenders:public ID3D9VideoRender {
public:
    D3D9VideoRenders();
    virtual~D3D9VideoRenders();
    virtual int Width(){ return m_nWidth; }
    virtual int Height(){ return m_nHeight; }
    virtual HWND getHwnd()override { return m_hWnd; }
public:
    int Init(HWND hwnd, unsigned int nWidth, unsigned int nHeight, bool isYuv);
    void Cleanup();
    bool CheckRenderSize(unsigned int nWidth, unsigned int nHeight);
    bool RenderRgb(char *buffer);
    bool RenderYuv(const uint8_t*yPtr,int y_stride,const uint8_t*uPtr,int u_stride,const uint8_t*vPtr,int v_stride);
private:
    std::atomic<bool>       m_isInit;
    bool                    m_bIsYuv;
    int                     m_nWidth;
    int                     m_nHeight;
    HWND                    m_hWnd;
    RECT                    m_rtViewport;
    CRITICAL_SECTION        m_critial;
    IDirect3D9              *m_pDirect3D9;
    IDirect3DDevice9        *m_pDirect3DDevice;
    IDirect3DSurface9       *m_pDirect3DSurfaceRender;
    D3DPRESENT_PARAMETERS d3dpp;
};

#endif

#endif //ZYBMEDIASDK_D3D9VIDEORENDER_H
