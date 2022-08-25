#ifndef ZYBMEDIASDK_ID3D9VIDEORENDERS123_H
#define ZYBMEDIASDK_ID3D9VIDEORENDERS123_H
#ifdef WIN32
#include <windows.h>
#include <stdint.h>
enum RenderMode{
    Surface = 0,
    Texture = 1,
};

class ID3D9VideoRender {
public:
    virtual~ID3D9VideoRender(){};
  
    virtual int Init(HWND hwnd, unsigned int nWidth, unsigned int nHeight, bool isYuv) = 0;
    virtual void Cleanup() = 0;
    virtual bool CheckRenderSize(unsigned int nWidth, unsigned int nHeight) = 0;
    virtual bool RenderRgb(char *buffer) = 0;
    virtual bool RenderYuv(const uint8_t*yPtr, int y_stride, const uint8_t*uPtr, int u_stride, const uint8_t*vPtr, int v_stride) = 0;
    virtual int Width() = 0;
    virtual int Height() = 0;
    virtual HWND getHwnd() = 0;
};
ID3D9VideoRender* createD3DRender(RenderMode mode);
#endif

#endif //
