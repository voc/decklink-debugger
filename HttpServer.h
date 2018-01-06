#ifndef __HttpServer__
#define __HttpServer__

#include <string>
#include <list>
#include <map>
#include <microhttpd.h>

#include "DeviceProber.h"
#include "tostring.h"

class HttpServer
{
public:
	HttpServer(std::list<DeviceProber*> deviceProbers);
	virtual ~HttpServer() {}

	virtual ULONG AddRef(void);
	virtual ULONG Release(void);

	int requestHandler(
		std::string method,
		std::string url,
		std::map<std::string, std::string>* responseHeaders,
		std::stringstream* responseBody
	);

	int indexRequestHandler(
		std::map<std::string, std::string>* responseHeaders,
		std::stringstream* responseBody
	);

private:
	static const int PORT = 8042;

private:
	int32_t                  m_refCount;
	std::list<DeviceProber*> m_deviceProbers;
	MHD_Daemon*              m_daemon;
};

#endif
