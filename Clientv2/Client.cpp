// Standard libraries
#include <iostream>
#include <string>

// Win32 API libraries
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

// Port to connect
#define DEFAULT_PORT 5019

struct Message
{
    char username[100];
    char text[512];
    int group_id;
};

struct ClientConfig
{
    char username[100];
    // char greeting[100];
    int group_id;
};

// Socket where connection establish to
SOCKET connect_sock;

// Transceiver threads
DWORD WINAPI receiveThread(LPVOID lpParam);
DWORD WINAPI sendThread(LPVOID lpParam);

int main(int argc, char** argv)
{
    WSADATA wsaData;
    struct sockaddr_in server_addr;
    struct hostent* hp;
    char server_name[] = "localhost";
    unsigned short port = DEFAULT_PORT;
    unsigned int addr;

    ClientConfig config;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
        return -1;
    }

    if (isalpha(server_name[0]))
    {
        hp = gethostbyname(server_name);
    }
    else
    {
        addr = inet_addr(server_name);
        hp = gethostbyaddr((char*)&addr, 4, AF_INET);
    }

    if (hp == NULL)
    {
        fprintf(stderr, "Cannot resolve address: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    // Configure socket
    memset(&server_addr, 0, sizeof(server_addr));
    memcpy(&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family = hp->h_addrtype;
    server_addr.sin_port = htons(port);

    connect_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_sock == INVALID_SOCKET)
    {
        fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    // Connect to server
    printf("Client connecting to: %s\n", hp->h_name);
    if (connect(connect_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "connect() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    // Fetch name
    printf("Enter your username: ");
    scanf("%99s", config.username);

    // Fetch target group
    config.group_id = 0;
    while (config.group_id <= 0)
    {
        printf("Enter your group_id(>=1): ");
        scanf("%d", &config.group_id);
    }
    // printf("Config you auto greeting when enter a group: ");
    // scanf("%99s", &config.greeting);

    // Create transceiver thread
    HANDLE hSendThread = CreateThread(NULL, 0, sendThread, &config, 0, NULL);
    HANDLE hReceiveThread = CreateThread(NULL, 0, receiveThread, NULL, 0, NULL);

    // Wait for the send thread to finish (or receive thread, depending on design)
    WaitForSingleObject(hSendThread, INFINITE);
    WaitForSingleObject(hReceiveThread, INFINITE);

    // Clean ups
    closesocket(connect_sock);
    WSACleanup();
    return 0;
}

DWORD WINAPI receiveThread(LPVOID lpParam)
{
    Message msg;
    int msg_len;
    while (true)
    {
        msg_len = recv(connect_sock, (char*)&msg, sizeof(msg), 0);

        // Socket error. Quit thread
        if (msg_len == SOCKET_ERROR)
        {
            fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
            return 1;
        }

        // Connection end
        if (msg_len == 0)
        {
            printf("Server closed connection\n");
            return 0;
        }

        // Regular message output
        if (msg.text[0] == '\0')continue;
        if ((int)msg.group_id == -1)
            printf("[System]%s", msg.text);
        else
            printf("%s : %s\n", msg.username, msg.text);
    }
}

DWORD WINAPI sendThread(LPVOID lpParam)
{
    ClientConfig* config = (ClientConfig*)lpParam;
    Message msg;
    strcpy(msg.username, config->username); // Copy the username into the struct
    msg.group_id = config->group_id;        // Set the group_id
    int msg_len;

    // Autolly call info command when user is the first time to join in the chatroom
    printf("[System]Hello %s! Welcome to our chatroom~\n", msg.username);
    strcpy(msg.text, "/info");
    send(connect_sock, (char*)&msg, sizeof(msg), 0);

    while (true)
    {
        fgets(msg.text, sizeof(msg.text), stdin);
        msg.text[strcspn(msg.text, "\n")] = 0; // Remove newline character

        if (msg.text[0] == '\0')continue;

        // Command handling
        // Quit
        if (strcmp(msg.text, "/q") == 0)
        {
            printf("[System]Exiting...\n");
            closesocket(connect_sock);
            WSACleanup();
            return 0;
        }

        // Change group
        if (strncmp(msg.text, "/change group ", 14) == 0)
        {
            int new_group_id = atoi(msg.text + 14);
            if (new_group_id >= 1)
                printf("[System]Group changed to %d\n", new_group_id);
            else
                printf("[System]Invalid group ID\n");
            msg.group_id = new_group_id; // Change group ID
            strcpy(msg.text, "\0");
        }

        // Not a command. Send message
        msg_len = send(connect_sock, (char*)&msg, sizeof(msg), 0);
        if (msg_len == SOCKET_ERROR)
        {
            fprintf(stderr, "send() failed with error %d\n", WSAGetLastError());
            return 1;
        }
    }
}