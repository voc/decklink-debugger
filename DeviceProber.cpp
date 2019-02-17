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

#include "scope_guard.hpp"
#include "log.h"

#define LLOG(x) LOG(x) << "DeviceProber: "

DeviceProber::DeviceProber(IDeckLink* deckLink) :
	m_deckLink(deckLink),
	m_deckLinkReleaser(&m_deckLink),

	m_captureDelegate(nullptr)
{
	m_canInput = queryCanInput();
	m_canAutodetect = queryCanAutodetect();
	m_isSubDevice = queryIsSubDevice();

	LLOG(DEBUG) << "canInput = " << m_canInput
		<< " && canAutodetect = " << m_canAutodetect
		<< "; isSubDevice = " << m_isSubDevice;

/*
	if (m_canAutodetect && m_canInput)
	{
		LLOG(DEBUG) << "creating CaptureDelegate";
		m_captureDelegate = new CaptureDelegate(m_deckLink);
		m_captureDelegate->Start();
	}
*/
}

IDeckLinkAttributes* DeviceProber::queryAttributesInterface()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

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

bool DeviceProber::queryCanInput()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkInput* deckLinkInput = NULL;

	LLOG(DEBUG1) << "querying IDeckLinkInput Interface";
	result = m_deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput);
	RefReleaser<IDeckLinkInput> deckLinkInputReleaser(&deckLinkInput);

	return (result == S_OK);
}

bool DeviceProber::queryCanAutodetect()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkAttributes* deckLinkAttributes = NULL;
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

/**
 * A SubDevice is a Decklink-Device that has a paired Device which supports Duplex-Mode
 */
bool DeviceProber::queryIsSubDevice()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkAttributes* deckLinkAttributes = NULL;
	RefReleaser<IDeckLinkAttributes> deckLinkAttributesReleaser(&deckLinkAttributes);

	LLOG(DEBUG1) << "querying IID_IDeckLinkAttributes Interface";
	result = m_deckLink->QueryInterface(IID_IDeckLinkAttributes, (void **)&deckLinkAttributes);
	throwIfNotOk(result, "Could not obtain the IDeckLinkAttributes interface");

	LLOG(DEBUG1) << "querying BMDDeckLinkPairedDevicePersistentID attribute";
	int64_t pairedDeviceId;
	result = deckLinkAttributes->GetInt(BMDDeckLinkPairedDevicePersistentID, &pairedDeviceId);
	if(result != S_OK)
	{
		LLOG(DEBUG1) << "failed to query BMDDeckLinkPairedDevicePersistentID attribute, this is no SubDevice";
		return false;
	}

	LLOG(DEBUG1) << "found paired device-id, looking device up";
	IDeckLink *pairedDevice = findDeckLinkInterfaceByPersistentId(pairedDeviceId);
	RefReleaser<IDeckLink> pairedDeviceReleaser(&pairedDevice);
	throwIfNull(pairedDevice, "did not find device for pairedDeviceId reported by Decklink-Device");

	IDeckLinkAttributes* pairedDeckLinkAttributes = NULL;
	RefReleaser<IDeckLinkAttributes> pairedDeckLinkAttributesReleaser(&pairedDeckLinkAttributes);

	LLOG(DEBUG1) << "querying IID_IDeckLinkAttributes Interface of pairedDevice";
	result = pairedDevice->QueryInterface(IID_IDeckLinkAttributes, (void **)&pairedDeckLinkAttributes);
	throwIfNotOk(result, "Could not obtain the IDeckLinkAttributes interface");

	LLOG(DEBUG1) << "querying BMDDeckLinkSupportsDuplexModeConfiguration flag";
	bool supportsDuplexModeConfiguration;
	result = pairedDeckLinkAttributes->GetFlag(BMDDeckLinkSupportsDuplexModeConfiguration, &supportsDuplexModeConfiguration);
	if(result != S_OK) {
		LLOG(DEBUG1) << "failed to query BMDDeckLinkSupportsDuplexModeConfiguration flag of pairedDevice, this is no SubDevice";
		return false;
	}

	return supportsDuplexModeConfiguration;
}

IDeckLink* DeviceProber::findDeckLinkInterfaceByPersistentId(int64_t pairedDeviceId)
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkIterator *deckLinkIterator = CreateDeckLinkIteratorInstance();
	RefReleaser<IDeckLinkIterator> iterReleaser(&deckLinkIterator);
	throwIfNull(deckLinkIterator,
		"A DeckLink iterator could not be created.");

	unsigned int i = 0;
	IDeckLink *deckLink = nullptr;
	while (deckLinkIterator->Next(&deckLink) == S_OK) {
		i++;
		LOG(DEBUG) << "probing Device " << i;
		RefReleaser<IDeckLink> deckLinkReleaser(&deckLink);

		IDeckLinkAttributes* deckLinkAttributes = NULL;
		RefReleaser<IDeckLinkAttributes> deckLinkAttributesReleaser(&deckLinkAttributes);

		LLOG(DEBUG1) << "querying IID_IDeckLinkAttributes Interface";
		result = deckLink->QueryInterface(IID_IDeckLinkAttributes, (void **)&deckLinkAttributes);
		throwIfNotOk(result, "Could not obtain the IDeckLinkAttributes interface");

		LLOG(DEBUG1) << "querying BMDDeckLinkPersistentID attribute";
		int64_t persistent_id;
		result = deckLinkAttributes->GetInt(BMDDeckLinkPersistentID, &persistent_id);
		if(result != S_OK) {
			LLOG(DEBUG1) << "could not query the BMDDeckLinkPersistentID attribute, continuing with next device";
			continue;
		}

		if(pairedDeviceId == persistent_id)
		{
			LLOG(DEBUG) << "found matching device";
			deckLink->AddRef();
			return deckLink;
		}
	}

	LLOG(DEBUG) << "found no matching device";
	return NULL;
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

	char* deviceNameString = NULL;

	result = m_deckLink->GetDisplayName((const char **) &deviceNameString);
	if (result != S_OK)
	{
		throw "Failed to get the Name for the DeckLink Device";
	}

	return std::string(deviceNameString);
}
