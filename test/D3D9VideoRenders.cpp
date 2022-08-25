#include "D3D9VideoRenders.h"

//
// Created by Administrator on 2019/8/29.
//
int rgb24_to_bmp(char* rgb24_buffer, const char* bmppath, int width, int height)
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
    FILE* fp_rgb24 = nullptr, *fp_bmp = nullptr;

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

#ifdef WIN32
//#include "libyuv/convert.h"
//#include "rtc_base/logging.h"

D3D9VideoRenders::D3D9VideoRenders(): 
    m_pDirect3D9(nullptr),
    m_pDirect3DDevice(nullptr),
    m_pDirect3DSurfaceRender(nullptr),
    m_nWidth(0),
    m_nHeight(0),
    m_isInit(false) {
  
    InitializeCriticalSection(&m_critial);
}

D3D9VideoRenders::~D3D9VideoRenders() {
    Cleanup();
    DeleteCriticalSection(&m_critial);
}

int D3D9VideoRenders::Init(HWND hwnd, unsigned int nWidth, unsigned int nHeight, bool isYuv) {
    if (!::IsWindow(hwnd)){
        //LOG(LS_INFO) << "D3D9VideoRenders::Init hwnd is error! hwnd: " << hwnd;
        printf("D3D9VideoRenders::Init hwnd is error!");
        return -1;
    }
    m_hWnd = hwnd;
    HRESULT lRet;
    
    Cleanup();
    ////LOG(LS_INFO) << "D3D9VideoRenders::Init";
    printf("D3D9VideoRenders::Init");
    m_pDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (m_pDirect3D9 == NULL){
        //LOG(LS_ERROR) << "D3D9VideoRenders::Init Direct3DCreate9 error!";
        printf("D3D9VideoRenders::Init Direct3DCreate9 error!");

        return -1;
    }        

    
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    //LOG(LS_INFO) << "D3D9VideoRenders::Init GetClientRect";
    printf("D3D9VideoRenders::Init GetClientRect");
    GetClientRect(hwnd, &m_rtViewport);
	//LOG(LS_INFO) << "############ hwnd = "<<hwnd << " width =" << m_rtViewport.right - m_rtViewport.left << " height = " << m_rtViewport.bottom - m_rtViewport.top;
    printf("############ hwnd = %d,width =%d, height =%d", hwnd, m_rtViewport.right - m_rtViewport.left, m_rtViewport.bottom - m_rtViewport.top);
    //LOG(LS_INFO) << "D3D9VideoRenders::Init m_pDirect3D9";
	lRet = m_pDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                                      &d3dpp, &m_pDirect3DDevice);

    //LOG(LS_INFO) << "D3D9VideoRenders::Init m_pDirect3D9 " << hex << lRet;
    printf("D3D9VideoRenders::Init m_pDirect3D9");
    if (FAILED(lRet)){
        printf("D3D9VideoRenders::Init CreateDevice error!");
        return -1;
    }

    /* if (isYuv) {
         LOG(LS_INFO) << "D3D9VideoRenders::Init CreateOffscreenPlainSurface yuv";
         lRet = m_pDirect3DDevice->CreateOffscreenPlainSurface(nWidth, nHeight,
                                                               (D3DFORMAT) MAKEFOURCC('Y', 'V', '1', '2'),
                                                               D3DPOOL_DEFAULT, &m_pDirect3DSurfaceRender, NULL);
         if (FAILED(lRet)){
             LOG(LS_ERROR) << "D3D9VideoRenders::Init CreateOffscreenPlainSurface error!";
             return -1;
         }
     } else {
         LOG(LS_INFO) << "D3D9VideoRenders::Init CreateOffscreenPlainSurface";*/
        lRet = m_pDirect3DDevice->CreateOffscreenPlainSurface(nWidth, nHeight, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
                                                              &m_pDirect3DSurfaceRender, NULL);
        if (FAILED(lRet)){
            //LOG(LS_ERROR) << "D3D9VideoRenders::Init CreateOffscreenPlainSurface error!";
            return -1;
        }
    /*}*/

    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_bIsYuv = isYuv;
    m_isInit = true;

    return 0;
}

void D3D9VideoRenders::Cleanup() {
    if (!m_isInit){
        return;
    }
    EnterCriticalSection(&m_critial);
    ////LOG(LS_INFO) << "D3D9VideoRenders::Cleanup m_pDirect3DSurfaceRender";
    if (m_pDirect3DSurfaceRender){
        m_pDirect3DSurfaceRender->Release();
        m_pDirect3DSurfaceRender = nullptr;
    }
    ////LOG(LS_INFO) << "D3D9VideoRenders::Cleanup m_pDirect3DDevice";
    if (m_pDirect3DDevice){
        m_pDirect3DDevice->Release();
        m_pDirect3DDevice = nullptr;
    }
    //LOG(LS_INFO) << "D3D9VideoRenders::Cleanup m_pDirect3D9";
    if (m_pDirect3D9){
        m_pDirect3D9->Release();
        m_pDirect3D9 = nullptr;
    }
    //LOG(LS_INFO) << "D3D9VideoRenders::Cleanup m_pDirect3D9 finished";
    LeaveCriticalSection(&m_critial);
	////LOG(LS_INFO) << "############ hwnd = " << m_hWnd << " D3D9VideoRenders::Cleanup finished";
    m_isInit = false;
}

bool D3D9VideoRenders::CheckRenderSize(unsigned int nWidth, unsigned int nHeight){
    if (!m_isInit){
        return true;
    }
    return m_nWidth == nWidth && m_nHeight == nHeight;
}

bool D3D9VideoRenders::RenderYuv(const uint8_t *yPtr, int y_stride, const uint8_t *uPtr, int u_stride,
                                const uint8_t *vPtr, int v_stride) {
    if (!m_isInit){
        return false;
    }
    if (!m_pDirect3DSurfaceRender || !yPtr || !uPtr || !vPtr) {
        return false;
    }
    if (m_bIsYuv == false) {
        return false;
    }

	RECT rtViewport;
	if (::IsWindow(m_hWnd) && GetClientRect(m_hWnd, &rtViewport))
	{
		int currentWidth = rtViewport.right - rtViewport.left;
		int currentHeight = rtViewport.bottom - rtViewport.top;
		int preWidth = m_rtViewport.right - m_rtViewport.left;
		int preHeight = m_rtViewport.bottom - m_rtViewport.top;
		LONG changedWidth = abs(currentWidth - preWidth);
		LONG changedHeight = abs(currentHeight - preHeight);
        if (((changedWidth > preWidth / 4) || (changedHeight > preHeight / 4))){
            //LOG(LS_INFO) << "hwnd = " << m_hWnd << " width =" << currentWidth << " height = " << currentHeight;
            if (Init(m_hWnd, m_nWidth, m_nHeight, m_bIsYuv) < 0){
                //LOG(LS_ERROR) << "RenderYuv Init error";
                return false;
            }          
          
        }
	}
	else{
		//LOG(LS_WARNING) << "hwnd is not valid";
		return false;
	}

    HRESULT lRet;
    D3DLOCKED_RECT d3d_rect;
    lRet = m_pDirect3DSurfaceRender->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT);
    if (FAILED(lRet)){
        //LOG(LS_ERROR) << "RenderRgb LockRect error" << lRet;
        return false;
    }

    byte *pDest = (BYTE *) d3d_rect.pBits;
    int stride = d3d_rect.Pitch;

    unsigned long i = 0;
    //if (m_bIsYuv) {  //如果送入的数据是YUV420P，需要注意它与YV12的区别（U,V的位置刚好相反）
    //    int y = (m_nWidth + 1) / 2;
    //    libyuv::I420Copy(yPtr, y_stride,
    //                     vPtr, v_stride,
    //                     uPtr, u_stride,
    //                     pDest, stride,
    //                     pDest + stride * m_nHeight, y,
    //                     pDest + stride * m_nHeight + y * ((m_nHeight + 1) / 2), y,
    //                     m_nWidth, m_nHeight);

    //    //LOG(INFO)<<"width:"<<m_nWidth<<",height:"<<m_nHeight<<",stride:"<<stride<<",y_stride:"<<y_stride<<",u:"<<u_stride;
    //}

    lRet = m_pDirect3DSurfaceRender->UnlockRect();
    if (FAILED(lRet)){
        //LOG(LS_ERROR) << "RenderRgb UnlockRect error " << lRet;
        return false;
    }

    if (m_pDirect3DDevice == NULL){
        //LOG(LS_ERROR) << "RenderRgb m_pDirect3DDevice nullptr";
        return false;
    }

    m_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    m_pDirect3DDevice->BeginScene();
    IDirect3DSurface9 *pBackBuffer = NULL;

    m_pDirect3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    //GetClientRect(m_hWnd, &m_rtViewport);

    //计算出目标矩形

    float w1 = m_rtViewport.right - m_rtViewport.left;
    float h1 = m_rtViewport.bottom - m_rtViewport.top;
    float w2 = (float) m_nWidth;
    float h2 = (float) m_nHeight;
    //全部填充绘制
    int srcW = 0;
    int srcH = 0;
    float srcX = 0;
    float srcY = 0;

    if (w2 / h2 > w1 / h1){
        srcH = h2;
        srcY = 0;
        srcW = h2 * w1 / h1;
        srcX = (w2 - srcW) / 2;
    }
    else{
        srcW = w2;
        srcX = 0;
        srcH = w2 * h1 / w1;
        srcY = (h2 - srcH) / 2;
    }

    RECT srcRect = { srcX, srcY, srcX + srcW, srcY + srcH };
#if 0
    int dstW = 0;
    int dstH = 0;
    float x = 0;
    float y = 0;
    if (w1 >= (w2 * h1) / h2) {
        dstH = h1;
        dstW = (h1 * w2) / h2;
        y = 0;
        x = (w1 - dstW) / 2;
    } else {
        dstW = w1;
        dstH = (w1 * h2) / w2;
        //dstH=240;
        x = 0;
        y = (h1 - dstH) / 2;
    }

    RECT destRect = {x, y, x + dstW, y + dstH};
#endif
	m_pDirect3DDevice->StretchRect(m_pDirect3DSurfaceRender, &srcRect, pBackBuffer, nullptr, D3DTEXF_LINEAR);
    m_pDirect3DDevice->EndScene();
    m_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
    pBackBuffer->Release();

    return true;
}

bool D3D9VideoRenders::RenderRgb(char *buffer) {

    //if (!m_pDirect3DSurfaceRender || !buffer)
    //    return false;

	RECT rtViewport;
	if (::IsWindow(m_hWnd) && GetClientRect(m_hWnd, &rtViewport))
	{
		int currentWidth = rtViewport.right - rtViewport.left;
		int currentHeight = rtViewport.bottom - rtViewport.top;
		int preWidth = m_rtViewport.right - m_rtViewport.left;
		int preHeight = m_rtViewport.bottom - m_rtViewport.top;
		LONG changedWidth = abs(currentWidth - preWidth);
		LONG changedHeight = abs(currentHeight - preHeight);
		if (((changedWidth > preWidth / 4) || (changedHeight > preHeight / 4))){
			//LOG(LS_INFO) << " hwnd = " << m_hWnd << " width =" << currentWidth << " height = " << currentHeight;
            if (Init(m_hWnd, m_nWidth, m_nHeight, m_bIsYuv) < 0){
                //LOG(LS_ERROR) << "RenderRgb Init error";
                return false;
            }
		}
	}
	else{
		//LOG(LS_WARNING) << "hwnd is not valid";
		return false;
	}



    if (m_pDirect3DDevice == NULL)
        return false;

    //m_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 255, 0), 1.0f, 0);
    IDirect3DSurface9 *pBackBuffer = NULL;

    //计算出目标矩形

    float w1 = m_rtViewport.right - m_rtViewport.left;
    float h1 = m_rtViewport.bottom - m_rtViewport.top;
    float w2 = (float) m_nWidth;
    float h2 = (float) m_nHeight;

    //全部填充绘制
    int srcW = 0;
    int srcH = 0;
    float srcX = 0;
    float srcY = 0;

    if (w2 / h2 > w1 / h1){
        srcH = h2;
        srcY = 0;
        srcW = h2 * w1 / h1;
        srcX = (w2 - srcW) / 2;
    }
    else{
        srcW = w2;
        srcX = 0;
        srcH = w2 * h1 / w1;
        srcY = (h2 - srcH) / 2;
    }

    RECT srcRect = { srcX, srcY, srcX + srcW, srcY + srcH };
    ///RECT srcRect = { 0, 0, w2, h2 };
    //    RECT destRect = { 0, 0, w1, h1 };

#if 1
    //居中非填充绘制
    int dstW = 0;
    int dstH = 0;
    float x = 0;
    float y = 0;
    if (w1 >= (w2 * h1) / h2) {
        dstH = h1;
        dstW = (h1 * w2) / h2;
        y = 0;
        x = (w1 - dstW) / 2;
    } else {
        dstW = w1;
        dstH = (w1 * h2) / w2;
        //dstH=240;
        x = 0;
        y = (h1 - dstH) / 2;
    }

    RECT destRect = { 0, 0, m_rtViewport.right, m_rtViewport.bottom };
#endif
    //RECT destRect = {0,0,160,120};
    HRESULT lRet;
    lRet = m_pDirect3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    
    //lRet = m_pDirect3DDevice->StretchRect(pBackBuffer, &m_rtViewport, m_pDirect3DSurfaceRender, nullptr, D3DTEXF_LINEAR);
    


    //m_pDirect3DDevice->BeginScene();

   
    //buffer = d3d_rect.Pitch;
    IDirect3DSurface9* m_pSurface = nullptr;
    m_pDirect3DDevice->CreateOffscreenPlainSurface(d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, d3dpp.BackBufferFormat, D3DPOOL_SYSTEMMEM, &m_pSurface, NULL);
    ////D3DXSaveSurfaceToFile("E:\\test.bmp", D3DXIFF_BMP, m_pSurface, NULL, NULL);
    //m_pDirect3DDevice->GetRenderTargetData(pBackBuffer, m_pSurface);
    //lRet = m_pDirect3DDevice->GetFrontBufferData(0, m_pSurface);
    m_pDirect3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

    D3DLOCKED_RECT d3d_rect;
    lRet = pBackBuffer->LockRect(&d3d_rect, NULL, 0);
    if (FAILED(lRet)) {
        //LOG(LS_ERROR) << "RenderRgb LockRect error";
        return false;
    }
    byte* pDest = (BYTE*)d3d_rect.pBits;
    int stride = d3d_rect.Pitch;
    buffer = new char[stride * m_nHeight];
    byte* pSrc = (byte*)buffer;
    unsigned long i = 0;

    int pixel_w_size = stride;
    for (i = 0; i < m_nHeight; i++) {
        memcpy(pSrc, pDest , pixel_w_size);
        pDest += stride;
        pSrc += pixel_w_size;
    }


    lRet = pBackBuffer->UnlockRect();
    if (FAILED(lRet)) {
        //LOG(LS_ERROR) << "RenderRgb UnlockRect error";
        return false;
    }
    //m_pDirect3DDevice->EndScene();
    //m_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
    pBackBuffer->Release();
    static FILE* fp_p = fopen("E:/testpng/1.rgb","wb+");
    if (fp_p) {
        fwrite(buffer, 1, stride * m_nHeight, fp_p);
    }
    fclose(fp_p);
    delete[]buffer;
    return true;
}
#endif