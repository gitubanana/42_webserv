#include "HTTP/Request.hpp"
#include "../const_values.hpp"
#include "../utils.hpp"
#include <iostream>

static const char *consume_exc_description[]
	= {"Empty line found", "Invalid format", "Invalid header field",
	   "Invalid header value"};

static const char *parse_state_str[]
	= {"start line", "header", "body", "chunk", "trailer", "CRLF after chunk"};

bool isSpace(const char c)
{
	return (c == ' ' || c == '\t');
}

HTTP::Request::Request(void)
	: _method(METHOD_NONE), _current_state(PARSE_STATE_STARTLINE)
{
}

HTTP::Request::~Request()
{
}

HTTP::Request::Request(const Request &orig)
	: _method(orig._method), _uri(orig._uri),
	  _version_string(orig._version_string), _version(orig._version),
	  _header(orig._header), _body(orig._body),
	  _current_state(orig._current_state)
{
}

HTTP::Request &HTTP::Request::operator=(const HTTP::Request &orig)
{
	if (this != &orig)
	{
		_method = orig._method;
		_uri = orig._uri;
		_version_string = orig._version_string;
		_version = orig._version;
		_header = orig._header;
		_body = orig._body;
		_current_state = orig._current_state;
	}
	return (*this);
}

int HTTP::Request::parse(std::string &buffer)
{
	// TODO: 디버그 완료 후 하단 변수 삭제
	const char *state_names[] = {"STARTLINE", "HEADER",  "BODY",
								 "CHUNK",     "TRAILER", "CRLF AFTER CHUNK"};
	const char *code_names[] = {"OK", "INVALID", "AGAIN", "IN PROCESS"};

	while (true)
	{
		int rc;

		std::cout << __func__ << ": current state is "
				  << state_names[_current_state] << std::endl;
		switch (_current_state)
		{
		case PARSE_STATE_STARTLINE:
			rc = consumeStartLine(buffer);
			std::cout << "Got return code " << code_names[rc] << std::endl;

			if (rc == RETURN_TYPE_OK)
				_current_state = PARSE_STATE_HEADER;
			else if (rc == RETURN_TYPE_AGAIN)
				return (RETURN_TYPE_AGAIN);
			break;

		case PARSE_STATE_HEADER:
			rc = consumeHeader(buffer);
			std::cout << "Got return code " << code_names[rc] << std::endl;

			if (rc == RETURN_TYPE_OK)
			{
				if (!_header.hasValue("Host") && !_header.hasValue("Trailer","Host"))
				{
					_error_offset = 0;
					std::cout << __func__ << "Header must include Host header field" << std::endl;
					throwException(CONSUME_EXC_INVALID_VALUE);
				}
				if (_header.hasValue("Transfer-Encoding", "chunked"))
				{
					// Transfer coding == chunked라면
					std::cout << "header has Transfer-Encoding == chunked"
							  << std::endl;
					if (_header.hasValue("Trailer"))
					{
						/** 유효한 Trailer 헤더 필드 값을 가지고 있는지 확인**/
						_trailer_values = _header.getValues("Trailer");
						if (_trailer_values.empty())
						{
							_error_offset = 0;
							throwException(CONSUME_EXC_EMPTY_LINE);
						}
						std::vector<std::string>::const_iterator iter
							= _trailer_values.begin();
						for (; iter != _trailer_values.end(); ++iter)
						{
							if (*iter == "Trailer" || *iter == "Content-Length"
								|| *iter == "Transfer-Encoding")
							{
								_error_offset = 0;
								throwException(CONSUME_EXC_INVALID_FIELD);
							}
						}
						std::cout << "header has Trailer" << std::endl;
					}
					_current_state = PARSE_STATE_CHUNK;
				}
				else if (_header.hasValue("Trailer"))
				{
					_error_offset = 0;
					std::cout << __func__
							  << ": Trailer with no Transfer-Encoding"
							  << std::endl;
					throwException(CONSUME_EXC_INVALID_VALUE);
				}
				else if (_header.hasValue("Content-Length"))
				{
					// Content-length >= 0이라면
					const std::string &content_length
						= getHeaderValue("Content-Length", 0);
					std::cout << "header has Content-Length" << std::endl;

					std::cout << "content-length : " << content_length
							  << std::endl;
					_content_length = toNum<int>(content_length);
					if (_content_length >= 0)
					{
						std::cout << "header has Content-Length >= 0"
								  << std::endl;
						_current_state = PARSE_STATE_BODY;
					}
				}
				else
					return (RETURN_TYPE_OK);
			}
			else if (rc == RETURN_TYPE_AGAIN)
				return (RETURN_TYPE_AGAIN);
			else
			{
				std::cout << "parsing ing..." << std::endl;
			}
			break;

		case PARSE_STATE_BODY:
			rc = consumeBody(buffer);
			std::cout << "Got return code " << code_names[rc] << std::endl;
			return (rc);
			break;

		case PARSE_STATE_CHUNK:
			rc = consumeChunk(buffer);
			std::cout << "Got return code " << code_names[rc] << std::endl;

			if (rc == RETURN_TYPE_OK)
			{
				if (_header.hasValue("Trailer"))
				{
					std::cout << "header has Trailer" << std::endl;
					_current_state = PARSE_STATE_TRAILER;
				}
				else
					_current_state = PARSE_STATE_CRLF_AFTER_CHUNK;
			}
			else if (rc == RETURN_TYPE_AGAIN)
				return (RETURN_TYPE_AGAIN);
			else
				;
			break;

		case PARSE_STATE_TRAILER:
			rc = consumeTrailer(buffer);
			std::cout << "Got return code " << code_names[rc] << std::endl;
			if (rc == RETURN_TYPE_AGAIN)
				return (RETURN_TYPE_AGAIN);
			else
				_current_state = PARSE_STATE_CRLF_AFTER_CHUNK;
			break;
		case PARSE_STATE_CRLF_AFTER_CHUNK:
			rc = consumeCRLF(buffer);
			std::cout << "Got return code " << code_names[rc] << std::endl;
			return (rc);
			break;
		}
	}
}

void HTTP::Request::throwException(int code) const
{
	std::stringstream what;

	if (code < 0 || code > CONSUME_EXC_INVALID_VALUE)
		what << "Unknown error";
	else
		what << consume_exc_description[code];
	what << ": " << parse_state_str[_current_state];
	throw(std::runtime_error(what.str()));
}

bool HTTP::Request::hasHeaderValue(std::string const &name,
								   std::string const &value) const
{
	return (_header.hasValue(name, value));
}

bool HTTP::Request::hasHeaderValue(const Header::const_iterator &name_iter,
								   const std::string &value) const
{
	return (_header.hasValue(name_iter, value));
}

bool HTTP::Request::hasHeaderValue(std::string const &name) const
{
	return (_header.hasValue(name));
}

const std::string &HTTP::Request::getHeaderValue(std::string const &name,
												 int idx) const
{
	return (_header.getValue(name, idx));
}

int HTTP::Request::getMethod(void) const
{
	return (_method);
}

const std::string &HTTP::Request::getURIPath(void) const
{
	return (_uri);
}

const std::string	&HTTP::Request::getBody() const
{
	return (_body);
}
