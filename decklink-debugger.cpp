#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include <vector>
#include <string>
#include <iostream>

#include "util.h"
#include "tostring.h"
#include "DeckLinkAPI.h"
#include "DeviceProber.h"
#include "HttpServer.h"
#include "TablePrinter.h"

static bool g_do_exit = false;

std::vector<IDeckLink*> collectDeckLinkDevices(void);
void freeDeckLinkDevices(std::vector<IDeckLink*> deckLinkDevices);

std::vector<DeviceProber*> createDeviceProbers(std::vector<IDeckLink*> deckLinkDevices);
void freeDeviceProbers(std::vector<DeviceProber*> deviceProbers);

void printStatusList(std::vector<DeviceProber*> deviceProbers, unsigned int iteration);
char* getDeviceName(IDeckLink* deckLink);

static void sigfunc(int signum);

int main (UNUSED int argc, UNUSED char** argv)
{
	std::vector<IDeckLink*> deckLinkDevices = collectDeckLinkDevices();

	if(deckLinkDevices.size() == 0)
	{
		std::cout << "No DeckLink devices found" << std::endl;
		exit(2);
	}

	std::vector<DeviceProber*> deviceProbers = createDeviceProbers(deckLinkDevices);

	HttpServer* httpServer = new HttpServer(deviceProbers);

	signal(SIGINT, sigfunc);
	signal(SIGTERM, sigfunc);
	signal(SIGHUP, sigfunc);

	unsigned int iteration = 0;
	while(!g_do_exit) {
		printStatusList(deviceProbers, iteration++);

		for(DeviceProber* deviceProber: deviceProbers) {
			if(!deviceProber->GetSignalDetected()) {
				deviceProber->SelectNextConnection();
			}
		}
		sleep(1);
	}

	std::cout << "Shutting down" << std::endl;
	freeDeviceProbers(deviceProbers);
	freeDeckLinkDevices(deckLinkDevices);
	assert(httpServer->Release() == 0);

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

std::vector<IDeckLink*> collectDeckLinkDevices(void)
{
	std::vector<IDeckLink*> deckLinkDevices;
	IDeckLinkIterator*    deckLinkIterator;

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
		deckLinkDevices.push_back(deckLink);
	}

	assert(deckLinkIterator->Release() == 0);

	return deckLinkDevices;
}

void freeDeckLinkDevices(std::vector<IDeckLink*> deckLinkDevices)
{
	for(IDeckLink* deckLink: deckLinkDevices)
	{
		assert(deckLink->Release() == 0);
	}
}

std::vector<DeviceProber*> createDeviceProbers(std::vector<IDeckLink*> deckLinkDevices)
{
	std::vector<DeviceProber*> deviceProbers;

	for(IDeckLink* deckLink: deckLinkDevices)
	{
		deviceProbers.push_back(new DeviceProber(deckLink));
	}

	return deviceProbers;
}

void freeDeviceProbers(std::vector<DeviceProber*> deviceProbers)
{
	for(DeviceProber* deviceProber : deviceProbers)
	{
		assert(deviceProber->Release() == 0);
	}
}

void printStatusList(std::vector<DeviceProber*> deviceProbers, unsigned int iteration)
{
	if(iteration > 0)
	{
		int nLines = deviceProbers.size() + 6;
		std::cout << "\033[" << nLines << "A";
	}

	bprinter::TablePrinter table(&std::cout);
	table.AddColumn("#", 15);
	table.AddColumn("Device Name", 31);
	table.AddColumn("Can Input & Detect", 20);
	table.AddColumn("Signal Detected", 17);
	table.AddColumn("Active Connection", 19);
	table.AddColumn("Detected Mode", 16);
	table.AddColumn("Pixel Format", 15);
	table.set_flush_left();
	table.PrintHeader();

	int deviceIndex = 0;
	for(DeviceProber* deviceProber : deviceProbers)
	{
		if(!deviceProber->GetSignalDetected())
		{
			table << bprinter::greyon();
		}

		std::string deviceName = deviceProber->GetDeviceName();
		if(deviceProber->IsPairedDevice())
		{
			deviceName = "\\-> " + deviceName;
		}

		table
			<< deviceIndex
			<< deviceName
			<< boolToString(deviceProber->CanAutodetect() && deviceProber->CanInput())
			<< boolToString(deviceProber->GetSignalDetected())
			<< videoConnectionToString(deviceProber->GetActiveConnection())
			<< deviceProber->GetDetectedMode()
			<< pixelFormatToString(deviceProber->GetPixelFormat())
			<< bprinter::greyoff();

		deviceIndex++;
	}
	table.PrintFooter();

	const char iterationSign[4] = { '|', '\\', '-', '/' };
	std::cout << std::endl << "     Scanning... " << iterationSign[iteration % 4] << std::endl;
}
