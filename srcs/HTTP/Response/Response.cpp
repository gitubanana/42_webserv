#include "HTTP/Response.hpp"
#include "HTTP/const_values.hpp"
#include "utils/ansi_escape.h"
#include "utils/string.hpp"
#include "utils/file.hpp"
#include <ctime>
#include <dirent.h>

using namespace HTTP;

const std::string Response::_http_version = "HTTP/1.1";

Response::Response(void) : _logger(async::Logger::getLogger("Response"))
{
	initGeneralHeaderFields();
	initResponseHeaderFields();
	initEntityHeaderFields();
}

Response::Response(Header header)
	: _header(header), _logger(async::Logger::getLogger("Response"))
{
}

Response::Response(Response const &other)
	: _response(other._response), _status_code(other._status_code),
	  _reason_phrase(other._reason_phrase), _header(other._header),
	  _body(other._body), _logger(other._logger)
{
}

Response &Response::operator=(Response const &other)
{
	if (this != &other)
	{
		_response = other._response;
		_status_code = other._status_code;
		_reason_phrase = other._reason_phrase;
		_header = other._header;
		_body = other._body;
	}
	return (*this);
}

Response::~Response(void)
{
}

// convert to string
const std::string &Response::toString(void)
{
	setDate();
	_response.clear();
	makeStatusLine();
	makeHeader();
	makeBody();
	return (_response);
}

void Response::makeStatusLine(void)
{
	_response.append(_http_version);
	_response.append(SP);
	_response.append(_status_code);
	_response.append(SP);
	_response.append(_reason_phrase);
	_response.append(CRLF);
}

void Response::makeHeader(void)
{
	for (Header::const_iterator it = _header.begin(); it != _header.end(); ++it)
	{
		if (it->second.empty())
			continue;

		std::string to_append(it->first + ": ");

		const std::vector<std::string> &values = _header.getValues(it->first);
		for (std::vector<std::string>::const_iterator val_it = values.begin();
			 val_it != values.end(); ++val_it)
		{
			to_append += *val_it;
			if (val_it + 1 != values.end())
				to_append += ", ";
		}
		to_append += CRLF;
		_response.append(to_append);
	}
	_response.append(CRLF);
}

void Response::makeBody(void)
{
	_response.append(_body);
}

void Response::makeDirectoryListing(const std::string &path,
									const std::string &uri)
{
	setContentType("text/html");

	_body.clear();
	_body.append("<html>\n"
				 "<head><title>Index of "
				 + uri
				 + "</title></head>\n"
				   "<body>\n"
				   "<h1>Index of "
				 + uri
				 + "</h1>\n"
				   "<hr><pre><a href=\"../\">../</a>\n");

	/* 각 파일에 대한 내용을 추가 */
	DIR *dir_stream;
	struct dirent *dir_info;
	std::string file_name;
	std::string file_path;
	std::string file_size;

	dir_stream = ::opendir(path.c_str());
	if (dir_stream == NULL)
		perror("opendir error");

	for (int i = 0; i < 2; ++i)
		dir_info = ::readdir(dir_stream);
	while ((dir_info = ::readdir(dir_stream)) != NULL)
	{
		file_name = dir_info->d_name;
		file_path = path + "/" + file_name;
		if (::isDirectory(file_path))
			file_name += '/';
		_body.append("<a href=\"" + file_name + "\">" + file_name + "</a>\n");
	}
	::closedir(dir_stream);

	_body.append("</pre><hr></body>\n"
				 "</html>\n");

	setContentLength(_body.length());
}

const std::string Response::getDescription(void) const
{
	const size_t bodylen = 20;
	const int status_code = toNum<int>(_status_code);

	std::stringstream buf;
	if (status_code < 200)
		buf << ANSI_BCYAN;
	else if (200 <= status_code && status_code < 400)
		buf << ANSI_BGREEN;
	else if (400 <= status_code && status_code < 500)
		buf << ANSI_BYELLOW;
	else
		buf << ANSI_BRED;
	buf << "[" << _status_code << " " << _reason_phrase << " | ";
	if (_body.size() > bodylen)
		buf << _body.substr(0, bodylen - 3) << "...";
	else
		buf << _body;
	buf << "]" << ANSI_RESET;
	return (buf.str());
}
