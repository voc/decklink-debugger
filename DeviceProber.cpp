#include <stdio.h>
#include <assert.h>
#include <iostream>

#include "DeviceProber.h"

DeviceProber::DeviceProber(IDeckLink* deckLink) : m_refCount(1), m_deckLink(deckLink)
{
	m_deckLink->AddRef();

	m_canAutodetect = queryCanAutodetect(m_deckLink);

	if (m_canAutodetect)
	{
		m_captureDelegate = setupCaptureDelegate(m_deckLink);
	}

	// check if autodetect is supported, set m_canAutoDetect
	// if yes, start CaptureDelegate

}

CaptureDelegate* DeviceProber::setupCaptureDelegate(IDeckLink* deckLink)
{
	HRESULT result;
	bool formatDetectionSupported;

	CaptureDelegate* captureDelegate = new CaptureDelegate();
	IDeckLinkAttributes* deckLinkAttributes = NULL;

	// Check the card supports format detection
	result = deckLink->QueryInterface(IID_IDeckLinkAttributes, (void**)&deckLinkAttributes);
	if (result != S_OK)
	{
		std::cerr << "Failed to Query Attributes-Interface" << std::endl;
		exit(1);
	}

	result = deckLinkAttributes->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &formatDetectionSupported);
	if (result != S_OK)
	{
		std::cerr << "Failed to Query Audio-Detection-Flag" << std::endl;
		exit(1);
	}

	deckLinkAttributes->Release();

	return captureDelegate;
}

bool DeviceProber::queryCanAutodetect(IDeckLink* deckLink)
{
	HRESULT result;
	bool formatDetectionSupported;

	IDeckLinkAttributes* deckLinkAttributes = NULL;

	// Check the card supports format detection
	result = deckLink->QueryInterface(IID_IDeckLinkAttributes, (void**)&deckLinkAttributes);
	if (result != S_OK)
	{
		std::cerr << "Failed to Query Attributes-Interface" << std::endl;
		exit(1);
	}

	result = deckLinkAttributes->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &formatDetectionSupported);
	if (result != S_OK)
	{
		std::cerr << "Failed to Query Audio-Detection-Flag" << std::endl;
		exit(1);
	}

	deckLinkAttributes->Release();

	return formatDetectionSupported;
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

		if(m_captureDelegate != NULL) {
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
