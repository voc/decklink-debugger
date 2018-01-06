#ifndef __MutableVideoFrame__
#define __MutableVideoFrame__

#include "DeckLinkAPI.h"

class MutableVideoFrame : public IDeckLinkVideoFrame
{
public:
	MutableVideoFrame(long width, long height, BMDPixelFormat pixelFormat);

	virtual ULONG AddRef(void);
	virtual ULONG Release(void);

	virtual long GetWidth (void);
	virtual long GetHeight (void);
	virtual long GetRowBytes (void);
	virtual BMDPixelFormat GetPixelFormat (void);
	virtual BMDFrameFlags GetFlags (void);
	virtual HRESULT GetBytes (/* out */ void **buffer);

	HRESULT QueryInterface(REFIID iid, LPVOID *ppv);

	virtual HRESULT GetTimecode (/* in */ BMDTimecodeFormat format, /* out */ IDeckLinkTimecode **timecode);
	virtual HRESULT GetAncillaryData (/* out */ IDeckLinkVideoFrameAncillary **ancillary);

private:
	int GetBytesPerPixel(BMDPixelFormat pixelFormat);
	long m_width;
	long m_height;
	BMDPixelFormat m_pixelFormat;
	int32_t m_refCount;
	char* m_buf;
};

#endif
