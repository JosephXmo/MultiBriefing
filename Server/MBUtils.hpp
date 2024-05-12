#pragma once

#include "SystemDependency.hpp"
#include "MultiBriefingTypes.hpp"

char* FetchName(SOCKET ClientSocket);

int RegisterClient(MBClientsRegTable* ClientsTable, MBClient* newClient);
int DeregisterClient(MBClientsRegTable* ClientsTable, MBClient* targetClient);

void ReportClientCounter(int ClientCounter);

void Announcement(MBClientsRegTable* ClientsTable, char* Msg);

SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress);

long GenRandByTime();