#include "MultiBriefingClient.hpp"

int __cdecl MSClient(void)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    const char* sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

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
        //if (ConnectAddress.sa_family == AF_INET) {
        //    SOCKADDR_IN* v4 = (SOCKADDR_IN*)&ConnectAddress;
        //    char v4String[INET_ADDRSTRLEN];
        //    inet_ntop(AF_INET, &v4->sin_addr, v4String, sizeof(v4String));
        //    printf("Connected to client: %s:%s\n\n", v4String, DEFAULT_PORT);
        //}
        //else if (ConnectAddress.sa_family == AF_INET6) {
        //    SOCKADDR_IN6* v6 = (SOCKADDR_IN6*)&ConnectAddress;
        //    char v6String[INET6_ADDRSTRLEN];
        //    inet_ntop(AF_INET6, &v6->sin6_addr, v6String, sizeof(v6String));
        //    printf("Connected to client: %s:%s\n\n", v6String, DEFAULT_PORT);
        //}
        else printf("WARNING: Address version is neither v4 nor v6. Unidentified address family!\n");
    }

    // Send message buffer
    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Message Sent: %s (%ld Bytes)\n", sendbuf, iResult);
    while (1);
    //HANDLE hLogFile = CreateFile(
    //    "MBLog.log",
    //    GENERIC_READ | GENERIC_WRITE,
    //    FILE_SHARE_READ,
    //    NULL,
    //    CREATE_NEW,
    //    FILE_APPEND_DATA,
    //    NULL
    //);
    //WriteFile(hLogFile, NULL, NULL, nullptr, nullptr);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("%d Bytes: Server Received\n\n", iResult);
        }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while (iResult > 0);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}

SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress) {
    int TargetAddrLen = sizeof(ParsedAddress);
    SimpleAddress* RetAddress = (SimpleAddress*) malloc(sizeof(SimpleAddress));

    if (ParsedAddress->sa_family == AF_INET) {
        RetAddress->address = (char*)malloc(INET_ADDRSTRLEN * sizeof(char));
        SOCKADDR_IN* v4 = (SOCKADDR_IN*)ParsedAddress;
        inet_ntop(AF_INET, &v4->sin_addr, RetAddress->address, INET_ADDRSTRLEN * sizeof(char));
        RetAddress->port = ntohs(v4->sin_port);
        return RetAddress;
    }
    else if (ParsedAddress->sa_family == AF_INET6) {
        RetAddress->address = (char*)malloc(INET6_ADDRSTRLEN * sizeof(char));
        SOCKADDR_IN6* v6 = (SOCKADDR_IN6*)ParsedAddress;
        inet_ntop(AF_INET6, &v6->sin6_addr, RetAddress->address, INET6_ADDRSTRLEN * sizeof(char));
        RetAddress->port = ntohs(v6->sin6_port);
        return RetAddress;
    }
    else return nullptr;
}
