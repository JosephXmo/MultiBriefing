#ifndef _RMHDATA_H_
#define _RMHDATA_H_

#pragma once

// C Standard Library
#include <iostream>
#include <sstream>

// Windows API Headers
#include <WinSock2.h>

// Project Constant Values & Enumerations
#include "RMHFoundations.h"

namespace ReturnMyHairHTTP {
	class Request {
	public:
		Request(HTTPMethod method = HTTPMethod::_GET,
			std::string pagePath = "index.html",
			HTTPVersion version = HTTPVersion::_1_0) :
			method(method),
			pagePath(pagePath),
			version(version),
			body("") {}
		~Request() = default;

		HTTPMethod getMethod(void);
		std::string getLiteralMethod(void);
		std::string getPagePath(void);
		HTTPVersion getHTTPVersion(void);
		std::string getLiteralHTTPVersion(void);
		std::string getHeader(std::string key);
		std::string getBody(void);
		std::string getHead(void);

		void setMethod(HTTPMethod method);
		void setPagePath(std::string path);
		void setHTTPVersion(HTTPVersion version);
		void setHeader(std::string key, std::string value);
		void setBody(std::string content);

		void resolveRequest(std::string rawRequest);
		void report(void);

	private:
		HTTPMethod method;
		std::string pagePath;
		HTTPVersion version;
		std::unordered_map<std::string, std::string> headers;
		std::string body;
	};

	class Response {
	public:
		Response(HTTPVersion version = HTTPVersion::_1_0, short statusCode = 500) :
			version(version),
			statusCode(statusCode),
			body("") {}
		~Response() = default;

		HTTPVersion getHTTPVersion(void);
		std::string getLiteralHTTPVersion(void);
		short getHTTPStatusCode(void);
		std::string getHTTPStatusPrompt(void);
		std::string getHeader(std::string key);
		std::string getBody(void);
		std::string getHead(void);

		void setHTTPVersion(HTTPVersion version);
		void setHTTPStatusCode(short statusCode);
		void setHeader(std::string key, std::string value);
		void setBody(std::string content);

		std::string toString();
		void report(void);

	private:
		HTTPVersion version;
		short statusCode;
		std::unordered_map<std::string, std::string> headers;
		std::string body;
	};
}

#endif