#ifndef SERVER_HPP
#define SERVER_HPP

#include "ConfigDirective.hpp"
#include "HTTP/Request.hpp"
#include "HTTP/RequestHandler.hpp"
#include "HTTP/Response.hpp"
#include "async/TCPIOProcessor.hpp"
#include <queue>
#include <set>
#include <string>
#include <vector>

namespace HTTP
{
class Server
{
  private:
	class Location
	{
	  private:
		bool _has_index;
		bool _do_redirection;
		bool _autoindex;
		std::string _path;
		std::string _root;
		std::vector<std::string> _index;
		std::pair<int, std::string> _redirection;
		std::set<int> _allowed_methods;

	  public:
		Location();
		Location(const ConfigContext &location_context);
		~Location();
		Location(const Location &orig);
		Location &operator=(const Location &orig);

		void parseDirectiveRoot(const ConfigContext &location_context);
		void parseDirectiveLimitExcept(const ConfigContext &location_context);
		void parseDirectiveReturn(const ConfigContext &location_context);
		void parseDirectiveAutoIndex(const ConfigContext &location_context);
		void parseDirectiveIndex(const ConfigContext &location_context);
		const std::string &getPath(void) const;

		// 추가
		const std::string &getRoot(void) const;
		bool isAllowedMethod(int method);
	};

	int _port;
	std::set<std::string> _server_name;
	std::map<int, std::string> _error_pages;
	std::map<std::string, Location> _locations;
	std::map<int, std::vector<RequestHandler> > _request_handlers;
	std::map<int, std::queue<Response> > _output_queue;

	void parseDirectiveListen(const ConfigContext &server_context);
	void parseDirectiveErrorPage(const ConfigContext &server_context);
	void parseDirectiveServerName(const ConfigContext &server_context);
	void parseDirectiveLocation(const ConfigContext &server_context);
	void ensureClientConnected(int client_fd);
	static bool isValidStatusCode(const int &status_code);

	// 추가
	void getMethodHandler(HTTP::Request &request, HTTP::Response &response,
						  const std::string &resource_path);
	void headMethodHandler(HTTP::Request &request, HTTP::Response &response,
						   const std::string &resource_path);
	void postMethodHandler(HTTP::Request &request, HTTP::Response &response,
						   const std::string &resource_path);
	void deleteMethodHandler(HTTP::Request &request, HTTP::Response &response,
							 const std::string &resource_path);
	std::string processURI(const HTTP::Server::Location &location,
						   std::string uri);

  public:
	Server(const ConfigContext &server_context);
	~Server();
	Server(const Server &orig);
	Server &operator=(const Server &orig);

	int getPort(void) const;
	void task(void);
	bool isForMe(const Request &request);
	void registerRequest(int client_fd, const Request &request);
	Response retrieveResponse(int client_fd);
	int hasResponses(void);
	bool hasResponses(int client_fd);
	void disconnect(int client_fd);

	// 추가
	void methodHandler(HTTP::Request &Request, int fd);
	const Location &getLocation(const std::string &location);
	void setErrorPage(HTTP::Response &response, int status_code);
};
} // namespace HTTP

#endif