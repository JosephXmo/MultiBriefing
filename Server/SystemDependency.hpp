#undef UNICODE

#define WIN32_LEAN_AND_MEAN

// Win32 API
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// C standards
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")