/**
 * CaptureDelegate represents the Interface to the DeckLinkAPI
 */

#include <assert.h>

#include <array>
#include <stdio.h>
#include <iostream>
#include <algorithm>

#include "CaptureDelegate.h"
#include "SubDeviceUtil.h"

#define LLOG(x) LOG(x) << "CaptureDelegate: "

CaptureDelegate::CaptureDelegate(IDeckLink* deckLink, IDeckLinkInput *deckLinkInput) :
	m_refCount(1),

	m_deckLink(deckLink),
	m_deckLinkInput(deckLinkInput),

	m_lastFrame(nullptr),
	m_lastFrameReleaser(&m_lastFrame),

	m_deckLinkConfiguration(nullptr),
	m_deckLinkConfigurationReleaser(&m_deckLinkConfiguration),

	m_deckLinkAttributes(nullptr),
	m_deckLinkAttributesReleaser(&m_deckLinkAttributes),

	m_hasSignal(false),
	m_detectedMode(""),
	m_pixelFormat(0),
	m_activeConnection(0)
{
	setDuplexToHalfDuplexModeIfSupported();

	m_deckLinkConfiguration = queryConfigurationInterface();
	m_deckLinkAttributes = queryAttributesInterface();
	m_decklinkConnections = queryInputConnections();
}

IDeckLinkConfiguration* CaptureDelegate::queryConfigurationInterface()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkConfiguration* deckLinkConfiguration = nullptr;

	result = m_deckLink->QueryInterface(IID_IDeckLinkConfiguration, (void **)&deckLinkConfiguration);
	throwIfNotOk(result, "Could not obtain the IID_IDeckLinkConfiguration interface");

	return deckLinkConfiguration;
}

IDeckLinkAttributes* CaptureDelegate::queryAttributesInterface()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkAttributes* deckLinkAttributes = nullptr;

	result = m_deckLink->QueryInterface(IID_IDeckLinkAttributes, (void **)&deckLinkAttributes);
	throwIfNotOk(result, "Could not obtain the IID_IDeckLinkAttributes interface");

	return deckLinkAttributes;
}

int64_t CaptureDelegate::queryInputConnections()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	int64_t connections;

	result = m_deckLinkAttributes->GetInt(BMDDeckLinkVideoInputConnections, &connections);
	throwIfNotOk(result, "Could not obtain the list of input connections");

	return connections;
}

void CaptureDelegate::setDuplexToHalfDuplexModeIfSupported()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	if(SubDeviceUtil::SupportsDuplexMode(m_deckLink))
	{
		LLOG(DEBUG) << "This device supports Duplex-Mode, setting to Half-Duplex";
		setDuplexToHalfDuplexMode(m_deckLink);
	}
	else
	{
		LLOG(DEBUG) << "This device does not support Duplex-Mode, querying for a Parent-Device";
		IDeckLink *parentDevice = SubDeviceUtil::QueryParentDevice(m_deckLink);
		RefReleaser<IDeckLink> parentDeviceReleaser(&parentDevice);

		if(parentDevice != nullptr) {
			LLOG(DEBUG) << "Parent-Device found, setting Parent-Device to Half-Duplex";
			setDuplexToHalfDuplexMode(parentDevice);
		}
	}
}

void CaptureDelegate::setDuplexToHalfDuplexMode(IDeckLink *deckLink)
{
	LLOG(INFO) << __PRETTY_FUNCTION__;
	HRESULT result;

	IDeckLinkConfiguration *deckLinkConfiguration = nullptr;
	RefReleaser<IDeckLinkConfiguration> deckLinkConfigurationReleaser(&deckLinkConfiguration);

	result = deckLink->QueryInterface(IID_IDeckLinkConfiguration, (void **)&deckLinkConfiguration);
	throwIfNotOk(result, "Could not obtain the IID_IDeckLinkConfiguration interface");

	result = deckLinkConfiguration->SetInt(bmdDeckLinkConfigDuplexMode, bmdDuplexModeHalf);
	throwIfNotOk(result, "Setting duplex mode failed");
}

void CaptureDelegate::Start()
{
	LLOG(INFO) << __PRETTY_FUNCTION__;
	HRESULT result;

	IDeckLinkDisplayMode* displayMode = queryFirstDisplayMode();
	RefReleaser<IDeckLinkDisplayMode> displayModeReleaser(&displayMode);

	LLOG(DEBUG) << "Setting Callback to *this*";
	m_deckLinkInput->SetCallback(this);

	LLOG(DEBUG) << "Enabling Video-Input";
	result = m_deckLinkInput->EnableVideoInput(displayMode->GetDisplayMode(), PIXEL_FORMAT, VIDEO_INPUT_FLAGS);
	throwIfNotOk(result, "Failed to enable video input. Is another application using the card?");

	result = m_deckLinkInput->EnableAudioInput(AUDIO_SAMPLE_RATE, AUDIO_SAMPLE_DEPTH, AUDIO_CHANNELS);
	throwIfNotOk(result, "Failed to enable audio-input");

	SelectNextConnection();

	result = m_deckLinkInput->StartStreams();
	throwIfNotOk(result, "Failed to enable video input. Is another application using the card?");
}

void CaptureDelegate::Stop()
{
	m_deckLinkInput->StopStreams();
	m_deckLinkInput->DisableAudioInput();
	m_deckLinkInput->DisableVideoInput();

	m_deckLinkInput->SetCallback(nullptr);
}

IDeckLinkDisplayMode* CaptureDelegate::queryFirstDisplayMode()
{
	HRESULT result;

	IDeckLinkDisplayModeIterator*	displayModeIterator = nullptr;
	RefReleaser<IDeckLinkDisplayModeIterator> displayModeIteratorReleaser(&displayModeIterator);

	IDeckLinkDisplayMode*			displayMode = nullptr;
	RefReleaser<IDeckLinkDisplayMode> displayModeReleaser(&displayMode);


	result = m_deckLinkInput->GetDisplayModeIterator(&displayModeIterator);
	throwIfNotOk(result, "Failed to get Display-Mode Iterator");

	// just select first display-mode to start auto-detection from
	result = displayModeIterator->Next(&displayMode);
	throwIfNotOk(result, "Failed to get Display-Mode from Iterator");

	displayMode->AddRef();
	return displayMode;
}

void CaptureDelegate::SelectNextConnection()
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

	m_deckLinkConfiguration->SetInt(bmdDeckLinkConfigVideoInputConnection, m_activeConnection);
}

BMDVideoConnection CaptureDelegate::querySelectedConnection()
{
	int64_t activeConnection;

	m_deckLinkConfiguration->GetInt(bmdDeckLinkConfigVideoInputConnection, &activeConnection);

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

ULONG CaptureDelegate::AddRef()
{
	ULONG newRefValue = __sync_add_and_fetch(&m_refCount, 1);
	LLOG(DEBUG2) << __PRETTY_FUNCTION__ << " newRefValue = " << newRefValue;
	return newRefValue;
}

ULONG CaptureDelegate::Release()
{
	ULONG newRefValue = __sync_sub_and_fetch(&m_refCount, 1);
	LLOG(DEBUG2) << __PRETTY_FUNCTION__ << " newRefValue = " << newRefValue;
	if (newRefValue == 0)
	{
		delete this;
	}
	return newRefValue;
}
