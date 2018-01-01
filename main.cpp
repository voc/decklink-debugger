#include <stdio.h>

#include <forward_list>
#include <map>

#include "util.h"
#include "DeckLinkAPI.h"
#include "DeviceProber.h"

std::forward_list<IDeckLink*> collectDeckLinkDevices(void);
std::map<IDeckLink*, DeviceProber*> createDeviceProbers(std::forward_list<IDeckLink*> deckLinkDevices);
void printStatusList(std::map<IDeckLink*, DeviceProber*> deviceProbers);
char* getDeviceName(IDeckLink* deckLink);

int main (UNUSED int argc, UNUSED char** argv)
{
	std::forward_list<IDeckLink*> deckLinkDevices = collectDeckLinkDevices();
	std::map<IDeckLink*, DeviceProber*> deviceProbers = createDeviceProbers(deckLinkDevices);

	printStatusList(deviceProbers);

	return 0;
}

std::forward_list<IDeckLink*> collectDeckLinkDevices(void)
{
	std::forward_list<IDeckLink*>	deckLinkDevices;
	IDeckLinkIterator*				deckLinkIterator;

	deckLinkIterator = CreateDeckLinkIteratorInstance();
	if (deckLinkIterator == NULL)
	{
		fprintf(stderr, "A DeckLink iterator could not be created. The DeckLink drivers may not be installed.\n");
		exit(1);
	}

	IDeckLink* deckLink = NULL;
	while (deckLinkIterator->Next(&deckLink) == S_OK)
	{
		deckLinkDevices.push_front(deckLink);
	}

	deckLinkIterator->Release();

	return deckLinkDevices;
}

std::map<IDeckLink*, DeviceProber*> createDeviceProbers(std::forward_list<IDeckLink*> deckLinkDevices)
{
	std::map<IDeckLink*, DeviceProber*> deckLinkDelegatesMap;

	for(IDeckLink* deckLink: deckLinkDevices)
	{
		DeviceProber* delegate = new DeviceProber(deckLink);
		deckLinkDelegatesMap[deckLink] = delegate;
	}

	return deckLinkDelegatesMap;
}

void printStatusList(std::map<IDeckLink*, DeviceProber*> deviceProbers)
{
	int deviceIndex = 0;

	for(auto const &entry : deviceProbers)
	{
		IDeckLink* deckLink = entry.first;

		char* devideName = getDeviceName(deckLink);
		printf("%2u: %s\n", deviceIndex, devideName);

		deviceIndex++;
	}

	if (deviceIndex == 0) {
		fprintf(stderr, "No DeckLink devices found\n");
	}
}

char* getDeviceName(IDeckLink* deckLink) {
	HRESULT result;

	char* deviceNameString = NULL;

	result = deckLink->GetModelName((const char **) &deviceNameString);
	if (result != S_OK)
	{
		fprintf(stderr, "Failed to get the Name for the DeckLink Device");
		exit(1);
	}

	return deviceNameString;
}
