#ifndef __DeviceProber__
#define __DeviceProber__

#include "DeckLinkAPI.h"
#include "util.h"

class DeviceProber
{
public:
	DeviceProber(IDeckLink* deckLink);
	~DeviceProber();
private:
	IDeckLink* m_deckLink;
};

#endif
