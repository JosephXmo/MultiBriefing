#pragma once

#include "SystemDependency.hpp"
#include "ClientConfigs.hpp"

struct MBClient {
	long id;
	char* name;
	SOCKET socket;
};

struct MBMessage {
	enum MsgType { COMMAND, TEXT };
	MBClient sender;
	SYSTEMTIME timestamp;
	MsgType type;
	char* message;
};

struct SimpleAddress {
	char* address;
	short port;
};