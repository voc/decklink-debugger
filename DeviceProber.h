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
	bool                 queryIsSubDevice();
	IDeckLinkAttributes* queryAttributesInterface();

	IDeckLink*           findDeckLinkInterfaceByPersistentId(int64_t pairedDeviceId);

private:
	IDeckLink*           m_deckLink;
	RefReleaser<IDeckLink> m_deckLinkReleaser;

	std::unique_ptr<CaptureDelegate> m_captureDelegate;

	bool                 m_canAutodetect;
	bool                 m_canInput;
	bool                 m_isSubDevice;
};

#endif
