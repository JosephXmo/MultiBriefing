#pragma once

#include "SystemDependency.hpp"

struct MBClient {
	long id;
	SOCKET socket;
};

struct MBMessage {
	MBClient sender;
	SYSTEMTIME timestamp;
	char* message;
};