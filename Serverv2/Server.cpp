// Standard libraries
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

// Win32 API libraries
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

// Port where server listen to
#define DEFAULT_PORT 5019

struct Message
{
    char username[100];
    char text[512];
    int group_id;
};

// A vector saving all connected clients' SOCKET object
std::vector<SOCKET> clients;
// A mapping from client's SOCKET to their groupID number
std::unordered_map<SOCKET, int> clientGroups;

// Critical section marker to make sure operations won't interrupt each other when handling clients in different threads.
CRITICAL_SECTION clientListCriticalSection;

// Broadcast message to clients in same group
void BroadcastMessage(const Message& msg, SOCKET senderSock);

// Client handler. Each client has one unique dedicated handler thread.
DWORD WINAPI handleClient(LPVOID clientSocket);

int main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET sock, client_sock;
    struct sockaddr_in local, client_addr;
    int addr_len;

    // Winsock Initialization
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
        return -1;
    }

    InitializeCriticalSection(&clientListCriticalSection);

    // Configure address object
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(DEFAULT_PORT);

    // Establish port listening
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
    {
        fprintf(stderr, "bind() failed with error %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    if (listen(sock, 5) == SOCKET_ERROR)
    {
        fprintf(stderr, "listen() failed with error %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    printf("Waiting for connections ........\n");

    while (true)
    {
        addr_len = sizeof(client_addr);
        client_sock = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock == INVALID_SOCKET)
        {
            fprintf(stderr, "accept() failed with error %d\n", WSAGetLastError());
            continue;
        }

        printf("Accepted connection from %s, port %d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
        EnterCriticalSection(&clientListCriticalSection);
        clients.push_back(client_sock);
        LeaveCriticalSection(&clientListCriticalSection);

        CreateThread(NULL, 0, handleClient, (LPVOID)client_sock, 0, NULL);
    }

    DeleteCriticalSection(&clientListCriticalSection);
    WSACleanup();
    return 0;
}

void BroadcastMessage(const Message& msg, SOCKET senderSock)
{
    EnterCriticalSection(&clientListCriticalSection);
    int senderGroupId = clientGroups[senderSock];
    for (SOCKET sock : clients)
    {
        if (sock != senderSock && clientGroups[sock] == senderGroupId)
        {
            send(sock, (char*)&msg, sizeof(msg), 0);
        }
    }
    LeaveCriticalSection(&clientListCriticalSection);
}

DWORD WINAPI handleClient(LPVOID clientSocket)
{
    SOCKET clientSock = (SOCKET)clientSocket;
    Message msg;
    int msg_len;

    while (1)
    {
        msg_len = recv(clientSock, (char*)&msg, sizeof(msg), 0);

        // Client forced quit
        if (msg_len == SOCKET_ERROR)
        {
            printf("Client %s closed connection and leave Group %d\n\n", msg.username, msg.group_id);
            closesocket(clientSock);
            break;
        }

        // Client normal quit
        if (msg_len == 0)
        {
            printf("Client closed connection\n");
            closesocket(clientSock);
            break;
        }

        // Update client's group ID
        if (msg.group_id >= 1 && (clientGroups.count(clientSock) == 0 || clientGroups[clientSock] != msg.group_id)) {
            clientGroups[clientSock] = msg.group_id;
            printf("Client %s changed group to %d\n\n", msg.username, msg.group_id);
            std::string name(msg.username);
            std::string infoMessage = "User " + name + " joined the group. Now we have " + std::to_string(clientGroups.size()) + " people \n";
            struct Message sy_msg;
            strcpy_s(sy_msg.username, "Server");
            strcpy_s(sy_msg.text, infoMessage.c_str());
            sy_msg.group_id = -1;
            BroadcastMessage(sy_msg, clientSock);
        }

        // Command reply
        // Group info
        if (strcmp(msg.text, "/ls") == 0)
        {
            std::unordered_map<int, int> groupCounts;
            for (const auto& pair : clientGroups)
            {
                groupCounts[pair.second]++;
            }
            std::string groupList = "Active groups:\n";
            for (const auto& group : groupCounts)
            {
                groupList += "Group ID " + std::to_string(group.first) + ": " + std::to_string(group.second) + " members\n";
            }
            groupList += "\n";
            struct Message msg;
            strcpy_s(msg.username, "Server");
            strcpy_s(msg.text, groupList.c_str());
            msg.group_id = -1;
            send(clientSock, (char*)&msg, sizeof(msg), 0);
            continue;
        }
        // Command list
        if (strcmp(msg.text, "/info") == 0)
        {
            std::string infoMessage = "Now there are " + std::to_string(clients.size()) +
                " people online.\n" +
                "1: You can use enter to send your message to the group. \n" +
                "2: You can use /ls to check the active in chat groups. \n" +
                "3: You can use /change group (number) to change the chat groups. \n" +
                "4: You can use /q to exit the chat \n\n";
            struct Message msg;
            strcpy_s(msg.username, "Server");
            strcpy_s(msg.text, infoMessage.c_str());
            msg.group_id = -1;
            send(clientSock, (char*)&msg, sizeof(msg), 0);
            continue;
        }

        BroadcastMessage(msg, clientSock);
    }

    EnterCriticalSection(&clientListCriticalSection);
    for (auto it = clients.begin(); it != clients.end();)
    {
        if (*it == clientSock)
        {
            it = clients.erase(it);
        }
        else
        {
            ++it;
        }
    }
    clientGroups.erase(clientSock);
    LeaveCriticalSection(&clientListCriticalSection);
    return 0;
}