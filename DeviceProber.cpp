/**
 * DeviceProber represents the Interface to the main Application
 */

#include <stdio.h>
#include <assert.h>
#include <iostream>

#include "DeviceProber.h"

#include "scope_guard.hpp"
#include "log.h"

#define LLOG(x) LOG(x) << "DeviceProber: "

DeviceProber::DeviceProber(IDeckLink* deckLink) : m_deckLink(deckLink), m_captureDelegate(nullptr)
{
	LLOG(DEBUG2) << "reffing IDeckLink Interface";
	m_deckLink->AddRef();
	auto deckLinkGuard = sg::make_scope_guard([this]{
		LOG(DEBUG) << "emergency freeing DeckLink Device";
		m_deckLink->Release();
	});

	m_deckLinkAttributes = queryAttributesInterface();
	auto deckLinkAttributesGuard = sg::make_scope_guard([this]{
		LOG(DEBUG) << "emergency freeing IDeckLinkAttributes Interface";
		m_deckLinkAttributes->Release();
	});

	m_canInput = queryCanInput();
	m_canAutodetect = queryCanAutodetect();
	LLOG(DEBUG) << "canInput = " << m_canInput << " && canAutodetect = " << m_canAutodetect;

	if (m_canAutodetect && m_canInput)
	{
		LLOG(DEBUG) << "creating CaptureDelegate";
		m_captureDelegate = new CaptureDelegate(m_deckLink);
		auto captureDelegateGuard = sg::make_scope_guard([this]{
			LOG(DEBUG) << "emergency freeing CaptureDelegate";
			m_captureDelegate->Release();
		});

		m_captureDelegate->Start();
		captureDelegateGuard.dismiss();
	}

	deckLinkGuard.dismiss();
	deckLinkAttributesGuard.dismiss();
}

DeviceProber::~DeviceProber()
{
	LLOG(DEBUG) << "releasing held references of DeviceProber";

	if(m_captureDelegate != NULL) {
		LLOG(DEBUG2) << "releasing CaptureDelegate";
		m_captureDelegate->Stop();

		assert(m_captureDelegate->Release() == 0);
	}

	if(m_deckLinkAttributes != NULL) {
		LLOG(DEBUG2) << "releasing IDeckLinkAttributes Interface";
		assert(m_deckLinkAttributes->Release() == 0);
	}

	LLOG(DEBUG2) << "releasing IDeckLink Interface";
	m_deckLink->Release();
}

IDeckLinkAttributes* DeviceProber::queryAttributesInterface(void)
{
	HRESULT result;
	IDeckLinkAttributes* deckLinkAttributes = NULL;

	LLOG(DEBUG1) << "querying IDeckLinkAttributes Interface";
	result = m_deckLink->QueryInterface(IID_IDeckLinkAttributes, (void **)&deckLinkAttributes);
	if (result != S_OK) {
		LLOG(ERROR) << "query failed";
		throw "Could not obtain the IDeckLinkAttributes interface";
	}

	return deckLinkAttributes;
}

bool DeviceProber::queryCanInput(void)
{
	HRESULT result;
	IDeckLinkInput* deckLinkInput = NULL;
	bool canInput;

	LLOG(DEBUG1) << "querying IDeckLinkInput Interface";
	result = m_deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput);
	canInput = (result == S_OK);

	LLOG(DEBUG2) << "releasing IDeckLinkInput Interface";
	deckLinkInput->Release();

	return canInput;
}

bool DeviceProber::queryCanAutodetect(void)
{
	HRESULT result;
	bool formatDetectionSupported;

	LLOG(DEBUG1) << "querying BMDDeckLinkSupportsInputFormatDetection attribute";
	result = m_deckLinkAttributes->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &formatDetectionSupported);
	if (result != S_OK)
	{
		return false;
	}

	return formatDetectionSupported;
}

bool DeviceProber::GetSignalDetected(void) {
	if (m_captureDelegate)
	{
		return m_captureDelegate->GetSignalDetected();
	}

	return false;
}

bool DeviceProber::IsSubDevice(void) {
	if (m_captureDelegate)
	{
		return m_captureDelegate->IsSubDevice();
	}

	return false;
}

std::string DeviceProber::GetDetectedMode(void)
{
	if (m_captureDelegate)
	{
		if(m_captureDelegate->GetSignalDetected())
		{
			return m_captureDelegate->GetDetectedMode();
		}
	}

	return "";
}

BMDPixelFormat DeviceProber::GetPixelFormat(void)
{
	if (m_captureDelegate)
	{
		if(m_captureDelegate->GetSignalDetected())
		{
			return m_captureDelegate->GetPixelFormat();
		}
	}

	return 0;
}

IDeckLinkVideoInputFrame* DeviceProber::GetLastFrame(void)
{
	if (m_captureDelegate)
	{
		return m_captureDelegate->GetLastFrame();
	}

	return NULL;
}

BMDVideoConnection DeviceProber::GetActiveConnection(void)
{
	if (m_captureDelegate)
	{
		return m_captureDelegate->GetActiveConnection();
	}

	return 0;
}

void DeviceProber::SelectNextConnection(void) {
	if (m_captureDelegate)
	{
		return m_captureDelegate->SelectNextConnection();
	}
}

std::string DeviceProber::GetDeviceName() {
	HRESULT result;

	char* deviceNameString = NULL;

	result = m_deckLink->GetDisplayName((const char **) &deviceNameString);
	if (result != S_OK)
	{
		throw "Failed to get the Name for the DeckLink Device";
	}

	return std::string(deviceNameString);
}
