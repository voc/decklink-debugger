#ifndef __HttpServer__
#define __HttpServer__

#include <string>
#include <vector>
#include <map>
#include <microhttpd.h>

#include "DeviceProber.h"
#include "ImageEncoder.h"
#include "tostring.h"

class HttpServer
{
public:
	HttpServer(std::vector<DeviceProber*> deviceProbers);
	virtual ~HttpServer() {}

	virtual ULONG AddRef(void);
	virtual ULONG Release(void);

	int requestHandler(
		std::string method,
		std::string url,
		std::map<std::string, std::string>* responseHeaders,
		std::stringstream* responseBody
	);

private:
	static const int PORT = 8042;

	int indexRequestHandler(
		std::map<std::string, std::string>* responseHeaders,
		std::stringstream* responseBody
	);

	int staticRequestHandler(
		std::string filename,
		std::map<std::string, std::string>* responseHeaders,
		std::stringstream* responseBody
	);

	int captureRequestHandler(
		std::string filename,
		std::map<std::string, std::string>* responseHeaders,
		std::stringstream* responseBody
	);

	int configRequestHandler(
		std::string filename,
		std::map<std::string, std::string>* responseHeaders,
		std::stringstream* responseBody
	);

private:
	int32_t                    m_refCount;
	std::vector<DeviceProber*> m_deviceProbers;
	std::vector<ImageEncoder*> m_imageEncoders;

	char                       m_hostname[BUFSIZ];

	MHD_Daemon*                m_daemon;
};

#endif
