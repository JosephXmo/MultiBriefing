#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <iostream>
#include <string>
#include <limits>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

#define BUFFER_LEN 1024
#define NAME_LEN 20

char name[NAME_LEN + 1]; // client's name

// Mutex for controlling send operation
HANDLE send_mutex;

// receive message and print out
unsigned __stdcall handle_recv(void* data)
{
    SOCKET client_sock = *((SOCKET*)data);

    // message buffer
    char buffer[BUFFER_LEN + 1];
    int buffer_len = 0;

    // receive
    while ((buffer_len = recv(client_sock, buffer, BUFFER_LEN, 0)) > 0)
    {
        buffer[buffer_len] = '\0'; // Null-terminate the received data
        cout << buffer << endl;    // Print out the received message
    }

    // If recv returns 0 or SOCKET_ERROR, it indicates that the server has been shut down or there was an error
    if (buffer_len == 0)
    {
        printf("The Server has been shutdown!\n");
    }
    else if (buffer_len == SOCKET_ERROR)
    {
        perror("recv");
    }

    return 0;
}

// Send message
void send_message(SOCKET client_sock)
{
    // Lock the mutex before sending
    WaitForSingleObject(send_mutex, INFINITE);

    // Get message from user
    char message[BUFFER_LEN + 1];
    cin.getline(message, BUFFER_LEN);

    // Send message to server
    if (send(client_sock, message, strlen(message), 0) < 0)
    {
        perror("send");
    }

    // Unlock the mutex after sending
    ReleaseMutex(send_mutex);
}

int main()
{
    WSADATA wsaData;
    SOCKET client_sock;

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }

    // Get IP address and port of the server and connect
    int server_port = 0;
    char server_ip[16] = { 0 };

    printf("Please enter IP address of the server: ");
    scanf("%s", server_ip);
    printf("Please enter port number of the server: ");
    scanf("%d", &server_port);
    getchar(); // Read useless '\n'

    // Resolve the server address and port
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char server_port_str[5];
    _itoa(server_port, server_port_str, 10);

    iResult = getaddrinfo(server_ip, server_port_str, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // Create a socket to connect with the server
        client_sock = socket(
            ptr->ai_family,
            ptr->ai_socktype,
            ptr->ai_protocol
        );

        if (client_sock == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to the server
        if (connect(client_sock, ptr->ai_addr, ptr->ai_addrlen) == SOCKET_ERROR)
        {
            perror("connect");
            continue;
        }
        break;
    }

    // Check state
    printf("Connecting......");
    fflush(stdout);
    char state[10] = { 0 };
    if (recv(client_sock, state, sizeof(state), 0) < 0)
    {
        perror("recv");
        return -1;
    }
    if (strcmp(state, "OK"))
    {
        printf("\rThe chatroom is already full!\n");
        return 0;
    }
    else
    {
        printf("\rConnect Successfully!\n");
    }

    // Get client name
    printf("Welcome to Use Multi-Person Chat room!\n");
    while (1)
    {
        printf("Please enter your name: ");
        cin.get(name, NAME_LEN);
        int name_len = strlen(name);
        // No input
        if (cin.eof())
        {
            // Reset
            cin.clear();
            clearerr(stdin);
            printf("\nYou need to input at least one word!\n");
            continue;
        }
        // Single enter
        else if (name_len == 0)
        {
            // Reset
            cin.clear();
            clearerr(stdin);
            cin.get();
            continue;
            printf("\nYou need to input at least one word!\n");
        }
        // Overflow
        if (name_len > NAME_LEN - 2)
        {
            // Reset
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            printf("\nReached the upper limit of the words!\n");
            continue;
        }
        cin.get(); // Remove '\n' in stdin
        name[name_len] = '\0';
        break;
    }
    if (send(client_sock, name, strlen(name), 0) < 0)
    {
        perror("send");
        return -1;
    }

    // Create mutex for send operation
    send_mutex = CreateMutex(NULL, FALSE, NULL);

    // Create a new thread to handle receive message
    HANDLE recv_thread;
    unsigned threadID;
    recv_thread = (HANDLE)_beginthreadex(NULL, 0, handle_recv, &client_sock, 0, &threadID);

    // Get message and send
    while (1)
    {
        send_message(client_sock);
    }

    // Close thread and socket
    CloseHandle(recv_thread);
    shutdown(client_sock, 2);
    WSACleanup();
    return 0;
}
