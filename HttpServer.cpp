#include "HttpServer.h"

#include <iostream>
#include <sstream>

#include <assert.h>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <microhttpd.h>

#include "rc.h"

int requestHandlerProxy(
	void *cls,
	struct MHD_Connection *connection,
	const char *url,
	const char *method,
	const char *version,
	const char *upload_data,
	size_t *upload_data_size,
	void **con_cls);

HttpServer::HttpServer(std::vector<DeviceProber*> deviceProbers) :
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

	gethostname(m_hostname, BUFSIZ);

	if (m_daemon == NULL) {
		std::cerr << "Error starting HTTP-Server, is Port "
			<< HttpServer::PORT << " in use?" << std::endl;
		exit(1);
	}

	std::cout
		<< std::endl
		<< "\tListening to http://" << m_hostname << ":" << HttpServer::PORT << std::endl
		<< "\tBrowse there for Pictures of the captured Material" << std::endl
		<< std::endl;

	for(DeviceProber* deviceProber : deviceProbers)
	{
		m_imageEncoders.push_back(new ImageEncoder(deviceProber));
	}
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
		for(ImageEncoder* imageEncoder : m_imageEncoders)
		{
			assert(imageEncoder->Release() == 0);
		}

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
	}
	else if(method == "GET" && url.find("/static/") == 0) {
		return staticRequestHandler(url.substr(sizeof("/static/") - 1), responseHeaders, responseBody);
	}
	else if(method == "GET" && url.find("/capture/") == 0) {
		return captureRequestHandler(url.substr(sizeof("/capture/") - 1), responseHeaders, responseBody);
	}
	else if(method == "GET" && url.find("/config/") == 0) {
		return configRequestHandler(url.substr(sizeof("/config/") - 1), responseHeaders, responseBody);
	}
	else
	{
		return MHD_HTTP_NOT_FOUND;
	}
}

int HttpServer::staticRequestHandler(
	std::string filename,
	std::map<std::string, std::string>* responseHeaders,
	std::stringstream* responseBody
) {
	if(rcs.count(filename) == 1) {
		(*responseHeaders)["Content-Type"] = rcs[filename].mimetype;
		(*responseHeaders)["Cache-Control"] = "max-age=31536000, public";

		(*responseBody) << rcs[filename].body;
		return MHD_HTTP_OK;
	}

	return MHD_HTTP_NOT_FOUND;
}

int HttpServer::captureRequestHandler(
	std::string filename,
	std::map<std::string, std::string>* responseHeaders,
	std::stringstream* responseBody
) {
	std::string index_str = filename.substr(0, filename.find("."));
	unsigned long index;
	try {
		index = std::stoul(index_str);
	}
	catch(std::invalid_argument) {
		return MHD_HTTP_NOT_FOUND;
	}
	catch(std::out_of_range) {
		return MHD_HTTP_NOT_FOUND;
	}

	if(index < m_imageEncoders.size())
	{
		(*responseHeaders)["Content-Type"] = "image/png";

		(*responseBody) << m_imageEncoders[index]->EncodeImage();
		return MHD_HTTP_OK;
	}

	return MHD_HTTP_NOT_FOUND;
}

int HttpServer::configRequestHandler(
	std::string filename,
	std::map<std::string, std::string>* responseHeaders,
	std::stringstream* responseBody
) {
	std::string index_str = filename;
	unsigned long index;
	try {
		index = std::stoul(index_str);
	}
	catch(std::invalid_argument) {
		return MHD_HTTP_NOT_FOUND;
	}
	catch(std::out_of_range) {
		return MHD_HTTP_NOT_FOUND;
	}

	if(index < m_deviceProbers.size())
	{
		DeviceProber *deviceProber = m_deviceProbers[index];
		(*responseHeaders)["Content-Type"] = "text/html; charset=UTF-8";

	(*responseBody) <<
"<!DOCTYPE html>"
"<html>"
"	<head>"
"		<link rel=\"stylesheet\" type=\"text/css\" href=\"/static/style.css\">"
"	</head>"
"	<body>"
"		<h1>Configuration-Snippets</h1>"
"		<h2>for the c3voc cm</h2>"
"		<code>"
"…\n"
"  -\n"
"    # " << deviceProber->GetDeviceName() << "\n"
"    name: cam" << (index+1) << "\n"
"    type: decklink-internal\n"
"    devicenumber: " << index << "\n"
"    video_connection: " << videoConnectionToString(deviceProber->GetActiveConnection()) << "\n"
"    video_mode: \"" << deviceProber->GetDetectedMode() << "\"\n"
"…"
"		</code>"
""
"		<h2>for voctocore/config.ini</h2>"
"		<code>"
"…\n"
"# " << deviceProber->GetDeviceName() << "\n"
"[source.cam" << (index+1) << "]\n"
"kind = decklink\n"
"devicenumber = " << index << "\n"
"video_connection = " << videoConnectionToString(deviceProber->GetActiveConnection()) << "\n"
"video_mode = " << deviceProber->GetDetectedMode() << "\n"
"audio_connection = embedded\n"
"deinterlace = assume-progressive\n"
"…"
"		</code>"
"	</body>"
"</html>";
		return MHD_HTTP_OK;
	}
	return MHD_HTTP_NOT_FOUND;
}

int HttpServer::indexRequestHandler(
	std::map<std::string, std::string>* responseHeaders,
	std::stringstream* responseBody
) {
	(*responseHeaders)["Content-Type"] = "text/html; charset=UTF-8";

	(*responseBody) <<
"<!DOCTYPE html>"
"<html>"
"	<head>"
"		<meta http-equiv=\"refresh\" content=\"1; URL=/\">"
"		<link rel=\"stylesheet\" type=\"text/css\" href=\"/static/style.css\">"
"	</head>"
"	<body>"
"		<h1>DecklinkDebugger on <u>" << m_hostname << "</u></h1>"
"		<table>"
"			<thead>"
"				<th>#</th>"
"				<th>Device Name</th>"
"				<th>Can Input &amp; Auto-Detect</th>"
"				<th>Signal Detected</th>"
"				<th>Active Connection</th>"
"				<th>Detected Mode</th>"
"				<th>Pixel Format</th>"
"				<th>Capture</th>"
"				<th>Config</th>"
"			</thead>"
"			<tbody>";

	int deviceIndex = 0;

	for(DeviceProber* deviceProber : m_deviceProbers)
	{

		std::string deviceName = deviceProber->GetDeviceName();
		if(deviceProber->IsSubDevice())
		{
			deviceName = "↳ " + deviceName;
		}

		(*responseBody) <<
"				<tr class=\"" << (deviceProber->GetSignalDetected() ? "signal" : "no-signal") << "\">"
"					<td>" << deviceIndex << "</td>"
"					<td>" << deviceName << "</td>"
"					<td>" << boolToString(deviceProber->CanAutodetect() && deviceProber->CanInput()) << "</td>"
"					<td>" << boolToString(deviceProber->GetSignalDetected()) << "</td>"
"					<td>" << videoConnectionToString(deviceProber->GetActiveConnection()) << "</td>"
"					<td>" << deviceProber->GetDetectedMode() << "</td>"
"					<td>" << pixelFormatToString(deviceProber->GetPixelFormat()) << "</td>"
"					<td>";

		if(deviceProber->GetSignalDetected())
		{
			(*responseBody) <<
"						<a href=\"/capture/" << deviceIndex << ".png\">"
"							Capture Image"
"						</a>";
		}

		(*responseBody) <<
"					</td>"
"					<td>";

		if(deviceProber->GetSignalDetected())
		{
			(*responseBody) <<
"						<a href=\"/config/" << deviceIndex << "\">"
"							Config-Snippets"
"						</a>";
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

	int status_code;
	try {
		status_code = httpServer->requestHandler(
			std::string(method),
			std::string(url),
			&responseHeaders,
			&responseBody);
	}
	catch (const std::exception& ex) {
		responseBody << "Interal Server Error";
		status_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
	}

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
