#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "AsyncLogger.hpp"
#include "AsyncTCPIOProcessor.hpp"
#include "ConfigDirective.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "HTTPServer.hpp"
#include <map>
#include <string>
#include <vector>

class WebServer
{
  private:
	typedef std::map<int, AsyncTCPIOProcessor> _TCPProcMap;
	typedef std::vector<HTTP::Server> _Servers;
	typedef std::map<int, _Servers> _ServerMap;
	typedef std::map<int, HTTP::Request> _ReqBufFdMap;
	typedef std::map<int, _ReqBufFdMap> _ReqBufPortMap;

	size_t _max_body_size;
	std::string _upload_store;
	_TCPProcMap _tcp_procs;
	_ServerMap _servers;
	_ReqBufPortMap _request_buffer;
	AsyncLogger &_logger;

	WebServer(void);

	void parseMaxBodySize(const ConfigContext &root_context);
	void parseUploadStore(const ConfigContext &root_context);
	void parseServer(const ConfigContext &server_context);

	void parseRequestForEachFd(int port, AsyncTCPIOProcessor &tcp_proc);
	void registerRequest(int port, int client_fd, HTTP::Request &request);
	void retrieveResponseForEachFd(int port, _Servers &servers);

  public:
	WebServer(const ConfigContext &root_context);
	~WebServer();
	WebServer(const WebServer &orig);
	WebServer &operator=(const WebServer &orig);

	void task(void);
};

#endif
