#include "ImageEncoder.h"

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <assert.h>
#include <png.h>

ImageEncoder::ImageEncoder(DeviceProber* deviceProber) :
	m_refCount(1),
	m_deviceProber(deviceProber)
{
	m_frameConverter = CreateVideoConversionInstance();
}

ULONG ImageEncoder::AddRef(void)
{
	return __sync_add_and_fetch(&m_refCount, 1);
}

ULONG ImageEncoder::Release(void)
{
	int32_t newRefValue = __sync_sub_and_fetch(&m_refCount, 1);
	if (newRefValue == 0)
	{
		m_frameConverter->Release(); // does not assert to 0

		delete this;
		return 0;
	}
	return newRefValue;
}

void ImageEncoder::updateImage() {
	//HRESULT result;
	IDeckLinkVideoInputFrame* frame = m_deviceProber->GetLastFrame();
	if(frame == NULL) {
		return;
	}

	frame->AddRef();

	void* frameBytes;
	frame->GetBytes(&frameBytes);

	int fp = open("/tmp/yuv10bit.data", O_WRONLY|O_CREAT|O_TRUNC, 0664);
	write(fp, frameBytes, frame->GetRowBytes() * frame->GetHeight());
	close(fp);
	std::cout << "written" << std::endl;

#if 0
	if(frame->GetPixelFormat() != bmdFormat8BitBGRA)
	{
		IDeckLinkMutableVideoFrame*	convertedFrame = NULL;

		result = m_deckLinkOutput->CreateVideoFrame(
			frame->GetWidth(),
			frame->GetHeight(),
			frame->GetWidth() * 8 * 4,
			bmdFormat8BitBGRA,
			bmdFrameFlagDefault,
			&convertedFrame);

		if (result != S_OK)
		{
			fprintf(stderr, "Failed to create video frame\n");
			exit(1);
		}

		result = m_frameConverter->ConvertFrame(frame, convertedFrame);
		if (result != S_OK)
		{
			fprintf(stderr, "Failed to convert frame\n");
			exit(1);
		}

		frame->Release();
		frame = convertedFrame;
	}
#endif

	frame->Release();
}
