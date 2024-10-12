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

#include "MBUtils.hpp"

char* FetchName(SOCKET ClientSocket) {
    int iResult;
    char* namebuf = (char*)malloc((MAX_NAME + 1) * sizeof(char));
    memset(namebuf, '\0', sizeof(namebuf));

    iResult = recv(ClientSocket, namebuf, MAX_NAME, 0);
    if (iResult > 0) {
        // Echo name to client
        int iSendResult = send(ClientSocket, namebuf, iResult, 0);
        if (iSendResult == SOCKET_ERROR) printf("send failed with error: %d\n", WSAGetLastError());
    }
    else {
        int WSAErrorCode = WSAGetLastError();
        if (WSAErrorCode == 10054)
            printf("Client forced quitted.\n");
        else
            printf("Name recv failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);

        strcpy_s(namebuf, 8 * sizeof(char), "UNKNOWN");
    }

    return namebuf;
}

int RegisterClient(MBClientsRegTable* ClientsTable, MBClient* newClient) {
    for (int i = 0; i < MAX_CONN; i++) {
        if (ClientsTable->table[i] == nullptr) {
            ClientsTable->table[i] = newClient;
            ClientsTable->counter++;

            return ClientsTable->counter;
        }
    }

    return -1;
}

int DeregisterClient(MBClientsRegTable* ClientsTable, MBClient* targetClient) {
    for (int i = 0; i < MAX_CONN; i++) {
        if (ClientsTable->table[i] == targetClient) {
            // Pop-off from register table
            ClientsTable->table[i] = nullptr;
            ClientsTable->counter--;

            // Release
            free(targetClient->name);
            free(targetClient);

            return ClientsTable->counter;
        }
    }

    return -1;
}

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