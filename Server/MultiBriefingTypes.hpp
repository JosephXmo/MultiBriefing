#pragma once

#include "SystemDependency.hpp"
#include "ServerConfigs.hpp"

struct MBClient {
	long id;
	char* name;
	SOCKET socket;
};

struct MBMessage {
	enum MsgType {COMMAND, TEXT};
	MBClient sender;
	SYSTEMTIME timestamp;
	MsgType type;
	char* message;
};

struct MBClientsRegTable {
	int counter = 0;
	MBClient* table[MAX_CONN] = { nullptr };
};

struct SimpleAddress {
	char* address;
	short port;
};