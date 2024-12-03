#ifndef _RMHFOUNDATIONS_H_
#define _RMHFOUNDATIONS_H_

#pragma once

// C Standard Library
#include <fstream>
#include <string>
#include <unordered_map>

// REMEMBER TO SET EACH RECEIVE BUFFER LENGTH HERE
#define BUFLEN 4096
#define SAFELOOP 500

namespace ReturnMyHairHTTP {
	enum HTTPMethod {
		_GET,
		_HEAD,
		_POST,
		_PUT,
		_DELETE,
		_CONNECT,
		_OPTIONS,
		_TRACE,
		_PATCH
	};

	enum HTTPVersion {
		_1_0,
		_1_1,
		_2_0,
		_3_0
	};

	inline std::ofstream GlobalLogLocation;
	inline std::string GlobalWebRootPath;

	inline std::unordered_map<short, std::string> HTTPStatusCodeDict = {
		{100, "Continue"},
		{101, "Switching Protocols"},
		{102, "Processing"},
		{103, "Early Hints"},

		{200, "OK"},
		{201, "Created"},
		{202, "Accepted"},
		{203, "Non-Authoritative Information"},
		{204, "No Content"},
		{205, "Reset Content"},
		{206, "Partial Content"},
		{207, "Multi-Status"},
		{208, "Already Reported"},

		{300, "Multiple Choices"},
		{301, "Moved Permanently"},
		{302, "Found"},
		{303, "See Other"},
		{304, "Not Modified"},
		{307, "Temporary Redirect"},
		{308, "Permanent Redirect"},

		{400, "Bad Request"},
		{401, "Unauthorized"},
		{402, "Payment Required"},
		{403, "Forbidden"},
		{404, "Not Found"},
		{405, "Method Not Allowed"},
		{406, "Not Acceptable"},
		{407, "Proxy Authentication Required"},
		{408, "Request Timeout"},
		{409, "Conflict"},
		{410, "Gone"},
		{411, "Length Required"},
		{412, "Precondition Failed"},
		{413, "Content Too Large"},
		{414, "URI Too Long"},
		{415, "Unsupported Media Type"},
		{416, "Range Not Satisfiable"},
		{417, "Expectation Failed"},
		{421, "Misdirected Request"},
		{422, "Unprocessable Content"},
		{423, "Locked"},
		{424, "Failed Dependency"},
		{425, "Too Early"},
		{426, "Upgrade Required"},
		{428, "Precondition Required"},
		{429, "Too Many Requests"},
		{431, "Request Header Field Too Large"},
		{451, "Unavailable For Legal Reasons"},

		{500, "Internal Server Error"},
		{501, "Not Implemented"},
		{502, "Bad Gateway"},
		{503, "Service Unavailable"},
		{504, "Gatewat Timeout"},
		{505, "HTTP Version Not Supported"},
		{506, "Variant Also Negotiates"},
		{507, "Insufficient Storage"},
		{508, "Loop Detected"},
		{510, "Not Extended"},
		{511, "Network Authentication Required"},

		{999, "Server Program Fault"}
	};
}

#endif // !_RMHFOUNDATIONS_H_
