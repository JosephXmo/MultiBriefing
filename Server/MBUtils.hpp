#pragma once

#include "SystemDependency.hpp"
#include "MultiBriefingTypes.hpp"

struct SimpleAddress {
	char* address;
	short port;
};

void ReportClientCounter(int ClientCounter);

SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress);

long GenRandByTime();