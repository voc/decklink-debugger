#include "assert.h"

#include <array>
#include <stdio.h>
#include <iostream>
#include <algorithm>

#include "CaptureDelegate.h"

CaptureDelegate::CaptureDelegate(IDeckLink* deckLink) :
	m_refCount(1),
	m_deckLink(deckLink),
	m_state(SEARCHING_FOR_SIGNAL),
	m_activeConnection(0)
{
	m_deckLink->AddRef();

	m_deckLinkInput = queryInputInterface();
	m_decklinkConnections = queryInputConnections();
	std::cout << "m_decklinkConnections" << m_decklinkConnections << std::endl;
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
	BMDVideoInputFlags videoInputFlags = bmdVideoInputEnableFormatDetection;

	m_deckLinkInput->SetCallback(this);

	result = m_deckLinkInput->EnableVideoInput(displayMode->GetDisplayMode(), PIXEL_FORMAT, videoInputFlags);
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

	selectNextConnection();

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

void CaptureDelegate::selectNextConnection(void)
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
		}

		BMDVideoConnection nextConnection = *currentConnectionIt;
		if(m_decklinkConnections & nextConnection)
		{
			m_activeConnection = nextConnection;
			break;
		}
	}
}

ProberState CaptureDelegate::GetState(void)
{
	return m_state;
}

std::string CaptureDelegate::GetDetectedMode(void)
{
	return m_detectedMode;
}

BMDVideoConnection CaptureDelegate::GetActiveConnection(void)
{
	return m_activeConnection;
}

HRESULT CaptureDelegate::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, UNUSED IDeckLinkAudioInputPacket* audioFrame)
{
	if (videoFrame->GetFlags() & bmdFrameHasNoInputSource)
	{
		m_state = SEARCHING_FOR_SIGNAL;
	}
	else
	{
		m_state = SIGNAL_DETECTED;

		std::cout << ", Size: " << videoFrame->GetWidth() << "x" << videoFrame->GetHeight();
		std::cout << ", PixelFormat: " << videoFrame->GetPixelFormat();
	}

	return S_OK;
}

HRESULT CaptureDelegate::VideoInputFormatChanged(UNUSED BMDVideoInputFormatChangedEvents events, UNUSED IDeckLinkDisplayMode *mode, BMDDetectedVideoInputFormatFlags formatFlags)
{
	char*	displayModeName = NULL;

	mode->GetName((const char**)&displayModeName);
	printf("Video format changed to %s %s\n", displayModeName, formatFlags & bmdDetectedVideoInputRGB444 ? "RGB" : "YUV");

	if (displayModeName)
		free(displayModeName);

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
		assert(m_deckLinkInput->Release() == 0);

		m_deckLink->Release();

		delete this;
		return 0;
	}
	return newRefValue;
}