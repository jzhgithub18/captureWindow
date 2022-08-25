#include "stdafx.h"
#include "DGIWPlus.h"
#include <string.h>
#include <atlconv.h> 

//#include "FreeImage\FreeImage.h"

/**
@file DGIPlus win32 版本
@author 钱波
@fucntion to load/save images from/to memory based on GDI+ ，only in windows
注意：linux无法使用
*/

#define DGIDelete(x) do{if(x!=NULL){delete x;x=NULL;}}while(0)
#define DGIDelete_Array(x) do{if(x!=NULL){ delete[]x;x=NULL;}}while(0)

#define DEFINE_GOBJ    Graphics gObj(_Mem)
//#define DEFINE_GOBJHDC Graphics gObjhdc(_Mem)
#define DEFINE_IMG(x) Gdiplus::Bitmap *x;x=NULL

#define GET_WIDTH(x) x->GetWidth()
#define GET_HEIGHT(x) x->GetHeight()
#define UNDEFINE_IMG(x) DGIDelete(x)


static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0, size = 0;
	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
	{
		return -1;
	}
	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)malloc(size);
	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT i = 0; i < num; ++i)
	{
		if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[i].Clsid;
			free(pImageCodecInfo);
			return i;
		}
	}
	free(pImageCodecInfo);
	return -1;
}

static bool mem_to_global(const void* buf, size_t size, HGLOBAL global)
{
	void* dest = ::GlobalLock(global);
	if (dest == NULL)
	{
		return false;
	}
	std::memcpy(dest, buf, size);
	::GlobalUnlock(global);
	return true;
}


static bool stream_to_mem(IStream* stream, void** outbuf, size_t* size)
{
	ULARGE_INTEGER ulnSize;
	LARGE_INTEGER lnOffset;
	lnOffset.QuadPart = 0;
	/* get the stream size */
	if (stream->Seek(lnOffset, STREAM_SEEK_END, &ulnSize) != S_OK)
	{
		return false;
	}
	if (stream->Seek(lnOffset, STREAM_SEEK_SET, NULL) != S_OK)
	{
		return false;
	}

	/* read it */
	*outbuf = malloc((size_t)ulnSize.QuadPart);
	*size = (size_t)ulnSize.QuadPart;
	ULONG bytesRead;
	if (stream->Read(*outbuf, (ULONG)ulnSize.QuadPart, &bytesRead) != S_OK)
	{
		free(*outbuf);
		return false;
	}

	return true;
}

static Gdiplus::Bitmap* mi_from_memory(const void* buf, size_t size)
{
	IStream* stream = NULL;
	HGLOBAL global = ::GlobalAlloc(GMEM_MOVEABLE, size);
	if (global == NULL)
	{
		return NULL;
	}
	/* copy the buf content to the HGLOBAL */
	if (!mem_to_global(buf, size, global))
	{
		::GlobalFree(global);
		return NULL;
	}
	/* get the IStream from the global object */
	if (::CreateStreamOnHGlobal(global, TRUE, &stream) != S_OK)
	{
		::GlobalFree(global);
		return NULL;
	}
	/* create the image from the stream */
	Gdiplus::Bitmap* image = Gdiplus::Bitmap::FromStream(stream);

	stream->Release();
	/* i suppose when the reference count for stream is 0, it will
	GlobalFree automatically. The Image maintain the object also.*/
	return image;
}

static void* mi_to_memory(Gdiplus::Bitmap* image, void** outbuf, size_t* size)
{
	IStream* stream = NULL;
	if (::CreateStreamOnHGlobal(NULL, TRUE, &stream) != S_OK)
	{
		return NULL;
	}
	/* get the jpg encoder */
	::CLSID jpgClsid;
	GetEncoderClsid(L"image/jpeg", &jpgClsid);

	/* save the image to stream */
	Gdiplus::Status save_s = image->Save(stream, &jpgClsid);
	if (save_s != Gdiplus::Ok)
	{
		stream->Release();
		return NULL;
	}

	/* read the stream to buffer */
	if (!stream_to_mem(stream, outbuf, size))
	{
		stream->Release();
		return NULL;
	}
	return *outbuf;
}

static GdiplusStartupInput m_gdiplusStartupInput;
static ULONG_PTR m_gdiplusToken;
void StartDGIWPlus()
{
	GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);

}
void EndDGIPlus()
{
	GdiplusShutdown(m_gdiplusToken);
}

DGIWin32Plus::DGIWin32Plus(void)
{
	StartDGIWPlus();
}


DGIWin32Plus::~DGIWin32Plus(void)
{
	EndDGIPlus();
}


int   DGIWin32Plus::EncodeRGB2Jpeg(char* RGB, int width, int height, int lineWidth, char** Jpeg, int& JpegLen)
{
	Bitmap* bitmap = new Bitmap(width, height, lineWidth, PixelFormat32bppRGB, (BYTE*)RGB);
	int ret = 0;
	if (bitmap == NULL)
		return -1;
	size_t size;
	if (mi_to_memory(bitmap, (void**)Jpeg, &size) == NULL)
		ret = -1;
	JpegLen = (int)size;
	DGIDelete(bitmap);

	return ret;
}
