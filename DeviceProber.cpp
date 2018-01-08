#include <stdio.h>
#include <assert.h>
#include <iostream>

#include "DeviceProber.h"

DeviceProber::DeviceProber(IDeckLink* deckLink) : m_refCount(1), m_deckLink(deckLink)
{
	m_deckLink->AddRef();
	m_deckLinkAttributes = queryAttributesInterface();

	m_canInput = queryCanInput();
	m_canAutodetect = queryCanAutodetect();
	m_isPairedDevice = queryIsPairedDevice();

	if (m_canAutodetect && m_canInput)
	{
		m_captureDelegate = new CaptureDelegate(m_deckLink);
		m_captureDelegate->Start();
	}
}

IDeckLinkAttributes* DeviceProber::queryAttributesInterface(void)
{
	HRESULT result;
	IDeckLinkAttributes* deckLinkAttributes = NULL;

	result = m_deckLink->QueryInterface(IID_IDeckLinkAttributes, (void **)&deckLinkAttributes);
	if (result != S_OK) {
		std::cerr << "Could not obtain the IID_IDeckLinkAttributes interface" << std::endl;
		exit(1);
	}

	return deckLinkAttributes;
}

bool DeviceProber::queryCanInput(void)
{
	HRESULT result;
	IDeckLinkInput* deckLinkInput = NULL;
	bool canInput;

	result = m_deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput);
	canInput = (result == S_OK);

	deckLinkInput->Release();

	return canInput;
}

bool DeviceProber::queryIsPairedDevice(void)
{
	HRESULT result;
	int64_t paired_device_id;

	result = m_deckLinkAttributes->GetInt(BMDDeckLinkPairedDevicePersistentID, &paired_device_id);
	return (result == S_OK);
}

bool DeviceProber::queryCanAutodetect(void)
{
	HRESULT result;
	bool formatDetectionSupported;

	result = m_deckLinkAttributes->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &formatDetectionSupported);
	if (result != S_OK)
	{
		std::cerr << "Failed to Query Auto-Detection-Flag" << std::endl;
		exit(1);
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

ULONG DeviceProber::AddRef(void)
{
	return __sync_add_and_fetch(&m_refCount, 1);
}

ULONG DeviceProber::Release(void)
{
	int32_t newRefValue = __sync_sub_and_fetch(&m_refCount, 1);
	if (newRefValue == 0)
	{
		m_deckLink->Release();
		m_deckLinkAttributes->Release();

		if(m_captureDelegate != NULL) {
			m_captureDelegate->Stop();

			assert(m_captureDelegate->Release() == 0);
		}

		delete this;
		return 0;
	}
	return newRefValue;
}

std::string DeviceProber::GetDeviceName() {
	HRESULT result;

	char* deviceNameString = NULL;

	result = m_deckLink->GetModelName((const char **) &deviceNameString);
	if (result != S_OK)
	{
		fprintf(stderr, "Failed to get the Name for the DeckLink Device");
		exit(1);
	}

	return std::string(deviceNameString);
}
