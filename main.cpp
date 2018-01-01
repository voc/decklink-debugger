#include "DeckLinkAPI.h"
#include <stdio.h>
#include <stdlib.h>

std::forward_list<IDeckLink*> collectDeckLinkDevices(void);
std::map<IDeckLink*, DeckLinkCaptureDelegate*> createCaptureDelegates(std::forward_list<IDeckLink*> deckLinkDevices);
void clear(void);
void printStatusList(std::forward_list<IDeckLink*> deckLinkDevices, std::map<IDeckLink*, DeckLinkCaptureDelegate*> captureDelegates);
void printIterationMeter(int iteration);

int main (int argc, char** argv)
{
	// TODO crrate iterator here and free at end
	std::forward_list<IDeckLink*> deckLinkDevices = collectDeckLinkDevices();
	DeckLinkCaptureDelegateList* captureDelegates = createCaptureDelegates(deckLinkIterator);

	int iteration = 0;
	while(true) {
		IDeckLinkIterator* deckLinkIterator = createDeckLinkIterator();

		clear();
		printStatusList(deckLinkDevices, captureDelegates);
		printIterationMeter(iteration++);
		sleep(5);
	}

	// TODO free map

	exit(0);
}


std::forward_list<IDeckLink*> collectDeckLinkDevices(void)
{
	std::forward_list<IDeckLink*> deckLinkDevices;
	IDeckLinkIterator*		deckLinkIterator;

	deckLinkIterator = CreateDeckLinkIteratorInstance();
	if (deckLinkIterator == NULL)
	{
		fprintf(stderr, "A DeckLink iterator could not be created. The DeckLink drivers may not be installed.\n");
		exit(1);
	}

	IDeckLink* deckLink = NULL;
	while (deckLinkIterator->Next(&deckLink) == S_OK)
	{
		deckLinkDevices.push_back(deckLink);
	}

	deckLinkIterator->Release(); // invalidates IDeckLink instances?

	return deckLinkDevices;
}


void printStatusList(std::forward_list<IDeckLink*> deckLinkDevices, DeckLinkCaptureDelegateList* captureDelegates)
{
	int deviceIndex = 0;
	HRESULT result;

	for(IDeckLink* deckLink : deckLinkDevices)
	{
		char* deviceNameString = NULL;
		DeckLinkCaptureDelegate* captureDelegate;

		captureDelegate = captureDelegates->ForDeckLink(deckLink);
		if (captureDelegate == NULL) {
			fprintf(stderr, "Failed to get captureDelegate for DeckLink Device - did this Device just appear?");
			exit(1);
		}

		result = deckLink->GetModelName((const char **) &deviceNameString);
		if (result != S_OK)
			fprintf(stderr, "Failed to get the Name for the DeckLink Device");
			exit(1);
		}

		if (captureDelegate->IsOnline()) {
			printf("%2u %s: Online @ %s", deviceIndex, deviceNameString, captureDelegate->GetDetectedMode());
		}
		else {
			printf("%2u %s: Offline", deviceIndex, deviceNameString);
		}
		deviceIndex++;
	}

	if (deviceIndex == 0) {
		fprintf(stderr, "No DeckLink devices found\n");
	}
}

std::map<IDeckLink*, DeckLinkCaptureDelegate*> createCaptureDelegates(std::forward_list<IDeckLink*> deckLinkDevices)
{
	std::map<IDeckLink*, DeckLinkCaptureDelegate*> deckLinkDelegatesMap;

	for(IDeckLink* deckLink: deckLinkDevices)
	{
		deckLinkDelegatesMap[deckLink] = 
	}
}
