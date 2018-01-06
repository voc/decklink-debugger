#include "MutableVideoFrame.h"

#include "util.h"

#include <string.h>

#include <cstddef>
#include <iostream>

#define CompareREFIID(iid1, iid2)	(memcmp(&iid1, &iid2, sizeof(REFIID)) == 0)

MutableVideoFrame::MutableVideoFrame(long width, long height, BMDPixelFormat pixelFormat) :
	m_width(width),
	m_height(height),
	m_pixelFormat(pixelFormat),
	m_refCount(1)
{
	long bytes = GetBytesPerPixel(m_pixelFormat) * m_width * m_height;
	m_buf = new char[bytes];

	if(m_buf == NULL) {
		std::cerr << "Unable to allocate Mutable FrameBuffer" << std::endl;
		exit(1);
	}
}


int MutableVideoFrame::GetBytesPerPixel(BMDPixelFormat pixelFormat)
{
	int bytesPerPixel = 2;

	switch(pixelFormat)
	{
	case bmdFormat8BitYUV:
		bytesPerPixel = 2;
		break;
	case bmdFormat8BitARGB:
	case bmdFormat8BitBGRA:
	case bmdFormat10BitYUV:
	case bmdFormat10BitRGB:
		bytesPerPixel = 4;
		break;
	}

	return bytesPerPixel;
}


long MutableVideoFrame::GetWidth (void)
{
	return m_width;
}

long MutableVideoFrame::GetHeight (void)
{
	return m_height;
}

long MutableVideoFrame::GetRowBytes (void)
{
	return GetBytesPerPixel(m_pixelFormat) * m_width;
}

BMDPixelFormat MutableVideoFrame::GetPixelFormat (void)
{
	return m_pixelFormat;
}

BMDFrameFlags MutableVideoFrame::GetFlags (void)
{
	return bmdFrameFlagDefault;
}

HRESULT MutableVideoFrame::GetBytes (/* out */ void **buffer)
{
	*buffer = m_buf;
	return S_OK;
}


HRESULT MutableVideoFrame::QueryInterface(REFIID iid, LPVOID *ppv)
{
	CFUUIDBytes iunknown = CFUUIDGetUUIDBytes(IUnknownUUID);

	if (CompareREFIID(iid, iunknown))
	{
		*ppv = static_cast<IDeckLinkVideoFrame*>(this);
	}
	else if (CompareREFIID(iid, IID_IDeckLinkVideoFrame))
	{
		*ppv = static_cast<IDeckLinkVideoFrame*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


HRESULT MutableVideoFrame::GetTimecode (/* in */ UNUSED BMDTimecodeFormat format, /* out */ UNUSED IDeckLinkTimecode **timecode)
{
	return E_NOTIMPL;
}

HRESULT MutableVideoFrame::GetAncillaryData (/* out */ UNUSED IDeckLinkVideoFrameAncillary **ancillary)
{
	return E_NOTIMPL;
}



ULONG MutableVideoFrame::AddRef(void)
{
	return __sync_add_and_fetch(&m_refCount, 1);
}

ULONG MutableVideoFrame::Release(void)
{
	int32_t newRefValue = __sync_sub_and_fetch(&m_refCount, 1);
	if (newRefValue == 0)
	{
		delete[] m_buf;

		delete this;
		return 0;
	}
	return newRefValue;
}
