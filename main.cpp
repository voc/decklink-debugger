#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include <list>
#include <string>
#include <iostream>

#include "util.h"
#include "assert.h"
#include "DeckLinkAPI.h"
#include "ProberState.h"
#include "DeviceProber.h"

static bool g_do_exit = false;

std::list<IDeckLink*> collectDeckLinkDevices(void);
void freeDeckLinkDevices(std::list<IDeckLink*> deckLinkDevices);

std::list<DeviceProber*> createDeviceProbers(std::list<IDeckLink*> deckLinkDevices);
void freeDeviceProbers(std::list<DeviceProber*> deviceProbers);

void printStatusList(std::list<DeviceProber*> deviceProbers);
char* getDeviceName(IDeckLink* deckLink);

static void sigfunc(int signum);

int main (UNUSED int argc, UNUSED char** argv)
{
	std::list<IDeckLink*> deckLinkDevices = collectDeckLinkDevices();
	std::list<DeviceProber*> deviceProbers = createDeviceProbers(deckLinkDevices);

	signal(SIGINT, sigfunc);
	signal(SIGTERM, sigfunc);
	signal(SIGHUP, sigfunc);

	while(!g_do_exit) {
		printStatusList(deviceProbers);
		sleep(1);
	}

	std::cout << "Shutting down" << std::endl;
	freeDeviceProbers(deviceProbers);
	freeDeckLinkDevices(deckLinkDevices);

	std::cout << "Bye." << std::endl;
	return 0;
}

static void sigfunc(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
	{
		g_do_exit = true;
	}
}

std::list<IDeckLink*> collectDeckLinkDevices(void)
{
	std::list<IDeckLink*>	deckLinkDevices;
	IDeckLinkIterator*				deckLinkIterator;

	deckLinkIterator = CreateDeckLinkIteratorInstance();
	if (deckLinkIterator == NULL)
	{
		std::cerr
			<< "A DeckLink iterator could not be created."
			<< "The DeckLink drivers may not be installed."
			<< std::endl;

		exit(1);
	}

	IDeckLink* deckLink = NULL;
	while (deckLinkIterator->Next(&deckLink) == S_OK)
	{
		deckLinkDevices.push_front(deckLink);
	}

	assert(deckLinkIterator->Release() == 0);

	return deckLinkDevices;
}

void freeDeckLinkDevices(std::list<IDeckLink*> deckLinkDevices)
{
	for(IDeckLink* deckLink: deckLinkDevices)
	{
		assert(deckLink->Release() == 0);
	}
}

std::list<DeviceProber*> createDeviceProbers(std::list<IDeckLink*> deckLinkDevices)
{
	std::list<DeviceProber*> deviceProbersList;

	for(IDeckLink* deckLink: deckLinkDevices)
	{
		deviceProbersList.push_front(new DeviceProber(deckLink));
	}

	return deviceProbersList;
}

void freeDeviceProbers(std::list<DeviceProber*> deviceProbers)
{
	for(DeviceProber* deviceProber : deviceProbers)
	{
		assert(deviceProber->Release() == 0);
	}
}

std::string proberStateToString(ProberState proberState)
{
	switch(proberState)
	{
		case SEARCHING_FOR_SIGNAL:
			return "SEARCHING_FOR_SIGNAL";

		case SIGNAL_DETECTED:
			return "SIGNAL_DETECTED";

		default:
			return "UNDEFINED";
	}
}

std::string videoConnectionToString(BMDVideoConnection videoConnection)
{
	switch(videoConnection)
	{
		case bmdVideoConnectionSDI:
			return "SDI";

		case bmdVideoConnectionHDMI:
			return "HDMI";

		case bmdVideoConnectionOpticalSDI:
			return "OpticalSDI";

		default:
			return "UNDEFINED";
	}
}

void printStatusList(std::list<DeviceProber*> deviceProbers)
{
	int deviceIndex = 0;

	for(DeviceProber* deviceProber : deviceProbers)
	{
		std::cout
			<< "#" << deviceIndex << ", " << deviceProber->GetDeviceName()
			<< ", CanAutodetect: "        << deviceProber->CanAutodetect()
			<< ", State: "                << proberStateToString(deviceProber->GetState())
			<< ", ActiveConnection: "     << videoConnectionToString(deviceProber->GetActiveConnection())
			<< ", DetectedMode: "         << deviceProber->GetDetectedMode()
			<< std::endl;

		deviceIndex++;
	}

	if (deviceIndex == 0) {
				std::cerr << "No DeckLink devices found" << std::endl;
	}
}
