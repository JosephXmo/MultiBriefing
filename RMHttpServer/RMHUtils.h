#ifndef _RMHUTILS_H_
#define _RMHUTILS_H_

#pragma once

// C Standard Library
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>

// Windows API Header
#include <WinSock2.h>

// Data Structures of HTTP
#include "RMHData.h"

namespace ReturnMyHairHTTP {
	std::string resolveSafePath(const std::string& requestedPath);

	std::string resolveFileMIME(std::string filename);

	std::pair<std::string, int> resolveAddress(SOCKADDR* parsedAddress);

	void log(const std::string& message, std::ostream& to);
}

#endif // !_RMHUTILS_H_
