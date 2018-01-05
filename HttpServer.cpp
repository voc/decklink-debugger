#include "HttpServer.h"

#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <microhttpd.h>

int requestHandlerProxy(
	void *cls,
	struct MHD_Connection *connection,
	const char *url,
	const char *method,
	const char *version,
	const char *upload_data,
	size_t *upload_data_size,
	void **con_cls);

HttpServer::HttpServer(std::list<DeviceProber*> deviceProbers) :
	m_refCount(1),
	m_deviceProbers(deviceProbers)
{
	m_daemon = MHD_start_daemon(
		/* flags */   MHD_USE_SELECT_INTERNALLY,
		/* port */    HttpServer::PORT,
		/* apc */     NULL,
		/* apc-cls */ NULL,
		/* dh */      requestHandlerProxy,
		/* dh-cls */  this,
		MHD_OPTION_END
	);

	if (m_daemon == NULL) {
		std::cerr << "Error starting HTTP-Server, is Port "
			<< HttpServer::PORT << " in use?" << std::endl;
		exit(1);
	}

	std::cout << "Listening to http://127.0.0.1:" << HttpServer::PORT << std::endl << std::endl;

}

ULONG HttpServer::AddRef(void)
{
	return __sync_add_and_fetch(&m_refCount, 1);
}

ULONG HttpServer::Release(void)
{
	int32_t newRefValue = __sync_sub_and_fetch(&m_refCount, 1);
	if (newRefValue == 0)
	{
		MHD_stop_daemon(m_daemon);

		delete this;
		return 0;
	}
	return newRefValue;
}

int HttpServer::requestHandler(
	std::string method,
	std::string url,
	std::map<std::string, std::string>* responseHeaders,
	std::stringstream* responseBody
) {
	(*responseHeaders)["Content-Type"] = "text/html";
	(*responseHeaders)["X-Foo"] = "bar";

	(*responseBody)
		<< "<html><body>Hello, browser at "
		<< method << " " << url
		<< "!</body></html>";

	return MHD_HTTP_OK;
}

int requestHandlerProxy(
	void *cls,
	struct MHD_Connection *connection,
	UNUSED const char *url,
	UNUSED const char *method,
	UNUSED const char *version,
	UNUSED const char *upload_data,
	UNUSED size_t *upload_data_size,
	UNUSED void **con_cls)
{
	HttpServer* httpServer = (HttpServer*) cls;

	std::stringstream responseBody;
	std::map<std::string, std::string> responseHeaders;

	int status_code = httpServer->requestHandler(
		std::string(method),
		std::string(url),
		&responseHeaders,
		&responseBody);

	struct MHD_Response *response = MHD_create_response_from_buffer(
		responseBody.str().size(),
		(void*)responseBody.str().c_str(),
		MHD_RESPMEM_MUST_COPY
	);

	for(const auto entry : responseHeaders)
	{
		MHD_add_response_header(response, entry.first.c_str(), entry.second.c_str());
	}

	int ret = MHD_queue_response(connection, status_code, response);
	MHD_destroy_response(response);
	return ret;
}
