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

	std::cout
		<< std::endl
		<< "\tListening to http://127.0.0.1:" << HttpServer::PORT << std::endl
		<< "\tBrowse there for Pictures of the captured Material" << std::endl
		<< std::endl;
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
	if(method == "GET" && url == "/") {
		return indexRequestHandler(responseHeaders, responseBody);
	} else {
		return MHD_HTTP_NOT_FOUND;
	}
}
int HttpServer::indexRequestHandler(
	std::map<std::string, std::string>* responseHeaders,
	std::stringstream* responseBody
) {
	(*responseHeaders)["Content-Type"] = "text/html";

	char hostname[BUFSIZ];
	gethostname(hostname, BUFSIZ);

	(*responseBody) <<
"<!DOCTYPE html>"
"<html>"
"	<head>"
"		<meta http-equiv=\"refresh\" content=\"1; URL=/\">"
"		<style>"
"			body {"
"				font-family: sans-serif;"
"			}"
""
"			.error { color: #FF3333; }"
"			.warning { color: #FFAD33; }"
"			.good { color: #4CAF50; }"
""
"			table {"
"				border: 2px solid #cccccc;"
"				border-collapse: collapse;"
"			}"
"			table td, table th {"
"				border-left: 1px solid #cccccc;"
"				border-right: 1px solid #cccccc;"
"				padding: 5px 10px;"
"			}"
"			table tr:nth-child(odd) td {"
"				background-color: #f0f0f0;"
"			}"
"			table tr.no-signal {"
"				color: #a0a0a0;"
"			}"
"			table td.none {"
"				text-align: center;"
"			}"
"			table td img {"
"				width: 160px;"
"				height: 90px;"
"			}"
"		</style>"
"	</head>"
"	<body>"
"		<h1>DecklinkDebugger on <u>" << hostname << "</u></h1>"
"		<table>"
"			<thead>"
"				<th>#</th>"
"				<th>DeviceName</th>"
"				<th>CanAutodetect</th>"
"				<th>GetSignalDetected</th>"
"				<th>ActiveConnection</th>"
"				<th>DetectedMode</th>"
"				<th>Thumbnail</th>"
"			</thead>"
"			<tbody>";

	int deviceIndex = 0;

	for(DeviceProber* deviceProber : m_deviceProbers)
	{
		(*responseBody) <<
"				<tr class=\"" << (deviceProber->GetSignalDetected() ? "signal" : "no-signal") << "\">"
"					<td>" << deviceIndex << "</td>"
"					<td>" << deviceProber->GetDeviceName() << "</td>"
"					<td>" << deviceProber->CanAutodetect() << "</td>"
"					<td>" << deviceProber->GetSignalDetected() << "</td>"
"					<td>" << videoConnectionToString(deviceProber->GetActiveConnection()) << "</td>"
"					<td>" << deviceProber->GetDetectedMode() << "</td>"
"					<td>";

		if(deviceProber->GetSignalDetected())
		{
			(*responseBody) <<
"						<a href=\"/capture/" << deviceIndex << ".png\">"
"							<img src=\"/capture/" << deviceIndex << ".png\">"
"						</a>";
		} else {
			(*responseBody) <<
"						<img src=\"/static/no-capture.png\">";
		}

		(*responseBody) <<
"					</td>"
"				</tr>";

		deviceIndex++;
	}

	if (deviceIndex == 0) {
		(*responseBody) <<
"				<tr>"
"					<td colspan=\"7\" class=\"none\">"
"						No DeckLink devices found"
"					</td>"
"				</tr>";
	}

	(*responseBody) <<
"			</tbody>"
"		</table>"
"	</body>"
"</html>" << std::endl;

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
