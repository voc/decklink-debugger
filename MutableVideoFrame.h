#ifndef __MutableVideoFrame__
#define __MutableVideoFrame__

#include "DeckLinkAPI.h"

class MutableVideoFrame : public IDeckLinkVideoFrame
{
public:
	MutableVideoFrame(long width, long height, BMDPixelFormat pixelFormat);

	virtual ULONG AddRef(void);
	virtual ULONG Release(void);

	virtual long GetWidth (void) = 0;
	virtual long GetHeight (void) = 0;
	virtual long GetRowBytes (void) = 0;
	virtual BMDPixelFormat GetPixelFormat (void) = 0;
	virtual BMDFrameFlags GetFlags (void) = 0;
	virtual HRESULT GetBytes (/* out */ void **buffer) = 0;

	virtual HRESULT GetTimecode (/* in */ BMDTimecodeFormat format, /* out */ IDeckLinkTimecode **timecode) = 0;
	virtual HRESULT GetAncillaryData (/* out */ IDeckLinkVideoFrameAncillary **ancillary) = 0;

private:
	int GetBytesPerPixel(BMDPixelFormat pixelFormat);
	long m_width;
	long m_height;
	BMDPixelFormat m_pixelFormat;
	int32_t m_refCount;
	char* m_buf;
};

#endif
