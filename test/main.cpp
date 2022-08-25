#include <Windows.h>
#include <WinUser.h>

#include "D3D9VideoRenders.h"
#include "sd.h"
int rgb24_to_bmp2(char* rgb24_buffer, const char* bmppath, int width, int height)
{
    //定义相应的bmp数据头结构体
    typedef struct
    {
        long imageSize;
        long blank;
        long startPosition;
    }BmpHead;

    typedef struct
    {
        long  Length;
        long  width;
        long  height;
        unsigned short  colorPlane;
        unsigned short  bitColor;
        long  zipFormat;
        long  realSize;
        long  xPels;
        long  yPels;
        long  colorUse;
        long  colorImportant;
    }InfoHead;

    int i = 0, j = 0;
    BmpHead m_BMPHeader = { 0 };
    InfoHead  m_BMPInfoHeader = { 0 };
    char bfType[2] = { 'B','M' };
    int header_size = sizeof(bfType) + sizeof(BmpHead) + sizeof(InfoHead);
    //unsigned char* rgb24_buffer = NULL;
    FILE* fp_rgb24 = nullptr, * fp_bmp = nullptr;

    /*if ((fp_rgb24 = fopen(rgb24path, "rb")) == NULL) {
        printf("Error: Cannot open input RGB24 file.\n");
        return -1;
    }*/
    if ((fp_bmp = fopen(bmppath, "wb")) == nullptr) {
        printf("Error: Cannot open output BMP file.\n");
        return -1;
    }

    /*  rgb24_buffer = (unsigned char*)malloc(width * height * 3);
      fread(rgb24_buffer, 1, width * height * 3, fp_rgb24);*/

      //设置相应的bmp头数据结构
    m_BMPHeader.imageSize = 3 * width * height + header_size;
    m_BMPHeader.startPosition = header_size;

    m_BMPInfoHeader.Length = sizeof(InfoHead);
    m_BMPInfoHeader.width = width;
    //BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
    m_BMPInfoHeader.height = -height;
    m_BMPInfoHeader.colorPlane = 1;
    m_BMPInfoHeader.bitColor = 24;
    m_BMPInfoHeader.realSize = 3 * width * height;

    fwrite(bfType, 1, sizeof(bfType), fp_bmp);
    fwrite(&m_BMPHeader, 1, sizeof(m_BMPHeader), fp_bmp);
    fwrite(&m_BMPInfoHeader, 1, sizeof(m_BMPInfoHeader), fp_bmp);

    //BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
    //It saves pixel data in Little Endian
    //So we change 'R' and 'B'
   /* for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            char temp = rgb24_buffer[(j * width + i) * 3 + 2];
            rgb24_buffer[(j * width + i) * 3 + 2] = rgb24_buffer[(j * width + i) * 3 + 0];
            rgb24_buffer[(j * width + i) * 3 + 0] = temp;
        }
    }*/
    //写入相应的bmp数据值部分
    fwrite(rgb24_buffer, 3 * width * height, 1, fp_bmp);
    /* fclose(fp_rgb24);*/
    fclose(fp_bmp);
    //free(rgb24_buffer);
    printf("Finish generate %s!\n", bmppath);
    return 0;
}
void ScreenShot(char* filename) {
    IDirect3DSurface9* tmp = NULL; 
    IDirect3DSurface9* back = NULL; //生成固定颜色模式的离屏表面（Width和 Height为屏幕或窗口的宽高） 
    //D3D9Device->CreateOffscreenPlainSurface(Width, Height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &tmp, NULL); 
    //// 获得BackBuffer的D3D Surface
    //D3D9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back); 
    // Copy一下，，需要时转换颜色格式 
    //D3DXLoadSurfaceFromSurface(tmp, NULL, NULL, back, NULL, NULL, D3DX_FILTER_NONE, 0); // 保存成BMP格式 
    ////D3DXSaveSurfaceToFile(filename, D3DXIFF_BMP, tmp, NULL, NULL); // 释放Surface，防止内存泄漏 SAFE_RELEASE(tmp);
    //SAFE_RELEASE(back);
}
//自定义的窗口过程
LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    switch (Msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
    return 0;
}

int main()
{
    int nCmdShow = 10;
    HINSTANCE hInstance = ::GetModuleHandle(NULL);
    // 创建窗口类
    WNDCLASS wnd = {
        CS_HREDRAW,
        MyWindowProc,          // 使用自定义的窗口过程函数
        0,0,hInstance,
        LoadIcon(NULL,IDI_APPLICATION),
        LoadCursor(NULL,IDC_ARROW),
        (HBRUSH)(GetStockObject(WHITE_BRUSH)),
        NULL, L"MyWindow"
    };
    // 注册窗口类
    RegisterClass(&wnd);
     HWND hWnd2 = CreateWindow(L"MyWindow", L"newWindow",WS_MAXIMIZE ,
       0, 200, 1280, 720, NULL, NULL, hInstance, NULL);
   ShowWindow(hWnd2, nCmdShow);

    D3D9VideoRenders render;
    //render.Init((HWND)0x004B120E, 960, 720, false);
    char* buffer = nullptr;
    //render.RenderRgb(buffer);
    ScreenCapture9 sc;
    int coutn = sc.InitDisplayCount();
    sc.InitD3D((HWND)hWnd2, 0);
    auto ptr_rgb32 = sc.Grab2Hwnd();
    
    int n = 0;
    int pictch = sc.GetPitch();
    int sh = sc.GetHeight();
    int sw = sc.GetWidth();
    int widht = sw;
    int height = sh;
    char* ptr_rgb24 = new char[widht * height * 3];
    char* ptr = (char*)ptr_rgb32;
    int j = 0;
    for (int i = 0; i < height; ++i) {
        j = 0;
        for (j = 0; j < widht*4; j += 4 ) {
            ptr_rgb24[n++] = ptr[pictch * i + j];
            ptr_rgb24[n++] = ptr[pictch * i + j +1];
            ptr_rgb24[n++] = ptr[pictch * i + j + 2];
            
        }
    }
    rgb24_to_bmp2((char*)ptr_rgb24, "E:/a.bmp", widht, height);

    return 0;
}
