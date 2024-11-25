#include "RMHUtils.h"
#include <WS2tcpip.h>

namespace ReturnMyHairHTTP {
	std::string resolveSafePath(const std::string& requestedPath)
	{
		std::string requestedPathCopy = requestedPath;

		// Check if web root is defined
		if (GlobalWebRootPath.empty())
			throw std::runtime_error("Web root not defined.");

		// Auto direct to index.html if requesting web root
		if (requestedPathCopy == "/")
			requestedPathCopy = "/index.html";

		// Prevent misparse to drive root
		if (!requestedPathCopy.empty() && requestedPathCopy[0] == '/')
			// requestedPathCopy.erase(0, 1);
			requestedPathCopy.insert(requestedPathCopy.begin(), '.');

		// Combine for resource's full path
		std::filesystem::path webRootPath = std::filesystem::canonical(GlobalWebRootPath);
		std::filesystem::path requestedFullPath = webRootPath / requestedPathCopy;
		// Canonicalize path against ../ attack
		requestedFullPath = std::filesystem::weakly_canonical(requestedFullPath);

		// Check if is still in web root
		if (!std::equal(webRootPath.begin(), webRootPath.end(), requestedFullPath.begin())) {
			throw std::runtime_error("Attempt to access resource outside of web root: " + requestedFullPath.string());
		}

		return requestedFullPath.string();
	}

	std::string resolveFileMIME(std::string filename) {
		static const std::unordered_map<std::string, std::string> mimeTypes = {
			{".mp4", "video/mp4"},
			{".mov", "video/quicktime"},
			{".mkv", "video/x-matroska"},
			{".mp3", "audio/mpeg"},
			{".m4a", "audio/mp4"},
			{".mid", "audio/midi"},
			{".midi", "audio/midi"},
			{".flac", "audio/flac"},
			{".jpg", "image/jpeg"},
			{".jpeg", "image/jpeg"},
			{".png", "image/png"},
			{".bmp", "image/bmp"},
			{".html", "text/html"},
			{".htm", "text/html"},
			{".rtf", "application/rtf"},
			{".txt", "text/plain"}
		};

		size_t pos = filename.find_last_of('.');
		if (pos == std::string::npos) {
			return "application/octet-stream";
		}

		std::string extension = filename.substr(pos);
		for (char& c : extension) {
			c = static_cast<char>(tolower(c));
		}

		auto it = mimeTypes.find(extension);
		if (it != mimeTypes.end()) {
			return it->second;
		}

		return "application/octet-stream";
	}

	std::pair<std::string, int> resolveAddress(SOCKADDR* parsedAddress) {
		int TargetAddrLen = sizeof(parsedAddress);
		std::string address;
		int port;

		if (parsedAddress->sa_family == AF_INET) {
			char addrBuf[INET_ADDRSTRLEN];
			SOCKADDR_IN* v4 = (SOCKADDR_IN*)parsedAddress;
			inet_ntop(AF_INET, &v4->sin_addr, addrBuf, INET_ADDRSTRLEN * sizeof(char));
			port = ntohs(v4->sin_port);
			address = addrBuf;
			return std::pair<std::string, int>(address, port);
		}
		else if (parsedAddress->sa_family == AF_INET6) {
			char addrBuf[INET6_ADDRSTRLEN];
			SOCKADDR_IN6* v6 = (SOCKADDR_IN6*)parsedAddress;
			inet_ntop(AF_INET6, &v6->sin6_addr, addrBuf, INET6_ADDRSTRLEN * sizeof(char));
			port = ntohs(v6->sin6_port);
			address = addrBuf;
			return std::pair<std::string, int>(address, port);
		}
		else return std::pair<std::string, int>("FAULT", -1);
	}

	void log(const std::string& message, std::ostream& to) {
		std::time_t currentTime = std::time(nullptr);
		std::tm* localTime = std::localtime(&currentTime);

		char buffer[32];
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);

		to << buffer << "\t" << message << std::endl;
		std::cout << buffer << "\t" << message << std::endl;
	}
}