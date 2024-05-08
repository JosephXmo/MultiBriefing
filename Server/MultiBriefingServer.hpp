// Include system library needed
#include "SystemDependency.hpp"

// Include common types
#include "MultiBriefingTypes.hpp"

// Save config macro definitions in another file for simpler management
#include "ServerConfigs.hpp"

// Include utilities
#include "SocketLinkedList.hpp"
#include "MBUtils.hpp"
#define FILETIME_TO_UNIXTIME(ft) (UINT)((*(LONGLONG*)&(ft)-116444736000000000)/10000000) 

// Public variables
int ClientCounter = 0;
SOCKET ClientSockets[MAX_CONN];

// Application Entrance
int __cdecl main(void);

// Functional process
int FullConnectReject(SOCKET ClientCSocket);
void communication(MBClient* Client);
