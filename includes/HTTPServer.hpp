#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "AsyncTCPIOProcessor.hpp"
#include "ConfigDirective.hpp"
#include <set>
#include <string>
#include <vector>

enum e_http_method
{
	GET = 0,
	HEAD,
	POST,
	PUT,
	DELETE,
	CONNECT,
	OPTIONS,
	TRACE
};

class HTTPServer
{
  private:
	class HTTPLocation
	{
	  private:
		bool has_index;
		bool do_redirection;
		bool autoindex;
		std::string path;
		std::string root;
		std::vector<std::string> index;
		std::pair<int, std::string> redirection;
		std::set<int> allowed_methods;

	  public:
		HTTPLocation();
		HTTPLocation(const ConfigContext &location_context);
		~HTTPLocation();
		HTTPLocation(const HTTPLocation &orig);
		HTTPLocation &operator=(const HTTPLocation &orig);

		void parseDirectiveRoot(const ConfigContext &location_context);
		void parseDirectiveLimitExcept(const ConfigContext &location_context);
		void parseDirectiveReturn(const ConfigContext &location_context);
		void parseDirectiveAutoIndex(const ConfigContext &location_context);
		void parseDirectiveIndex(const ConfigContext &location_context);
		const std::string &getPath(void);
	};

	int port;
	std::string server_name;
	std::map<int, std::string> error_pages;
	std::map<std::string, HTTPLocation> locations;
	static const std::map<std::string, int> http_methods;

	void parseDirectiveListen(const ConfigContext &server_context);
	void parseDirectiveErrorPage(const ConfigContext &server_context);
	void parseDirectiveServerName(const ConfigContext &server_context);
	void parseDirectiveLocation(const ConfigContext &server_context);

  public:
	HTTPServer(const ConfigContext &server_context);
	~HTTPServer();
	HTTPServer(const HTTPServer &orig);
	HTTPServer &operator=(const HTTPServer &orig);
};

#endif
