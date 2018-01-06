#ifndef __ImageEncoder__
#define __ImageEncoder__

#include "DeckLinkAPI.h"
#include "DeviceProber.h"
#include "util.h"

class ImageEncoder
{
public:
	ImageEncoder(DeviceProber* deviceProber);
	virtual ~ImageEncoder() {}

	virtual void UpdateImage();
	virtual std::string GetLastImage() { return m_lastImage; }

	virtual ULONG AddRef(void);
	virtual ULONG Release(void);

private:
	IDeckLinkVideoFrame* convertFrameIfReqired(IDeckLinkVideoFrame* frame);
	std::string encodeToPng(IDeckLinkVideoFrame* frame);

private:
	int32_t                   m_refCount;
	DeviceProber*             m_deviceProber;
	IDeckLinkVideoConversion* m_frameConverter;
	std::string               m_lastImage;
};

#endif
