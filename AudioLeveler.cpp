#include "AudioLeveler.h"

#include <climits>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>


AudioLeveler::AudioLeveler(DeviceProber* deviceProber) :
	m_refCount(1),
	m_deviceProber(deviceProber)
{
}

ULONG AudioLeveler::AddRef(void)
{
	return __sync_add_and_fetch(&m_refCount, 1);
}

ULONG AudioLeveler::Release(void)
{
	int32_t newRefValue = __sync_sub_and_fetch(&m_refCount, 1);
	if (newRefValue == 0)
	{
		delete this;
		return 0;
	}
	return newRefValue;
}

std::string AudioLeveler::GetCurrentLevel() {
	IDeckLinkAudioInputPacket* packet = m_deviceProber->GetLastAudio();
	if(packet == NULL) {
		return "----";
	}

	packet->AddRef();

	size_t numSamples = packet->GetSampleFrameCount();
	size_t packetSize = numSamples
	  	  	  	  * (CaptureDelegate::AUDIO_SAMPLE_DEPTH / 8)
				  * CaptureDelegate::AUDIO_CHANNELS;
	void * buffer = malloc(packetSize);
	if (!buffer) {
		// out of memory or something
		packet->Release();
		return "NOMEM";
	}

	void * buffer2 = buffer;
	packet->GetBytes(&buffer2);
	packet->Release();

	if (!buffer2) {
		// buffer to small
		free(buffer);
		return "ERROR";
	}

	std::string ret = "UNSUPPORTED SAMPLE SIZE";
	if (CaptureDelegate::AUDIO_SAMPLE_DEPTH == 16) {
		ret = GetPeakLevel16Bit(numSamples, buffer);
	}

	free(buffer);
	return ret;
}

std::string AudioLeveler::GetPeakLevel16Bit(size_t numSamples, void * buffer) {
	signed short *maxValue = new signed short[CaptureDelegate::AUDIO_CHANNELS];
	signed short *sampleBuffer = (signed short *)buffer;

	signed short accu = 0;
	for (size_t sample = 0; sample < numSamples; sample++) {
		for (int channel = 0; channel < CaptureDelegate::AUDIO_CHANNELS; channel++) {
			accu = sampleBuffer[(sample * CaptureDelegate::AUDIO_CHANNELS) + channel];
			if (abs(accu) > maxValue[channel]) {
				maxValue[channel] = abs(accu);
			}
		}
	}

	std::stringstream dBStream;

	for (int channel = 0; channel < CaptureDelegate::AUDIO_CHANNELS; channel++) {
		if (channel > 0) {
			dBStream << ", ";
		}
		if (maxValue[channel] == 0) {
			dBStream << "--";
		} else {
			float dB = std::log(((float) maxValue[channel]/SHRT_MAX)) * 10;
			dBStream << std::setprecision(3) << dB;
		}
	}

	return dBStream.str();
}

