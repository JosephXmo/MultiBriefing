#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>
#include <queue>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_LEN 1024
#define NAME_LEN 20
#define MAX_CLIENT_NUM 32

struct Client
{
    int valid;               // to judge whether this user is online
    int fd_id;               // user ID number
    SOCKET socket;           // socket to this user
    char name[NAME_LEN + 1]; // name of the user
    HANDLE recv_thread;      // thread for receiving messages
    HANDLE send_thread;      // thread for sending messages
} client[MAX_CLIENT_NUM] = { 0 };

queue<string> message_q; // message buffer

// the full number of clients exist in the chatroom
int current_client_num = 0;
// sync current_client_num
HANDLE num_mutex;

// Mutex for broadcasting messages to all clients
HANDLE broadcast_mutex;

// send message
unsigned int __stdcall handle_send(void* data)
{
    while (1)
    {
        // Wait for a message to be available
        WaitForSingleObject(broadcast_mutex, INFINITE);
        // Broadcast message to all clients
        while (!message_q.empty())
        {
            string message_buffer = message_q.front();
            for (int i = 0; i < MAX_CLIENT_NUM; i++)
            {
                if (client[i].valid)
                {
                    int len = send(client[i].socket, message_buffer.c_str(), message_buffer.length(), 0);
                    if (len < 0)
                    {
                        perror("send");
                    }
                }
            }
            message_q.pop();
        }
        // Release the mutex
        ReleaseMutex(broadcast_mutex);
        // Sleep for a while to avoid busy-waiting
        Sleep(100);
    }
    return 0;
}

// receive message
unsigned int __stdcall handle_recv(void* data)
{
    struct Client* pipe = (struct Client*)data;

    // message buffer
    char buffer[BUFFER_LEN + 1];
    int buffer_len = 0;

    // receive
    while ((buffer_len = recv(pipe->socket, buffer, BUFFER_LEN, 0)) > 0)
    {
        buffer[buffer_len] = '\0'; // Null-terminate the received data
        // Create a message with the sender's name
        string message_buffer = pipe->name;
        message_buffer += ": ";
        message_buffer += buffer;

        // Lock the mutex before adding message to the queue
        WaitForSingleObject(broadcast_mutex, INFINITE);
        // Add message to the queue
        message_q.push(message_buffer);
        // Release the mutex
        ReleaseMutex(broadcast_mutex);
    }

    // If recv returns 0 or SOCKET_ERROR, it indicates that the client has been disconnected or there was an error
    if (buffer_len == 0)
    {
        printf("%s left the chatroom.\n", pipe->name);
    }
    else if (buffer_len == SOCKET_ERROR)
    {
        printf("%s left the chatroom.\n", pipe->name);
    }

    // Close the client socket and thread
    pipe->valid = 0;
    current_client_num--;
    shutdown(pipe->socket, 2);
    CloseHandle(pipe->recv_thread);
    CloseHandle(pipe->send_thread);

    return 0;
}

// deal with each client
unsigned int __stdcall chat(void* data)
{
    struct Client* pipe = (struct Client*)data;

    // Send welcome message to the client
    char welcome_msg[100];
    sprintf(welcome_msg, "Hello %s, Welcome to the chatroom. We have %d people in total.\n", pipe->name, current_client_num);
    send(pipe->socket, welcome_msg, strlen(welcome_msg), 0);

    // Create a new thread to handle sending messages to the client
    pipe->send_thread = (HANDLE)_beginthreadex(NULL, 0, handle_send, NULL, 0, NULL);

    // Receive messages from the client
    handle_recv(data);

    return 0;
}

int main()
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    // Create server socket
    SOCKET server_sock;
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        perror("socket");
        return 1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    // Get server port and bind
    int server_port = 0;
    while (1)
    {
        printf("Please enter the port number of the server: ");
        scanf("%d", &server_port);

        addr.sin_port = htons(server_port);
        if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            perror("bind");
            continue;
        }
        break;
    }

    // Limit the number of clients
    if (listen(server_sock, MAX_CLIENT_NUM + 1) == SOCKET_ERROR)
    {
        perror("listen");
        return -1;
    }
    printf("Server start successfully!\n");
    printf("You can join the chatroom by connecting to 127.0.0.1:%d\n\n", server_port);

    // Initialize mutexes
    num_mutex = CreateMutex(NULL, FALSE, NULL);
    broadcast_mutex = CreateMutex(NULL, FALSE, NULL);

    // Waiting for clients to connect
    while (1)
    {
        // Create a new connection
        SOCKET client_sock = accept(server_sock, NULL, NULL);
        if (client_sock == INVALID_SOCKET)
        {
            perror("accept");
            return -1;
        }

        // Check if the chatroom is full
        if (current_client_num >= MAX_CLIENT_NUM)
        {
            if (send(client_sock, "ERROR", strlen("ERROR"), 0) < 0)
                perror("send");
            shutdown(client_sock, 2);
            continue;
        }
        else
        {
            if (send(client_sock, "OK", strlen("OK"), 0) < 0)
                perror("send");
        }

        // Get client's name
        char name[NAME_LEN + 1] = { 0 };
        int state = recv(client_sock, name, NAME_LEN, 0);
        if (state < 0)
        {
            perror("recv");
            shutdown(client_sock, 2);
            continue;
        }
        // If the client does not input a name and leaves directly
        else if (state == 0)
        {
            shutdown(client_sock, 2);
            continue;
        }

        // Update client array and create a new thread for the client
        for (int i = 0; i < MAX_CLIENT_NUM; i++)
        {
            // Find the first unused client
            if (!client[i].valid)
            {
                WaitForSingleObject(num_mutex, INFINITE);
                // Set name
                memset(client[i].name, 0, sizeof(client[i].name));
                strcpy_s(client[i].name, sizeof(client[i].name), name);
                // Set other info
                client[i].valid = 1;
                client[i].fd_id = i;
                client[i].socket = client_sock;

                current_client_num++;
                ReleaseMutex(num_mutex);

                // Create a new receive thread for the client
                client[i].recv_thread = (HANDLE)_beginthreadex(NULL, 0, chat, (void*)&client[i], 0, NULL);
                printf("%s joined the chatroom. %d people in total.\n", client[i].name, current_client_num);

                break;
            }
        }
    }

    // Close sockets and cleanup
    for (int i = 0; i < MAX_CLIENT_NUM; i++)
    {
        if (client[i].valid)
            shutdown(client[i].socket, 2);
        CloseHandle(client[i].recv_thread);
        CloseHandle(client[i].send_thread);
    }
    shutdown(server_sock, 2);
    WSACleanup();
    return 0;
}
