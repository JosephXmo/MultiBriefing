#include "MultiBriefingServer.hpp"

int __cdecl MSServer(void)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    printf("-------- MultiBriefing Server Initialization Success --------\n");
    printf("Waiting for client connection...\n\n");

    do {
        // Server reaches maximum connection limit
        if (ClientCounter == MAX_CONN) {
            CreateThread(
                NULL, NULL,
                (LPTHREAD_START_ROUTINE)FullConnectReject,
                (LPVOID)accept(ListenSocket, NULL, NULL),
                NULL, NULL
            );
            continue;
        }

        // Server has spare resource for new client connection
        // Accept a client socket
        SOCKADDR ClientAddress;
        ClientSocket = accept(ListenSocket, &ClientAddress, NULL);
        if (ClientSocket == SOCKET_ERROR) {     // INVALID_SOCKET
            // printf("accept failed with error: %d\n", WSAGetLastError());
            // closesocket(ListenSocket);
            // WSACleanup();
            // return 1;
            continue;
        }
        ClientSockets[ClientCounter++] = ClientSocket;

        // Create MBClient Object
        SYSTEMTIME CurrentTime; GetSystemTime(&CurrentTime);
        MBClient Client{ GenRandByTime(), Client.socket = ClientSocket };

        // Report client connection & information
        SOCKADDR TargetAddress;
        int TargetAddrLen = sizeof(TargetAddress);
        if (getpeername(ClientSocket, &TargetAddress, &TargetAddrLen) == 0) {
            SimpleAddress* saTargetAddress = ResolveAddress(&TargetAddress);

            if (saTargetAddress != nullptr) {
                printf("Client connection from: %s\n", saTargetAddress->address);
                ReportClientCounter(ClientCounter);
            }
            else printf("WARNING: Address version is neither v4 nor v6. Unidentified address family!\n");
        }

        // Create client handler thread
        CreateThread(
            NULL, NULL,
            (LPTHREAD_START_ROUTINE)communication,
            (LPVOID)&Client,
            NULL, NULL
        );
        printf("Waiting for client connection...\n\n");
    } while (1);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);

    // Total cleanup
    // Attention that when performing a multi-client service, server needs to listen continuously.
    // Therefore the listening socket must be closed when server shut down.
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}

int FullConnectReject(SOCKET ClientSocket) {
    const char* FailMsg ="Server reached maximum connection limit. Try again later." ;
    int iSendResult = send(ClientSocket, FailMsg, sizeof(FailMsg), 0);

    if (iSendResult == SOCKET_ERROR) {
        printf("Full Connect Rejection failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        return -1;
    }

    return iSendResult;
}

void communication(MBClient* Client) {
    int iResult, iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection
    do {
        ZeroMemory(recvbuf, sizeof(recvbuf));
        iResult = recv(Client->socket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("%ld: %s (%d Bytes)\n", Client->id, recvbuf, iResult);

            // Echo the buffer back to the sender
            for (int i = 0; i < ClientCounter; i++) {
                iSendResult = send(ClientSockets[i], recvbuf, iResult, 0);
                if (iSendResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(Client->socket);
                }
            }
            printf("Echoed %d Bytes to %d clients.\n\n", iSendResult, ClientCounter);
        }
        else if (iResult == 0) {
            printf("Connection closing...\n");
            ClientCounter--;
            ReportClientCounter(ClientCounter);
        }
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(Client->socket);
            // TODO: Complete client list pop-off mechanism
            ReportClientCounter(ClientCounter);
        }
    } while (iResult > 0);
}
