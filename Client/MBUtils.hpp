#pragma once

#include "SystemDependency.hpp"
#include "MultiBriefingTypes.hpp"

SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress);

MBMessage ResolveMsg(MBClient sender, SYSTEMTIME time, char* msg);

int Log(char* msg);