#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_SERV "localhost"
#define DEFAULT_PORT "27015"

struct MBMessage {
	char* ipAddress;
	char* username;
	SYSTEMTIME timestamp;
	char* message;
};

struct SimpleAddress {
	char* address;
	short port;
};

int __cdecl MSClient(void);
SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress);
