#include "HTTP/RequestHandler.hpp"
#include "utils/file.hpp"
#include "utils/string.hpp"

using namespace HTTP;

Server::RequestHandler::RequestHandler(Server *server, const Request &request,
									   const Server::Location &location)
	: _request(request), _location(location), _server(server),
	  _status(RESPONSE_STATUS_AGAIN),
	  _resource_path(location.generateResourcePath(request)),
	  _logger(async::Logger::getLogger("RequestHandler"))
{
	if (_request.hasHeaderValue("Connection", "close"))
		_response.setConnection(false);
	else
		_response.setConnection(true);
}

Server::RequestHandler::~RequestHandler()
{
}

void Server::RequestHandler::registerErrorResponse(const int code,
												   const std::exception &e)
{
	_response = _server->generateErrorResponse(code);
	_status = Server::RequestHandler::RESPONSE_STATUS_OK;
	_logger << async::warning << e.what();
}

Response Server::RequestHandler::retrieve(void)
{
	if (_status != RESPONSE_STATUS_OK)
		throw(std::runtime_error("Response has not been made yet."));
	return (_response);
}

bool Server::RequestHandler::isDirectory(void) const
{
	return (::isDirectory(_resource_path));
}

bool Server::RequestHandler::isInvalidDirectoryFormat(void) const
{
	if (_resource_path == "/")
		return (false);
	if (isDirectory() && _resource_path.back() != '/')
		return (true);
	return (false);
}
