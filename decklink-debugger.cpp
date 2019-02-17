#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include <atomic>
#include <vector>
#include <string>
#include <iostream>

#include "util.h"
#include "tostring.h"
#include "DeckLinkAPI.h"
#include "DeviceProber.h"
#include "HttpServer.h"
#include "TablePrinter.h"

#include "RefReleaser.hpp"
#include "scope_guard.hpp"
#include "log.h"

static std::atomic<bool> g_do_exit{false};

std::vector<DeviceProber*> createDeviceProbers();
void freeDeviceProbers(std::vector<DeviceProber*> deviceProbers);

void printStatusList(std::vector<DeviceProber*> deviceProbers, unsigned int iteration);
char* getDeviceName(IDeckLink* deckLink);

static void sigfunc(int signum);

void _main() {
	LOG(DEBUG) << "creating Device-Probers";
	std::vector<DeviceProber*> deviceProbers = createDeviceProbers();

	auto deviceProbersGuard = sg::make_scope_guard([deviceProbers]{
		LOG(DEBUG) << "freeing Device-Probers";
		freeDeviceProbers(deviceProbers);
	});

	if(deviceProbers.size() == 0)
	{
		throw "No DeckLink devices found";
	}


	LOG(INFO) << "=============================";

	LOG(INFO) << "starting Capturing";
	for(DeviceProber* deviceProber: deviceProbers) {
		deviceProber->Start();
		LOG(INFO) << "-----------------------------";
	}

	LOG(DEBUG) << "creating HttpServer";
	HttpServer* httpServer = new HttpServer(deviceProbers);
	auto httpServerGuard = sg::make_scope_guard([httpServer]{
		LOG(DEBUG) << "freeing HttpServer";
		assert(httpServer->Release() == 0);
	});

	LOG(DEBUG2) << "registering Signal-Handler";
	signal(SIGINT, sigfunc);
	signal(SIGTERM, sigfunc);
	signal(SIGHUP, sigfunc);

	LOG(DEBUG2) << "entering Display-Loop";
	unsigned int iteration = 0;
	while(!g_do_exit.load(std::memory_order_acquire))
	{
		printStatusList(deviceProbers, iteration++);

		for(DeviceProber* deviceProber: deviceProbers) {
			if(!deviceProber->GetSignalDetected()) {
				deviceProber->SelectNextConnection();
			}
		}
		sleep(1);
	}

	std::cout << "Bye." << std::endl;
}

int main (UNUSED int argc, UNUSED char** argv)
{
	try {
		_main();
	}
	catch(const char* e) {
		std::cerr << "exception cought: " << e << std::endl;
		return 1;
	}
	catch(...) {
		std::cerr << "unknown exception cought" << std::endl;
		return 1;
	}

	return 0;
}

static void sigfunc(int signum)
{
	LOG(INFO) << "cought signal "<< signum;
	if (signum == SIGINT || signum == SIGTERM)
	{
		LOG(DEBUG) << "g_do_exit = true";
		g_do_exit = true;
	}
}

std::vector<DeviceProber*> createDeviceProbers()
{
	IDeckLinkIterator*    deckLinkIterator = CreateDeckLinkIteratorInstance();
	RefReleaser<IDeckLinkIterator> deckLinkIteratorReleaser(&deckLinkIterator);
	throwIfNull(deckLinkIterator,
		"A DeckLink iterator could not be created. "
		"The DeckLink drivers may not be installed.");

	std::vector<DeviceProber*> deviceProbers;

	unsigned int i = 0;
	IDeckLink* deckLink = nullptr;
	try {
		while (deckLinkIterator->Next(&deckLink) == S_OK)
		{
			RefReleaser<IDeckLink> deckLinkReleaser(&deckLink);
			i++;
			LOG(DEBUG1) << "creating DeviceProber for Device " << i;
			deviceProbers.push_back(new DeviceProber(deckLink));
			LOG(INFO) << "-----------------------------";
		}
	}
	catch(...) {
		LOG(ERROR) << "cought exception, freeing already created DeviceProbers";
		LOG(INFO) << "-----------------------------";
		freeDeviceProbers(deviceProbers);
		throw;
	}

	return deviceProbers;
}

void freeDeviceProbers(std::vector<DeviceProber*> deviceProbers)
{
	unsigned int i = 0;
	for(DeviceProber* deviceProber : deviceProbers)
	{
		i++;
		LOG(DEBUG1) << "freeing DeviceProber for Device " << i;
		delete deviceProber;
		LOG(INFO) << "-----------------------------";
	}
	deviceProbers.clear();
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
		if(deviceProber->IsSubDevice())
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
