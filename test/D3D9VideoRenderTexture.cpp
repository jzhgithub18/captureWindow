
#include "D3D9VideoRenderTexture.h"
#include "D3D9VideoRenders.h"
#ifdef WIN32
//#include "libyuv/convert.h"
//#include "libyuv/scale.h"
//#include "rtc_base/logging.h"
//#include "rtc_base/timeutils.h"
// 定义顶点格式标识符：x、y、z、深度、纹理
#define D3DFVF_VERTEX (D3DFVF_XYZRHW | D3DFVF_TEX1)

// 定义顶点结构
struct stD3DVertex
{
    float x, y, z, rhw, u, v;//x,y,z坐标；rhw深度；u,v坐标
};
ID3D9VideoRender* createD3DRender(RenderMode mode){
    if (mode == Texture){
        return new D3D9VideoRenderTexture();
    }
    return new D3D9VideoRenders();
}
D3D9VideoRenderTexture::D3D9VideoRenderTexture() :
m_pDirect3D9(nullptr),
m_pDirect3DDevice(nullptr),
m_nWidth(0),
m_nHeight(0),
m_isInit(false) {

    //LOG(LS_INFO) << "D3D9VideoRenderTexture::create";
}

D3D9VideoRenderTexture::~D3D9VideoRenderTexture() {
    Cleanup();
    //LOG(LS_INFO) << "D3D9VideoRenderTexture::destory";
}

int D3D9VideoRenderTexture::Init(HWND hwnd, unsigned int nWidth, unsigned int nHeight, bool isYuv) {
    if (!::IsWindow(hwnd)){
        //LOG(LS_INFO) << "D3D9VideoRenderTexture::Init hwnd is error! hwnd: " << hwnd;
        return -1;
    }
    m_isInit = false;
    m_hWnd = hwnd;
    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_bIsYuv = isYuv;
    HRESULT lRet;
    D3DDISPLAYMODE displayMode;
    Cleanup();
  

    //LOG(LS_INFO) << "D3D9VideoRenderTexture::Init";
    m_pDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (m_pDirect3D9 == NULL){
        //LOG(LS_ERROR) << "D3D9VideoRenderTexture::Init Direct3DCreate9 error!";
        return -1;
    }
    // Get the desktop display mode.
    if (m_pDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode)){
        //LOG(LS_ERROR) << "D3D9VideoRenderTexture::Init GetAdapterDisplayMode error!";
        Cleanup();
        return -1;
    }
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = displayMode.Format;

    ////LOG(LS_INFO) << "D3D9VideoRenderTexture::Init GetClientRect";
    GetClientRect(hwnd, &m_rtViewport);
    int win_width = m_rtViewport.right - m_rtViewport.left;
    int win_height = m_rtViewport.bottom - m_rtViewport.top;
    //LOG(LS_INFO) << "############ hwnd = " << hwnd << " width =" << win_width << " height = " << win_height;
    //LOG(LS_INFO) << "D3D9VideoRenderTexture::Init m_pDirect3D9";
    lRet = m_pDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
        &d3dpp, &m_pDirect3DDevice);
    
    //LOG(LS_INFO) << "D3D9VideoRenderTexture::Init m_pDirect3D9 " << hex << lRet;
    if (FAILED(lRet)){
        Cleanup();
        //LOG(LS_ERROR) << "D3D9VideoRenderTexture::Init CreateDevice error! m_pDirect3D9 " << m_pDirect3DDevice;
        return -1;
    }
    // 创建顶点缓冲
    stD3DVertex objData[] =
    {
        { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
        { win_width, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f },
        { 0.0f, win_height, 0.0f, 1.0f, 0.0f, 1.0f },
        { win_width, win_height, 0.0f, 1.0f, 1.0f, 1.0f }
    };

    lRet = m_pDirect3DDevice->CreateVertexBuffer(sizeof(objData), 0, D3DFVF_VERTEX, D3DPOOL_DEFAULT, &m_VertexBuffer, NULL);
    if (FAILED(lRet)){
        ////LOG(LS_ERROR) << "D3D9VideoRenderTexture::Init CreateVertexBuffer error!" << lRet;
        Cleanup();
        return -1;
    }

    // 填充顶点缓冲
    void *ptr;
    //LOG(LS_INFO) << "D3D9VideoRenderTexture::m_VertexBuffer finished!";

    lRet = m_VertexBuffer->Lock(0, sizeof(objData), (void**)&ptr, 0);
    if (FAILED(lRet)){
        //LOG(LS_ERROR) << "D3D9VideoRenderTexture::Init m_VertexBuffer Lock error!" << lRet;
        return -1;
    }

    memcpy(ptr, objData, sizeof(objData));

    lRet = m_VertexBuffer->Unlock();
    if (FAILED(lRet)){
        ////LOG(LS_ERROR) << "D3D9VideoRenderTexture::Init m_VertexBuffer Unlock error!" << lRet;
        return -1;
    }
    //创建纹理
    m_srcW = 0;
    m_srcH = 0;
    m_srcX = 0;
    m_srcY = 0;
       
    if (!changeTexture(m_srcX, m_srcY, m_srcW, m_srcH)){
        //LOG(LS_ERROR) << "D3D9VideoRenderTexture::Init changeTexture error!";
        Cleanup();
        return -1;
    }
    //LOG(LS_INFO) << "D3D9VideoRenderTexture::Init finished!";
    m_isInit = true;
    return 0;
}

void D3D9VideoRenderTexture::Cleanup() {
    if (!m_isInit){
        return;
    }
  
    LOG(LS_INFO) << "D3D9VideoRenderTexture::Cleanup m_pDirect3DDevice";
    if (m_pDirect3DDevice){
        m_pDirect3DDevice->Release();
        m_pDirect3DDevice = nullptr;
    }
    LOG(LS_INFO) << "D3D9VideoRenderTexture::Cleanup m_pDirect3D9";
    if (m_pDirect3D9){
        m_pDirect3D9->Release();
        m_pDirect3D9 = nullptr;
    }

	LOG(LS_INFO) << "D3D9VideoRenderTexture::Cleanup m_D3DTexture";
    if (m_D3DTexture){
        m_D3DTexture->Release();
        m_D3DTexture = nullptr;
        LOG(LS_INFO) << "D3D9VideoRenderTexture::Cleanup m_D3DTexture->Release";
    }
	LOG(LS_INFO) << "D3D9VideoRenderTexture::Cleanup m_VertexBuffer";
    if (m_VertexBuffer){
        m_VertexBuffer->Release();
        m_VertexBuffer = nullptr;
    }
    
    LOG(LS_INFO) << "D3D9VideoRenderTexture::Cleanup m_pDirect3D9 finished";
    LOG(LS_INFO) << "############ hwnd = " << m_hWnd << " D3D9VideoRenderTexture::Cleanup finished";
    m_isInit = false;
    m_srcW = 0;
    m_srcH = 0;
    m_srcX = 0;
    m_srcY = 0;
}

bool D3D9VideoRenderTexture::CheckRenderSize(unsigned int nWidth, unsigned int nHeight){
    if (!m_isInit){
        return true;
    }
    return m_nWidth == nWidth && m_nHeight == nHeight;
}

bool D3D9VideoRenderTexture::RenderYuv(const uint8_t *yPtr, int y_stride, const uint8_t *uPtr, int u_stride,
    const uint8_t *vPtr, int v_stride) {
    if (!yPtr || !uPtr || !vPtr || !m_isInit) {
        LOG(LS_ERROR) << "RenderYuv data null";
        return false;
    }
  
    //全部填充绘制
    if (!m_D3DTexture){
        LOG(LS_ERROR) << "RenderYuv m_D3DTexture->Release";
        changeTexture(m_srcX, m_srcY, m_srcW, m_srcH);
        return false;
    }
    if (::IsWindow(m_hWnd) == false)
    {
        LOG(LS_WARNING) << "hwnd is not valid";
        return false;
    }
    RECT rtViewport;
    if (GetClientRect(m_hWnd, &rtViewport)){
        int currentWidth = rtViewport.right - rtViewport.left;
        int currentHeight = rtViewport.bottom - rtViewport.top;
        int preWidth = m_rtViewport.right - m_rtViewport.left;
        int preHeight = m_rtViewport.bottom - m_rtViewport.top;
        LONG changedWidth = abs(currentWidth - preWidth);
        LONG changedHeight = abs(currentHeight - preHeight);
        if (((changedWidth > preWidth / 4) || (changedHeight > preHeight / 4))){
            m_rtViewport = rtViewport;
            changeTexture(m_srcX, m_srcY, m_srcW, m_srcH);
        }
    }
    else{
        LOG(LS_WARNING) << "GetClientRect is not failed";
        return false;
    }

    

    //向纹理拷贝数据
    D3DLOCKED_RECT d3d_rect;
    m_D3DTexture->LockRect(0, &d3d_rect, 0, 0);

    byte *pDest = (BYTE *)d3d_rect.pBits;
    int stride = d3d_rect.Pitch;

    unsigned long i = 0;
    if (m_bIsYuv) {  //如果送入的数据是YUV420P，需要注意它与YV12的区别（U,V的位置刚好相反）
        int y = (m_srcW + 1) / 2;
        for (i = 0; i < m_srcH; ++i) {
            memcpy(pDest, yPtr+m_srcX, stride);
            memcpy(pDest + stride * m_srcH, vPtr + m_srcX/2, y);
            memcpy(pDest + stride * m_srcH + y * ((m_srcH + 1) / 2), uPtr + m_srcX / 2, y);
        }

        //LOG(INFO)<<"width:"<<m_nWidth<<",height:"<<m_nHeight<<",stride:"<<stride<<",y_stride:"<<y_stride<<",u:"<<u_stride;
    }

    m_D3DTexture->UnlockRect(0);

    //渲染
    if (m_pDirect3DDevice == NULL)
        return false;


    m_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
    m_pDirect3DDevice->BeginScene();

    // 将顶点缓存设置为渲染流
    m_pDirect3DDevice->SetStreamSource(0, m_VertexBuffer, 0, sizeof(stD3DVertex));
    m_pDirect3DDevice->SetFVF(D3DFVF_VERTEX);//设置顶点渲染格式

    //设置纹理
    m_pDirect3DDevice->SetTexture(0, m_D3DTexture);
    m_pDirect3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);//绘制要渲染的图元
    /*
    D3DPT_POINTLIST 点
    D3DPT_LINELIST 线
    D3DPT_LINESTRIP 多条连续线段 1,2；2,3
    D3DPT_TRIANGLELIST 三角形
    D3DPT_TRIANGLESTRIP 多个三角形1,2,3；2，3,4 三角形带
    D3DPT_TRIANGLEFAN 1,2,3；1,3,4 三角形扇
    */
    m_pDirect3DDevice->SetTexture(0, NULL);
    m_pDirect3DDevice->EndScene();
    m_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
    //pBackBuffer->Release();

    return true;
}

bool D3D9VideoRenderTexture::RenderRgb(char *buffer) {

    if (!buffer || !m_isInit){
        LOG(LS_ERROR) << "RenderRgb m_isInit "<<m_isInit;
        return false;
    }

    //全部填充绘制
    if (!m_D3DTexture){
        LOG(LS_ERROR) << "RenderRgb m_D3DTexture->Release";
        changeTexture(m_srcX, m_srcY, m_srcW, m_srcH);
        return false;
    }
    if (::IsWindow(m_hWnd) == false)
    {
        LOG(LS_WARNING) << "hwnd is not valid";
        return false;
    }
    RECT rtViewport;
    if (GetClientRect(m_hWnd, &rtViewport)){
        int currentWidth = rtViewport.right - rtViewport.left;
        int currentHeight = rtViewport.bottom - rtViewport.top;
        int preWidth = m_rtViewport.right - m_rtViewport.left;
        int preHeight = m_rtViewport.bottom - m_rtViewport.top;
        LONG changedWidth = abs(currentWidth - preWidth);
        LONG changedHeight = abs(currentHeight - preHeight);
        if (((changedWidth > preWidth / 4) || (changedHeight > preHeight / 4)) ||
            ((changedHeight != 0 || changedWidth != 0) && m_lastchangeTime - rtc::Time32()>3000)){
            m_rtViewport = rtViewport;
            m_lastchangeTime = rtc::Time32();
            if (0 != Init(m_hWnd, m_nWidth, m_nHeight, false)){
                LOG(LS_ERROR) << "re init failed!";
                return false;
            }
        }
    }
    else{
        LOG(LS_WARNING) << "GetClientRect is not failed";
        return false;
    }
   
    //向纹理拷贝数据
    D3DLOCKED_RECT d3d_rect;
    auto ret = m_D3DTexture->LockRect(0, &d3d_rect, 0, 0);
    if (FAILED(ret)){
        LOG(LS_WARNING) << "m_D3DTexture LockRect failed";
        return false;
    }
    byte *pSrc = (byte *)buffer;
    byte * pDest = (BYTE *)d3d_rect.pBits;
    int stride = d3d_rect.Pitch;
    int pixel_w_size = m_nWidth * 4;
    if (pDest == nullptr){
        LOG(LS_WARNING) << "d3d_rect.pBits is null";
        return false;
    }
    
    pSrc += pixel_w_size * m_srcY;
    for (int i = m_srcY; i < m_nHeight - 2 * m_srcY; ++i){
        memcpy(pDest, pSrc + m_srcX * 4, m_srcW * 4);
        pDest += stride;
        pSrc += pixel_w_size;
    }
    ret = m_D3DTexture->UnlockRect(0);
    if (FAILED(ret)){
        LOG(LS_WARNING) << "m_D3DTexture LockRect failed";
        return false;
    }
    //渲染
    if (m_pDirect3DDevice == NULL)
        return false;

    
    ret = m_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    if (FAILED(ret)){
        LOG(LS_WARNING) << "m_pDirect3DDevice Clear failed";
        return false;
    }
    ret = m_pDirect3DDevice->BeginScene();
    if (FAILED(ret)){
        LOG(LS_WARNING) << "m_pDirect3DDevice BeginScene failed";
        return false;
    }
    // 将顶点缓存设置为渲染流
    m_pDirect3DDevice->SetStreamSource(0, m_VertexBuffer, 0, sizeof(stD3DVertex));
    m_pDirect3DDevice->SetFVF(D3DFVF_VERTEX);//设置顶点渲染格式
    
    //设置纹理
    m_pDirect3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    m_pDirect3DDevice->SetTexture(0, m_D3DTexture);
    m_pDirect3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);//绘制要渲染的图元
    /*
    D3DPT_POINTLIST 点
    D3DPT_LINELIST 线
    D3DPT_LINESTRIP 多条连续线段 1,2；2,3
    D3DPT_TRIANGLELIST 三角形
    D3DPT_TRIANGLESTRIP 多个三角形1,2,3；2，3,4 三角形带
    D3DPT_TRIANGLEFAN 1,2,3；1,3,4 三角形扇
    */
    m_pDirect3DDevice->SetTexture(0, NULL);
    ret = m_pDirect3DDevice->EndScene();
    if (FAILED(ret)){
        LOG(LS_WARNING) << "m_pDirect3DDevice EndScene failed";
        return false;
    }
    m_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);

    return true;
}
bool D3D9VideoRenderTexture::changeTexture(int& srcX, int& srcY, int& srcW, int& srcH){
    float w1 = m_rtViewport.right - m_rtViewport.left;
    float h1 = m_rtViewport.bottom - m_rtViewport.top;
    float w2 = (float)m_nWidth;
    float h2 = (float)m_nHeight;
   
    //计算出目标矩形   

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

    if (m_preSrcW != srcW || m_preSrcH != srcH){
        //创建纹理
        if (m_D3DTexture){
            LOG(LS_ERROR) << "D3D9VideoRenderTexture CreateTexture release";
            m_D3DTexture->Release();
            m_D3DTexture = nullptr;
        }
        if (!m_pDirect3DDevice){
            LOG(LS_ERROR) << "D3D9VideoRenderTexture m_pDirect3DDevice null ";
            return false;
        }
        LOG(LS_ERROR) << "D3D9VideoRenderTexture::CreateTexture";
        if (m_bIsYuv){
            auto ret = m_pDirect3DDevice->CreateTexture(srcW, srcH, 0, D3DUSAGE_AUTOGENMIPMAP,
                (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'), D3DPOOL_MANAGED, &m_D3DTexture, 0);
            if (ret < 0 || !m_D3DTexture){
                LOG(LS_ERROR) << "D3D9VideoRenderTexture::Init CreateTexture error! " << ret;
                return false;
            }
        }
        else{
            auto ret = m_pDirect3DDevice->CreateTexture(srcW, srcH, 0, D3DUSAGE_AUTOGENMIPMAP,
                D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &m_D3DTexture, 0);
            if (ret < 0 || !m_D3DTexture){
                LOG(LS_ERROR) << "D3D9VideoRenderTexture::Init CreateTexture error! " << ret;
                return false;
            }
        }
        
        m_preSrcW = srcW;
        m_preSrcH = srcH;
    }
    return true;
}
#endif