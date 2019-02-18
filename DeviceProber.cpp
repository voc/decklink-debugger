/**
 * DeviceProber represents an Interface to the static and always-true
 * information about a declink-device.
 * If the device allows auto-input and auto-detection, a CaptureDelegate will
 * be started, which will aquire realtime-information about the current signal
 * present on this device.
 * All calls to the CaptureDelegate are proxied through the DeviceProber, so
 * the DeviceProber is the primary interface to the main application.
 */

#include <stdio.h>
#include <assert.h>
#include <iostream>

#include "DeviceProber.h"
#include "SubDeviceUtil.h"

#include "scope_guard.hpp"
#include "log.h"

#define LLOG(x) LOG(x) << "DeviceProber: "

DeviceProber::DeviceProber(IDeckLink* deckLink) :
	m_deckLink(deckLink),
	m_deckLinkReleaser(&m_deckLink),

	m_deckLinkInput(nullptr),
	m_deckLinkInputReleaser(&m_deckLinkInput),

	m_captureDelegate(nullptr),
	m_captureDelegateReleaser(&m_captureDelegate)
{
	m_deckLink->AddRef();
	LLOG(INFO) << __PRETTY_FUNCTION__ << " for \"" << GetDeviceName() << "\"";

	m_canInput = queryCanInput();
	m_canAutodetect = queryCanAutodetect();
	m_isSubDevice = SubDeviceUtil::IsSubDevice(m_deckLink);

	LLOG(DEBUG) << "canInput = " << m_canInput
		<< " && canAutodetect = " << m_canAutodetect
		<< "; isSubDevice = " << m_isSubDevice;

	if (m_canAutodetect && m_canInput)
	{
		LLOG(DEBUG) << "querying IID_IDeckLinkInput interface";
		HRESULT result = m_deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&m_deckLinkInput);
		throwIfNotOk(result, "Failed to query IID_IDeckLinkInput Interface");

		LLOG(DEBUG) << "creating CaptureDelegate";
		m_captureDelegate = new CaptureDelegate(m_deckLink, m_deckLinkInput);

		LOG(INFO) << "starting CaptureDelegate";
		m_captureDelegate->Start();
	}
}

DeviceProber::~DeviceProber() {
	LLOG(INFO) << __PRETTY_FUNCTION__;

	if(m_captureDelegate) {
		LOG(INFO) << "stopping CaptureDelegate";
		m_captureDelegate->Stop();
	}
}

bool DeviceProber::queryCanInput()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkInput* deckLinkInput = nullptr;

	LLOG(DEBUG1) << "querying IDeckLinkInput Interface";
	result = m_deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput);
	RefReleaser<IDeckLinkInput> deckLinkInputReleaser(&deckLinkInput);

	return (result == S_OK);
}

bool DeviceProber::queryCanAutodetect()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkAttributes* deckLinkAttributes = nullptr;
	RefReleaser<IDeckLinkAttributes> deckLinkAttributesReleaser(&deckLinkAttributes);

	LLOG(DEBUG1) << "querying IID_IDeckLinkAttributes Interface";
	result = m_deckLink->QueryInterface(IID_IDeckLinkAttributes, (void **)&deckLinkAttributes);
	throwIfNotOk(result, "Could not obtain the IDeckLinkAttributes interface");

	LLOG(DEBUG1) << "querying BMDDeckLinkSupportsInputFormatDetection flag";
	bool formatDetectionSupported;
	result = deckLinkAttributes->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &formatDetectionSupported);
	throwIfNotOk(result, "Could not query BMDDeckLinkSupportsInputFormatDetection flag");

	return formatDetectionSupported;
}

bool DeviceProber::GetSignalDetected() {
	if (m_captureDelegate)
	{
		return m_captureDelegate->GetSignalDetected();
	}

	return false;
}

std::string DeviceProber::GetDetectedMode()
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

BMDPixelFormat DeviceProber::GetPixelFormat()
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

IDeckLinkVideoInputFrame* DeviceProber::GetLastFrame()
{
	if (m_captureDelegate)
	{
		return m_captureDelegate->GetLastFrame();
	}

	return NULL;
}

BMDVideoConnection DeviceProber::GetActiveConnection()
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

	char* deviceNameString = nullptr;

	result = m_deckLink->GetDisplayName((const char **) &deviceNameString);
	throwIfNotOk(result, "Failed to get the Name for the DeckLink Device");

	std::string str = std::string(deviceNameString);
	free(deviceNameString);
	return str;
}
