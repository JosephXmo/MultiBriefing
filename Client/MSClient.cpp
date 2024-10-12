/*
     MultiBriefing Communicator - An experimental program implementing chat over IP, and more.
     Copyright (C) 2024  Sibo Qiu, Runjie Miao, Yucheng Tong, and Menghan Wang

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <https://www.gnu.org/licenses/>.

     Please contact dev team via qhdqsb@hotmail.com if you need to.
 */

#include "MultiBriefingClient.hpp"

char name[MAX_NAME + 1];
HANDLE ConsoleD;

int __cdecl MSClient(void)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    HANDLE ConsoleD = CreateMutex(NULL, FALSE, "Console Print Daemon");

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(DEFAULT_SERV, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    printf("Connecting to server...\n");
    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(
            ptr->ai_family, 
            ptr->ai_socktype,
            ptr->ai_protocol
        );
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    SOCKADDR ConnectAddress;
    int TargetAddrLen = sizeof(ConnectAddress);
    if (getpeername(ConnectSocket, &ConnectAddress, &TargetAddrLen) == 0) {
        SimpleAddress* saConnectAddress = ResolveAddress(&ConnectAddress);
        if (saConnectAddress != nullptr) {
            printf("Connected to server: %s\n\n", saConnectAddress->address);
        }
        else printf("WARNING: Address version is neither v4 nor v6. Unidentified address family!\n");
    }

    ZeroMemory(name, (MAX_NAME + 1) * sizeof(char));
    std::cout << "Name: ";
    std::cin >> name;
    getc(stdin);
    iResult = send(ConnectSocket, name, (int)strlen(name), 0);
    if (iResult == SOCKET_ERROR) printf("send failed with error: %d\n", WSAGetLastError());
    recv(ConnectSocket, name, MAX_NAME, 0);
    std::cout << "Your name is: " << name << std::endl << std::endl;
    fflush(stdin);
    fflush(stdout);

    HANDLE Transceiver[2];

    Transceiver[0] = CreateThread(
        NULL, NULL,
        (LPTHREAD_START_ROUTINE)Sender,
        (LPVOID)&ConnectSocket,
        NULL, NULL
    );
    Transceiver[1] = CreateThread(
        NULL, NULL,
        (LPTHREAD_START_ROUTINE)Receiver,
        (LPVOID)&ConnectSocket,
        NULL, NULL
    );

    WaitForMultipleObjects(2, Transceiver, TRUE, INFINITE);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    CloseHandle(ConsoleD);
    CloseHandle(Transceiver[0]);
    CloseHandle(Transceiver[1]);

    return 0;
}

void Sender(SOCKET* ArgSocket) {
    int iResult;
    char sendbuf[DEFAULT_BUFLEN];
    bool finFlag = 1;
    SOCKET ConnectSocket = *ArgSocket;

    while (finFlag) {
        std::cout << name << "> ";
        gets_s(sendbuf);

        finFlag = strcmp(sendbuf, "/quit");
        if (!finFlag) break;

        // Send message buffer
        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            WaitForSingleObject(ConsoleD, INFINITE);
            printf("send failed with error: %d\n", WSAGetLastError());
            ReleaseMutex(ConsoleD);
        }

        // WaitForSingleObject(ConsoleD, INFINITE);
        // printf("Message Sent: %s (%ld Bytes)\n", sendbuf, iResult);
        // Log((char*) sendbuf);
        // ReleaseMutex(ConsoleD);

        ZeroMemory(sendbuf, sizeof(sendbuf));
    }

    // shutdown the connection to prepare for program termination
    iResult = shutdown(ConnectSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
        WaitForSingleObject(ConsoleD, INFINITE);
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        ReleaseMutex(ConsoleD);
    }
}

void Receiver(SOCKET* ArgSocket) {
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    SOCKET ConnectSocket = *ArgSocket;
    int recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer closes the connection
    ZeroMemory(recvbuf, sizeof(recvbuf));
    while ((iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0)) > 0) {
        if (iResult > 0) {
            WaitForSingleObject(ConsoleD, INFINITE);
            printf("\n\n%s\n\n", recvbuf);
            printf("%s> ", name);
            ReleaseMutex(ConsoleD);
        }
        ZeroMemory(recvbuf, sizeof(recvbuf));
    }

    if (iResult < 0) {
        WaitForSingleObject(ConsoleD, INFINITE);
        printf("\trecv failed with error: %d\n", WSAGetLastError());
        ReleaseMutex(ConsoleD);
    }

    WaitForSingleObject(ConsoleD, INFINITE);
    printf("Connection closed\n");
    ReleaseMutex(ConsoleD);
}