#ifndef __DeviceProber__
#define __DeviceProber__

#include "DeckLinkAPI.h"
#include "util.h"
#include "assert.h"

class DeviceProber
{
public:
	DeviceProber(IDeckLink* deckLink);
	virtual ~DeviceProber() {}

	virtual ULONG AddRef(void);
	virtual ULONG Release(void);

private:
	int32_t				m_refCount;
	IDeckLink*			m_deckLink;
};

#endif
