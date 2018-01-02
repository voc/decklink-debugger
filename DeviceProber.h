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
	virtual bool CanAutodetect() { return m_canAutodetect; }

private:
	IDeckLinkInput* queryInputInterface(IDeckLink* deckLink);
	bool queryCanAutodetect(IDeckLink* deckLink);
	CaptureDelegate* setupCaptureDelegate(IDeckLink* deckLink);

private:
	int32_t				m_refCount;
	IDeckLink*			m_deckLink;
	CaptureDelegate*	m_captureDelegate;

	bool				m_canAutodetect;
};

#endif
