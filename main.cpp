#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include <list>
#include <string>
#include <iostream>

#include "util.h"
#include "tostring.h"
#include "DeckLinkAPI.h"
#include "DeviceProber.h"
#include "HttpServer.h"
#include "ImageEncoder.h"

static bool g_do_exit = false;

std::list<IDeckLink*> collectDeckLinkDevices(void);
void freeDeckLinkDevices(std::list<IDeckLink*> deckLinkDevices);

std::list<DeviceProber*> createDeviceProbers(std::list<IDeckLink*> deckLinkDevices);
void freeDeviceProbers(std::list<DeviceProber*> deviceProbers);

std::list<ImageEncoder*> createImageEncoders(std::list<DeviceProber*> deviceProbers);
void freeImageEncoders(std::list<ImageEncoder*> imageEncoders);

void printStatusList(std::list<DeviceProber*> deviceProbers);
char* getDeviceName(IDeckLink* deckLink);

static void sigfunc(int signum);

int main (UNUSED int argc, UNUSED char** argv)
{
	std::list<IDeckLink*> deckLinkDevices = collectDeckLinkDevices();
	std::list<DeviceProber*> deviceProbers = createDeviceProbers(deckLinkDevices);
	std::list<ImageEncoder*> imageEncoders = createImageEncoders(deviceProbers);

	HttpServer* httpServer = new HttpServer(deviceProbers);

	signal(SIGINT, sigfunc);
	signal(SIGTERM, sigfunc);
	signal(SIGHUP, sigfunc);

	while(!g_do_exit) {
		printStatusList(deviceProbers);

		for(ImageEncoder* imageEncoder: imageEncoders) {
			imageEncoder->updateImage();
		}

		for(DeviceProber* deviceProber: deviceProbers) {
			if(!deviceProber->GetSignalDetected()) {
				deviceProber->SelectNextConnection();
			}
		}
		sleep(1);
	}

	std::cout << "Shutting down" << std::endl;
	freeImageEncoders(imageEncoders);
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

std::list<IDeckLink*> collectDeckLinkDevices(void)
{
	std::list<IDeckLink*> deckLinkDevices;
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
	std::list<DeviceProber*> deviceProbers;

	for(IDeckLink* deckLink: deckLinkDevices)
	{
		deviceProbers.push_front(new DeviceProber(deckLink));
	}

	return deviceProbers;
}

void freeDeviceProbers(std::list<DeviceProber*> deviceProbers)
{
	for(DeviceProber* deviceProber : deviceProbers)
	{
		assert(deviceProber->Release() == 0);
	}
}

std::list<ImageEncoder*> createImageEncoders(std::list<DeviceProber*> deviceProbers)
{
	std::list<ImageEncoder*> imageEncoders;

	for(DeviceProber* deviceProber: deviceProbers)
	{
		imageEncoders.push_front(new ImageEncoder(deviceProber));
	}

	return imageEncoders;
}

void freeImageEncoders(std::list<ImageEncoder*> imageEncoders)
{
	for(ImageEncoder* imageEncoder : imageEncoders)
	{
		assert(imageEncoder->Release() == 0);
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
			<< ", GetSignalDetected: "    << deviceProber->GetSignalDetected()
			<< ", ActiveConnection: "     << videoConnectionToString(deviceProber->GetActiveConnection())
			<< ", DetectedMode: "         << deviceProber->GetDetectedMode()
			<< std::endl;

		deviceIndex++;
	}

	if (deviceIndex == 0) {
				std::cerr << "No DeckLink devices found" << std::endl;
	}
}
