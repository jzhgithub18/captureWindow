//
// Created by Administrator on 2019/8/29.
//

#ifndef ZYBMEDIASDK_D3D9VIDEORENDERTEXTURE_H
#define ZYBMEDIASDK_D3D9VIDEORENDERTEXTURE_H
#ifdef WIN32
#include "ID3D9VideoRenders.h"
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <d3d9.h>
#include <thread>
#include <mutex>
#include <atomic>

using namespace std;

class D3D9VideoRenderTexture :public ID3D9VideoRender {
public:
    D3D9VideoRenderTexture();
    virtual~D3D9VideoRenderTexture();

    virtual int Init(HWND hwnd, unsigned int nWidth, unsigned int nHeight, bool isYuv) ;
    virtual void Cleanup() override;
    virtual bool CheckRenderSize(unsigned int nWidth, unsigned int nHeight) override;
    virtual bool RenderRgb(char *buffer) override;
    virtual bool RenderYuv(const uint8_t*yPtr, int y_stride, const uint8_t*uPtr, int u_stride, const uint8_t*vPtr, int v_stride) override;

    virtual int Width(){ return m_nWidth; }
    virtual int Height(){ return m_nHeight; }
private:
    bool changeTexture(int& x,int& y,int& w,int& h);

    std::atomic<bool>       m_isInit;
    bool                    m_bIsYuv;
    int                     m_nWidth;
    int                     m_nHeight;
    HWND                    m_hWnd;
    RECT                    m_rtViewport;
  
    // 定义d3d接口、设备
    LPDIRECT3D9 m_pDirect3D9 = NULL;
    LPDIRECT3DDEVICE9 m_pDirect3DDevice = NULL;
    // 定义定点缓冲
    LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer = NULL;
    //定义纹理
    LPDIRECT3DTEXTURE9 m_D3DTexture = NULL;
    int m_srcW = 0;
    int m_srcH = 0;
    int m_preSrcW = 0;
    int m_preSrcH = 0;
    int m_srcX = 0;
    int m_srcY = 0;

    uint32_t m_lastchangeTime = 0;
};

#endif

#endif //ZYBMEDIASDK_D3D9VIDEORENDER_H
