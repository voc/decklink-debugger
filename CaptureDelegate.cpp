#include <stdio.h>

#include "CaptureDelegate.h"

CaptureDelegate::CaptureDelegate() : m_refCount(1)
{
	printf("CaptureDelegate created\n");
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
		delete this;
		return 0;
	}
	return newRefValue;
}

HRESULT CaptureDelegate::VideoInputFrameArrived(UNUSED IDeckLinkVideoInputFrame* videoFrame, UNUSED IDeckLinkAudioInputPacket* audioFrame)
{
	printf("CaptureDelegate::VideoInputFrameArrived");
	return S_OK;
}

HRESULT CaptureDelegate::VideoInputFormatChanged(UNUSED BMDVideoInputFormatChangedEvents events, UNUSED IDeckLinkDisplayMode *mode, UNUSED BMDDetectedVideoInputFormatFlags formatFlags)
{
	printf("CaptureDelegate::VideoInputFormatChanged");
	return S_OK;
}
