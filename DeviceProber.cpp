#include <stdio.h>

#include "DeviceProber.h"

DeviceProber::DeviceProber(IDeckLink* deckLink) : m_refCount(1), m_deckLink(deckLink)
{
	m_deckLink->AddRef();
}

ULONG DeviceProber::AddRef(void)
{
	return __sync_add_and_fetch(&m_refCount, 1);
}

ULONG DeviceProber::Release(void)
{
	int32_t newRefValue = __sync_sub_and_fetch(&m_refCount, 1);
	if (newRefValue == 0)
	{
		m_deckLink->Release();

		delete this;
		return 0;
	}
	return newRefValue;
}
