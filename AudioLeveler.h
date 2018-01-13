#ifndef __AudioLeveler__
#define __AudioLeveler__

#include "DeviceProber.h"

class AudioLeveler
{
public:
	AudioLeveler(DeviceProber* deviceProber);
	virtual ~AudioLeveler() {}

	virtual std::string GetCurrentLevel();

	virtual ULONG AddRef(void);
	virtual ULONG Release(void);

private:
	std::string GetPeakLevel16Bit(size_t numSamples, void * buffer);

	int32_t                   m_refCount;
	DeviceProber*             m_deviceProber;
};


#endif
