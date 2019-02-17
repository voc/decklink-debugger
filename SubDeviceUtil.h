#ifndef __SubDeviceUtil__
#define __SubDeviceUtil__

#include "DeckLinkAPI.h"

class SubDeviceUtil
{
public:
	static bool IsSubDevice(IDeckLink *deckLink);
	static bool SupportsDuplexMode(IDeckLink *deckLink);
	static IDeckLink *QueryParentDevice(IDeckLink *deckLink);

private:
	static IDeckLink *findDeckLinkInterfaceByPersistentId(int64_t pairedDeviceId);
};

#endif
