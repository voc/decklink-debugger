#include <assert.h>

#include <array>
#include <stdio.h>
#include <iostream>
#include <algorithm>

#include "CaptureDelegate.h"

CaptureDelegate::CaptureDelegate(IDeckLink* deckLink) :
	m_refCount(1),
	m_deckLink(deckLink),
	m_lastFrame(NULL),
	m_hasSignal(false),
	m_detectedMode(""),
	m_pixelFormat(0),
	m_activeConnection(0)
{
	m_deckLink->AddRef();

	m_deckLinkInput = queryInputInterface();
	m_decklinkConnections = queryInputConnections();
}

IDeckLinkInput* CaptureDelegate::queryInputInterface(void)
{
	HRESULT result;
	IDeckLinkInput* deckLinkInput = NULL;

	result = m_deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput);
	if (result != S_OK)
	{
		std::cerr << "Failed to get Input Interface" << std::endl;
		exit(1);
	}

	return deckLinkInput;
}

int64_t CaptureDelegate::queryInputConnections(void)
{
	HRESULT result;
	IDeckLinkAttributes* deckLinkAttributes = NULL;

	int64_t connections;

	result = m_deckLink->QueryInterface(IID_IDeckLinkAttributes, (void**)&deckLinkAttributes);
	if (result != S_OK)
	{
		std::cerr << "Could not obtain the IDeckLinkAttributes interface" << std::endl;
		exit(1);
	}

	result = deckLinkAttributes->GetInt(BMDDeckLinkVideoInputConnections, &connections);
	if (result != S_OK)
	{
		std::cerr << "Could not obtain the list of input connections" << std::endl;
		exit(1);
	}

	assert(deckLinkAttributes->Release() == 0);

	return connections;
}

void CaptureDelegate::Start(void)
{
	HRESULT result;

	IDeckLinkDisplayMode* displayMode = queryFirstDisplayMode();

	m_deckLinkInput->SetCallback(this);

	result = m_deckLinkInput->EnableVideoInput(displayMode->GetDisplayMode(), PIXEL_FORMAT, VIDEO_INPUT_FLAGS);
	if (result != S_OK)
	{
		std::cerr << "Failed to enable video input. Is another application using the card?" << std::endl;
		exit(1);
	}

	result = m_deckLinkInput->EnableAudioInput(AUDIO_SAMPLE_RATE, AUDIO_SAMPLE_DEPTH, AUDIO_CHANNELS);
	if (result != S_OK)
	{
		std::cerr << "Failed to enable audio-input" << std::endl;
		exit(1);
	}

	assert(displayMode->Release() == 0);

	SelectNextConnection();

	result = m_deckLinkInput->StartStreams();
	if (result != S_OK)
	{
		std::cerr << "Failed to enable video input. Is another application using the card?" << std::endl;
		exit(1);
	}
}

void CaptureDelegate::Stop(void)
{
	m_deckLinkInput->StopStreams();
	m_deckLinkInput->DisableAudioInput();
	m_deckLinkInput->DisableVideoInput();

	m_deckLinkInput->SetCallback(NULL);
}

IDeckLinkDisplayMode* CaptureDelegate::queryFirstDisplayMode(void)
{
	HRESULT result;

	IDeckLinkDisplayModeIterator*	displayModeIterator = NULL;
	IDeckLinkDisplayMode*			displayMode = NULL;


	result = m_deckLinkInput->GetDisplayModeIterator(&displayModeIterator);
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

	return displayMode;
}

void CaptureDelegate::SelectNextConnection(void)
{
	std::array<BMDVideoConnection, 3> relevantConnections = {
		bmdVideoConnectionSDI,
		bmdVideoConnectionHDMI,
		bmdVideoConnectionOpticalSDI,
	};

	auto currentConnectionIt = std::find(relevantConnections.begin(), relevantConnections.end(), m_activeConnection);

	while(true)
	{
		if(currentConnectionIt == relevantConnections.end())
		{
			currentConnectionIt =relevantConnections.begin();
		}
		else {
			currentConnectionIt++;

			if(currentConnectionIt == relevantConnections.end())
			{
				currentConnectionIt =relevantConnections.begin();
			}
		}

		BMDVideoConnection nextConnection = *currentConnectionIt;
		if(m_decklinkConnections & nextConnection)
		{
			m_activeConnection = nextConnection;
			break;
		}
	}

	HRESULT result;
	IDeckLinkConfiguration* deckLinkConfiguration = NULL;

	result = m_deckLink->QueryInterface(IID_IDeckLinkConfiguration, (void**)&deckLinkConfiguration);
	if (result != S_OK)
	{
		std::cerr << "Failed to Query Attributes-Interface" << std::endl;
		exit(1);
	}

	deckLinkConfiguration->SetInt(bmdDeckLinkConfigVideoInputConnection, m_activeConnection);
	assert(deckLinkConfiguration->Release() == 0);
}

BMDVideoConnection CaptureDelegate::querySelectedConnection(void)
{
	HRESULT result;
	IDeckLinkConfiguration* deckLinkConfiguration = NULL;
	int64_t activeConnection;

	result = m_deckLink->QueryInterface(IID_IDeckLinkConfiguration, (void**)&deckLinkConfiguration);
	if (result != S_OK)
	{
		std::cerr << "Failed to Query Attributes-Interface" << std::endl;
		exit(1);
	}

	deckLinkConfiguration->GetInt(bmdDeckLinkConfigVideoInputConnection, &activeConnection);
	assert(deckLinkConfiguration->Release() == 0);

	return activeConnection;
}

HRESULT CaptureDelegate::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, UNUSED IDeckLinkAudioInputPacket* audioFrame)
{
	if (videoFrame == NULL || videoFrame->GetFlags() & bmdFrameHasNoInputSource)
	{
		m_hasSignal = false;
	}
	else
	{
		m_hasSignal = true;

		if(m_lastFrame) {
			m_lastFrame->Release();
		}

		m_pixelFormat = videoFrame->GetPixelFormat();

		m_lastFrame = videoFrame;
		m_lastFrame->AddRef();

		// it sometimes happens, that the switch to another connection is ignored when, just at this time,
		// a signal arrives. Make sure to always report the correct selected interface.
		m_activeConnection = querySelectedConnection();
	}

	return S_OK;
}

HRESULT CaptureDelegate::VideoInputFormatChanged(UNUSED BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode *mode, BMDDetectedVideoInputFormatFlags formatFlags)
{
	HRESULT result;
	char*	displayModeName = NULL;

	bool isRgb = formatFlags & bmdDetectedVideoInputRGB444;

	mode->GetName((const char**)&displayModeName);
	m_detectedMode = displayModeName;
	m_detectedMode += " ";
	m_detectedMode += (isRgb ? "RGB" : "YUV");

	if (displayModeName)
		free(displayModeName);

	BMDPixelFormat pixelFormat = (isRgb ? bmdFormat10BitRGB : bmdFormat10BitYUV);

	m_deckLinkInput->StopStreams();
	result = m_deckLinkInput->EnableVideoInput(mode->GetDisplayMode(), pixelFormat, VIDEO_INPUT_FLAGS);
	if (result != S_OK)
	{
		std::cerr << "Unable to Switch to new Video-Format" << std::endl;
		exit(1);
	}

	m_deckLinkInput->StartStreams();

	return S_OK;
}

ULONG CaptureDelegate::AddRef(void)
{
	return __sync_add_and_fetch(&m_refCount, 1);
}

ULONG CaptureDelegate::Release(void)
{
	int32_t newRefValue = __sync_sub_and_fetch(&m_refCount, 1);
	if (newRefValue == 0)
	{
		m_deckLinkInput->Release();

		m_deckLink->Release();

		delete this;
		return 0;
	}
	return newRefValue;
}
