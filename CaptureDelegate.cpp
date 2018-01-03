#include <stdio.h>
#include <iostream>

#include "CaptureDelegate.h"

CaptureDelegate::CaptureDelegate(IDeckLink* deckLink) : m_refCount(1), m_deckLink(deckLink)
{
	m_deckLink->AddRef();

	m_deckLinkInput = queryInputInterface();
}

IDeckLinkInput* CaptureDelegate::queryInputInterface()
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

void CaptureDelegate::Start(void)
{
	HRESULT result;

	IDeckLinkDisplayMode* displayMode = selectFirstDisplayMode();
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

IDeckLinkDisplayMode* CaptureDelegate::selectFirstDisplayMode()
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

ProberState CaptureDelegate::GetState()
{
	return SEARCHING_FOR_SIGNAL;
}

std::string CaptureDelegate::GetDetectedMode()
{
	return "";
}

std::string CaptureDelegate::GetActivePort()
{
	return "";
}

HRESULT CaptureDelegate::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, UNUSED IDeckLinkAudioInputPacket* audioFrame)
{
	return S_OK;

	if (videoFrame->GetFlags() & bmdFrameHasNoInputSource)
	{
		std::cout << ", No input signal detected";
	}
	else
	{
		if(hasRightEyeFrame(videoFrame))
		{
			std::cout << ", 3D Extension: Right-Eye-Frame present";
		}

		std::cout << ", Size: " << videoFrame->GetWidth() << "x" << videoFrame->GetHeight();
		std::cout << ", PixelFormat: " << videoFrame->GetPixelFormat();
	}

	std::cout << std::endl;
	return S_OK;
}

bool CaptureDelegate::hasRightEyeFrame(IDeckLinkVideoInputFrame* videoFrame)
{
	HRESULT result;

	bool rightEyeFramePresent = false;

	IDeckLinkVideoFrame*				rightEyeFrame = NULL;
	IDeckLinkVideoFrame3DExtensions*	threeDExtensions = NULL;

	result = videoFrame->QueryInterface(IID_IDeckLinkVideoFrame3DExtensions, (void **) &threeDExtensions);

	if ((result == S_OK) && (threeDExtensions->GetFrameForRightEye(&rightEyeFrame) == S_OK))
	{
		rightEyeFramePresent = true;
	}

	if (rightEyeFrame)
	{
		rightEyeFrame->Release();
		rightEyeFrame = NULL;
	}

	if (threeDExtensions)
	{
		threeDExtensions->Release();
		threeDExtensions = NULL;
	}

	return rightEyeFramePresent;
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