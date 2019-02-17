#ifndef __DeviceProber__
#define __DeviceProber__

#include <string>

#include "DeckLinkAPI.h"
#include "CaptureDelegate.h"
#include "util.h"

#include "RefReleaser.hpp"
#include "RefDeleter.hpp"

class DeviceProber
{
public:
	DeviceProber(IDeckLink* deckLink);
	virtual ~DeviceProber() {}

	virtual std::string GetDeviceName();
	virtual bool        CanAutodetect()  { return m_canAutodetect; }
	virtual bool        CanInput()       { return m_canInput; }

	// proxy to CaptureDelegate
	virtual bool               GetSignalDetected(void);
	virtual bool               IsSubDevice();
	virtual std::string        GetDetectedMode(void);
	virtual BMDPixelFormat     GetPixelFormat(void);
	virtual BMDVideoConnection GetActiveConnection(void);

	virtual void               SelectNextConnection(void);

	virtual IDeckLinkVideoInputFrame* GetLastFrame(void);

private:
	bool                 queryCanAutodetect(void);
	bool                 queryCanInput(void);
	IDeckLinkAttributes* queryAttributesInterface(void);

private:
	IDeckLink*           m_deckLink;
	RefReleaser<IDeckLink> m_deckLinkReleaser;

	IDeckLinkAttributes* m_deckLinkAttributes;
	RefReleaser<IDeckLinkAttributes> m_deckLinkAttributesReleaser;

	CaptureDelegate*     m_captureDelegate;
	RefDeleter<CaptureDelegate> m_captureDelegateDeleter;

	bool                 m_canAutodetect;
	bool                 m_canInput;
};

#endif
