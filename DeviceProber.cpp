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
		m_deckLinkInput = queryInputInterface(m_deckLink);
		m_captureDelegate = setupCaptureDelegate(m_deckLinkInput);
	}

	// check if autodetect is supported, set m_canAutoDetect
	// if yes, start CaptureDelegate

}
#
IDeckLinkInput* DeviceProber::queryInputInterface(IDeckLink* deckLink)
{
	HRESULT result;
	IDeckLinkInput* deckLinkInput = NULL;

	result = deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput);
	if (result != S_OK)
	{
		std::cerr << "Failed to get Input Interface" << std::endl;
		exit(1);
	}

	return deckLinkInput;
}

CaptureDelegate* DeviceProber::setupCaptureDelegate(IDeckLinkInput* deckLinkInput)
{
	HRESULT result;

	IDeckLinkDisplayModeIterator*	displayModeIterator = NULL;
	IDeckLinkDisplayMode*			displayMode = NULL;


	result = deckLinkInput->GetDisplayModeIterator(&displayModeIterator);
	if (result != S_OK)
	{
		std::cerr << "Failed to get Display-Mode Iterator" << std::endl;
		exit(1);
	}

	// just select first display-mode to start auto-detection from
	result = displayModeIterator->Next(&displayMode);
	if (result != S_OK)
	{
		std::cerr << "Failed to get Display-Mode from Iterator" << std::endl;
		exit(1);
	}

	assert(displayModeIterator->Release() == 0);

	CaptureDelegate* captureDelegate = new CaptureDelegate();
	deckLinkInput->SetCallback(captureDelegate);

	BMDVideoInputFlags videoInputFlags = bmdVideoInputEnableFormatDetection;
	BMDPixelFormat pixelFormat = bmdFormat8BitYUV;

	result = deckLinkInput->EnableVideoInput(displayMode->GetDisplayMode(), pixelFormat, videoInputFlags);
	if (result != S_OK)
	{
		std::cerr << "Failed to enable video input. Is another application using the card?" << std::endl;
		exit(1);
	}

	int audioSampleDepth = 16;
	int audioChannels = 16;

	result = deckLinkInput->EnableAudioInput(bmdAudioSampleRate48kHz, audioSampleDepth, audioChannels);
	if (result != S_OK)
	{
		std::cerr << "Failed to enable audio-input" << std::endl;
		exit(1);
	}

	assert(displayMode->Release() == 0);

	result = deckLinkInput->StartStreams();
	if (result != S_OK)
	{
		std::cerr << "Failed to enable video input. Is another application using the card?" << std::endl;
		exit(1);
	}

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
		std::cerr << "Failed to Query Auto-Detection-Flag" << std::endl;
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

		if(m_deckLinkInput != NULL) {
			m_deckLinkInput->StopStreams();
			m_deckLinkInput->DisableAudioInput();
			m_deckLinkInput->DisableVideoInput();
			assert(m_deckLinkInput->Release() == 0);
		}

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
