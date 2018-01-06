#ifndef __DeviceProber__
#define __DeviceProber__

#include <string>

#include "DeckLinkAPI.h"
#include "CaptureDelegate.h"
#include "util.h"

class DeviceProber
{
public:
	DeviceProber(IDeckLink* deckLink);
	virtual ~DeviceProber() {}

	virtual ULONG AddRef(void);
	virtual ULONG Release(void);

	virtual std::string GetDeviceName();
	virtual bool        CanAutodetect() { return m_canAutodetect; }

	virtual IDeckLink*         GetIDecklink(void) { return m_deckLink; }
	virtual bool               GetSignalDetected(void);
	virtual std::string        GetDetectedMode(void);
	virtual BMDPixelFormat     GetPixelFormat(void);
	virtual BMDVideoConnection GetActiveConnection(void);

	virtual void               SelectNextConnection(void);

	virtual IDeckLinkVideoInputFrame* GetLastFrame(void);

private:
	bool queryCanAutodetect(void);

private:
	int32_t             m_refCount;
	IDeckLink*          m_deckLink;
	CaptureDelegate*    m_captureDelegate;

	bool                m_canAutodetect;
};

#endif
