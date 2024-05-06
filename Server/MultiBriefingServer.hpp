#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define MAX_CONN 10

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int ClientCounter = 0;
SOCKET ClientSockets[MAX_CONN];

struct MBMessage {
	MBClient client;
	SYSTEMTIME timestamp;
	char* message;
};

struct MBClient {
	char* nickname;
	SOCKET Socket;
};

struct SimpleAddress {
    char* address;
    short port;
};

// Application Entrance
int __cdecl main(void);

// Functional process
int FullConnectReject(SOCKET ClientCSocket);
void communication(SOCKET ClientSocket);

// Utilities
SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress);