#pragma once

#include "SystemDependency.hpp"
#include "MultiBriefingTypes.hpp"

void ReportClientCounter(int ClientCounter);

void Announcement(MBClientsRegTable* ClientsTable, char* Msg);

SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress);

long GenRandByTime();