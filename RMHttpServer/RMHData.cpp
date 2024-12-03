#include "RMHData.h"
#include "RMHUtils.h"

namespace ReturnMyHairHTTP {
	HTTPMethod Request::getMethod(void) { return this->method; }

	std::string Request::getLiteralMethod(void) {
		switch (this->method) {
		case HTTPMethod::_GET:
			return "GET";
			break;
		case HTTPMethod::_HEAD:
			return "HEAD";
			break;
		case HTTPMethod::_POST:
			return "POST";
			break;
		default:
			return "Bad HTTP Method.";
			break;
		}
	}

	std::string Request::getPagePath(void) { return this->pagePath; }

	HTTPVersion Request::getHTTPVersion(void) { return this->version; }

	std::string Request::getLiteralHTTPVersion(void) {
		switch (this->version) {
		case HTTPVersion::_1_0:
			return "1.0";
			break;
		case HTTPVersion::_1_1:
			return "1.1";
			break;
		case HTTPVersion::_2_0:
			return "2.0";
			break;
		case HTTPVersion::_3_0:
			return "3.0";
			break;
		default:
			return "Bad HTTP Version.";
			break;
		}
	}

	std::string Request::getHeader(std::string key) {
		auto it = this->headers.find(key);
		if (it != headers.end()) {
			return it->second;
		}

		return "";
	}

	std::string Request::getBody(void) {
		return this->body;
	}

	std::string Request::getHead(void) {
		return this->getLiteralMethod() + " /" + std::filesystem::relative(this->getPagePath(), GlobalWebRootPath).string() + " HTTP/" + this->getLiteralHTTPVersion();
	}

	void Request::setMethod(HTTPMethod method) {
		this->method = method;
	}

	void Request::setPagePath(std::string path) {
		this->pagePath = path;
	}

	void Request::setHTTPVersion(HTTPVersion version) {
		this->version = version;
	}

	void Request::setHeader(std::string key, std::string value) {
		this->headers[key] = value;
	}

	void Request::setBody(std::string content) {
		this->body = content + '\n';
	}

	void Request::resolveRequest(std::string rawRequest) {
		std::istringstream strStream(rawRequest);
		std::string lineBuffer;

		if (std::getline(strStream, lineBuffer)) {
			std::istringstream irl(lineBuffer);
			std::string methodText, pagePathText, versionText;
			if (!(irl >> methodText >> pagePathText >> versionText)) {
				log("Invalid HTTP initial request line", GlobalLogLocation);
				return;
			}

			try {
				this->setPagePath(resolveSafePath(pagePathText));
			}
			catch (const std::exception e) {
				this->setPagePath(pagePathText);
				throw e;
			}

			if (methodText == "GET")
				this->setMethod(HTTPMethod::_GET);
			else if (methodText == "POST")
				this->setMethod(HTTPMethod::_POST);
			else if (methodText == "HEAD")
				this->setMethod(HTTPMethod::_HEAD);
			else if (methodText == "PUT")
				this->setMethod(HTTPMethod::_PUT);
			else if (methodText == "DELETE")
				this->setMethod(HTTPMethod::_DELETE);
			else if (methodText == "CONNECT")
				this->setMethod(HTTPMethod::_CONNECT);
			else if (methodText == "OPTIONS")
				this->setMethod(HTTPMethod::_OPTIONS);
			else if (methodText == "TRACE")
				this->setMethod(HTTPMethod::_TRACE);
			else if (methodText == "PATCH")
				this->setMethod(HTTPMethod::_PATCH);
			else
				log("Unsupported HTTP method: " + methodText, GlobalLogLocation);

			if (versionText == "HTTP/1.0")
				this->setHTTPVersion(HTTPVersion::_1_0);
			else if (versionText == "HTTP/1.1")
				this->setHTTPVersion(HTTPVersion::_1_1);
			else if (versionText == "HTTP/2.0")
				this->setHTTPVersion(HTTPVersion::_2_0);
			else if (versionText == "HTTP/3.0")
				this->setHTTPVersion(HTTPVersion::_3_0);
			else
				log("Unsupported HTTP version: " + versionText, GlobalLogLocation);
		}

		while (std::getline(strStream, lineBuffer) && lineBuffer != "\r") {
			std::istringstream headerStream(lineBuffer);
			std::string keyBuffer, valueBuffer;

			if (std::getline(headerStream, keyBuffer, ':')) {
				std::getline(headerStream, valueBuffer);

				// Trim spaces at both ends of key and value
				keyBuffer.erase(keyBuffer.begin(), std::find_if(keyBuffer.begin(), keyBuffer.end(), [](unsigned char ch) { return !std::isspace(ch); }));
				keyBuffer.erase(std::find_if(keyBuffer.rbegin(), keyBuffer.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), keyBuffer.end());

				valueBuffer.erase(valueBuffer.begin(), std::find_if(valueBuffer.begin(), valueBuffer.end(), [](unsigned char ch) { return !std::isspace(ch); }));
				valueBuffer.erase(std::find_if(valueBuffer.rbegin(), valueBuffer.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), valueBuffer.end());
			}

			if (!keyBuffer.empty() && !valueBuffer.empty())
				this->setHeader(keyBuffer, valueBuffer);
		}

		this->body = "";
		while (std::getline(strStream, lineBuffer))
			body += lineBuffer + '\n';
		if (!this->body.empty())
			this->body.pop_back();
	}

	void Request::report(void) {
		std::cout << "Method: " << this->getLiteralMethod() << std::endl;
		std::cout << "Requested resource: " << this->getPagePath() << std::endl;
		std::cout << "HTTP version: " << this->getLiteralHTTPVersion() << std::endl;
		for (std::pair<std::string, std::string> kvPair : this->headers)
			std::cout << kvPair.first << ": " << kvPair.second << std::endl;
		std::cout << "Body:" << std::endl << this->getBody() << std::endl;
	}

	HTTPVersion Response::getHTTPVersion(void) {
		return this->version;
	}

	std::string Response::getLiteralHTTPVersion(void) {
		std::string versionText = "";

		switch (this->version) {
		case HTTPVersion::_1_0:
			versionText = "HTTP/1.0";
			break;
		case HTTPVersion::_1_1:
			versionText = "HTTP/1.1";
			break;
		case HTTPVersion::_2_0:
			versionText = "HTTP/2.0";
			break;
		case HTTPVersion::_3_0:
			versionText = "HTTP/3.0";
			break;
		default:
			versionText = "Unidentified";
		}

		return versionText;
	}

	short Response::getHTTPStatusCode(void) {
		return this->statusCode;
	}

	std::string Response::getHTTPStatusPrompt(void) {
		return HTTPStatusCodeDict[this->statusCode];
	}

	std::string Response::getHeader(std::string key) {
		auto it = this->headers.find(key);
		if (it != headers.end()) {
			return it->second;
		}

		return "";
	}

	std::string Response::getBody(void) {
		return this->body;
	}

	std::string Response::getHead(void) {
		return this->getLiteralHTTPVersion() + ' ' + std::to_string(this->statusCode) + ' ' + this->getHTTPStatusPrompt();
	}

	void Response::setHTTPVersion(HTTPVersion version) {
		this->version = version;
	}

	void Response::setHTTPStatusCode(short statusCode) {
		if (HTTPStatusCodeDict.contains(statusCode))
			this->statusCode = statusCode;
		else
			throw std::runtime_error("Unidentified HTTP status code");
	}

	void Response::setHeader(std::string key, std::string value) {
		this->headers[key] = value;
	}

	void Response::setBody(std::string content) {
		this->body = content;
	}

	std::string Response::toString() {
		std::string literalHeaders;
		for (std::pair<std::string, std::string> kvPair : this->headers)
			literalHeaders = literalHeaders + kvPair.first + ": " + kvPair.second + "\r\n";

		return this->getLiteralHTTPVersion() + ' ' + std::to_string(this->statusCode) + ' ' + this->getHTTPStatusPrompt() + "\r\n" +
			literalHeaders + "\r\n" + this->body;
	}

	void Response::report(void) {
		std::cout << "HTTP Version:		" << this->getLiteralHTTPVersion() << std::endl;
		std::cout << "Status Code:		" << this->getHTTPStatusCode() << std::endl;
		std::cout << "Status Prompt:	" << this->getHTTPStatusPrompt() << std::endl;
		for (std::pair<std::string, std::string> kvPair : this->headers)
			std::cout << kvPair.first << ": " << kvPair.second << std::endl;
		std::cout << "Body:" << std::endl << this->getBody() << std::endl;
	}
}