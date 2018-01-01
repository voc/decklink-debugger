#include <stdio.h>

#include "DeviceProber.h"

DeviceProber::DeviceProber(IDeckLink* deckLink) : m_deckLink(deckLink)
{
}

DeviceProber::~DeviceProber() {
	printf("free m_deckLink");
	m_deckLink->Release();
	m_deckLink = NULL;
}
