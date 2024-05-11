#include "MBUtils.hpp"

void ReportClientCounter(int ClientCounter) {
    printf("\tTotal connections: %d\n\n", ClientCounter);
}

void Announcement(MBClientsRegTable* ClientsTable, char* Msg) {
    int iSendResult;

    // Echo the buffer to all connected clients
    int ForwardCounter = 0;
    for (int i = 0; i < MAX_CONN; i++) {
        if (ClientsTable->table[i] == nullptr) continue;

        char sendbuf[DEFAULT_BUFLEN];
        ZeroMemory(sendbuf, DEFAULT_BUFLEN * sizeof(char));
        sprintf_s(
            sendbuf, DEFAULT_BUFLEN,
            "!Announcement!\n%s",
            Msg
        );

        iSendResult = send(ClientsTable->table[i]->socket, sendbuf, (int)strlen(sendbuf), 0);
        if (iSendResult == SOCKET_ERROR) printf("send failed with error: %d\n", WSAGetLastError());
        else ForwardCounter++;
    }
    printf("Echoed to %d clients.\n\n", ForwardCounter);
}

SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress) {
    int TargetAddrLen = sizeof(ParsedAddress);
    SimpleAddress* RetAddress = (SimpleAddress*)malloc(sizeof(SimpleAddress));
    RetAddress->address = nullptr;
    RetAddress->port = -1;

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

long GenRandByTime() {
    srand(time(NULL));

    int high = rand(), low = rand();
    long ret;

    memcpy(&ret, &low, sizeof(low));
    memcpy(&ret + sizeof(low), &high, sizeof(high));

    return ret;
}