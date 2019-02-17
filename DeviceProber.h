#ifndef __DeviceProber__
#define __DeviceProber__

#include <string>
#include <memory>

#include "DeckLinkAPI.h"
#include "CaptureDelegate.h"
#include "util.h"

#include "RefReleaser.hpp"

class DeviceProber
{
public:
	DeviceProber(IDeckLink* deckLink);
	virtual ~DeviceProber() {}

	virtual void Start();

	virtual std::string GetDeviceName();
	virtual bool        CanAutodetect()  { return m_canAutodetect; }
	virtual bool        CanInput()       { return m_canInput; }
	virtual bool        IsSubDevice()    { return m_isSubDevice; }

	// proxy to CaptureDelegate
	virtual bool               GetSignalDetected();
	virtual std::string        GetDetectedMode();
	virtual BMDPixelFormat     GetPixelFormat();
	virtual BMDVideoConnection GetActiveConnection();

	virtual void               SelectNextConnection();

	virtual IDeckLinkVideoInputFrame* GetLastFrame();

private:
	bool                 queryCanAutodetect();
	bool                 queryCanInput();

private:
	IDeckLink*             m_deckLink;
	RefReleaser<IDeckLink> m_deckLinkReleaser;

	CaptureDelegate*       m_captureDelegate;
	RefReleaser<CaptureDelegate> m_captureDelegateReleaser;

	bool                 m_canAutodetect;
	bool                 m_canInput;
	bool                 m_isSubDevice;
};

#endif
