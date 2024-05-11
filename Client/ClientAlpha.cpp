#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <limits>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
using namespace std;

#define BUFFER_LEN 1024
#define MAX_NAME 20

char name[MAX_NAME + 1]; // client's name

// receive message and print out
DWORD WINAPI handle_recv(LPVOID data)
{
    SOCKET pipe = *(SOCKET*)data;

    // message buffer
    string message_buffer;
    int message_len = 0;

    // one transfer buffer
    char buffer[BUFFER_LEN + 1];
    int buffer_len = 0;

    // receive
    while ((buffer_len = recv(pipe, buffer, BUFFER_LEN, 0)) > 0)
    {
        // to find '\n' as the end of the message
        for (int i = 0; i < buffer_len; i++)
        {
            if (message_len == 0)
                message_buffer = buffer[i];
            else
                message_buffer += buffer[i];

            message_len++;

            if (buffer[i] == '\n')
            {
                // print out the message
                cout << message_buffer << endl;

                // new message start
                message_len = 0;
                message_buffer.clear();
            }
        }
        memset(buffer, 0, sizeof(buffer));
    }
    // because the recv() function is blocking, so when the while() loop break, it means the server is offline
    printf("The Server has been shutdown!\n");
    return 0;
}

int ClientAlpha()
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // create a socket to connect with the server
    SOCKET client_sock;
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        perror("socket");
        WSACleanup();
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;

    // get IP address and port of the server and connect
    int server_port = 0;
    char server_ip[16] = { 0 };
    while (1)
    {
        cout << "Please enter IP address of the server: ";
        cin >> server_ip;
        cout << "Please enter port number of the server: ";
        cin >> server_port;
        fflush(stdin);
        // getchar(); // read useless '\n'

        addr.sin_port = htons(server_port);
        inet_pton(AF_INET, server_ip, &addr.sin_addr.s_addr);
        // connect the server
        if (connect(client_sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            perror("connect");
            continue;
        }
        break;
    }

    // check state
    printf("Connecting......");
    //fflush(stdout);
    //char state[10] = { 0 };
    //if (recv(client_sock, state, sizeof(state), 0) < 0)
    //{
    //    perror("recv");
    //    closesocket(client_sock);
    //    WSACleanup();
    //    return -1;
    //}
    //if (strcmp(state, "OK"))
    //{
    //    printf("\rThe chatroom is already full!\n");
    //    closesocket(client_sock);
    //    WSACleanup();
    //    return 0;
    //}
    //else
    //{
    //    printf("\rConnect Successfully!\n");
    //}

    //////////////// get client name ////////////////
    printf("Welcome to Use Multi-Person Chat room!\n");
    while (1)
    {
        printf("Please enter your name: ");
        cin.get(name, MAX_NAME);
        int name_len = strlen(name);
        // no input
        if (cin.eof())
        {
            // reset
            cin.clear();
            clearerr(stdin);
            printf("\nYou need to input at least one word!\n");
            continue;
        }
        // sigle enter
        else if (name_len == 0)
        {
            // reset
            cin.clear();
            clearerr(stdin);
            cin.get();
            continue;
            printf("\nYou need to input at least one word!\n");
        }
        // overflow
        if (name_len > MAX_NAME - 2)
        {
            // reset
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            printf("\nReached the upper limit of the words!\n");
            continue;
        }
        cin.get(); // remove '\n' in stdin
        name[name_len] = '\0';
        break;
    }
    if (send(client_sock, name, strlen(name), 0) < 0)
    {
        perror("send");
        closesocket(client_sock);
        WSACleanup();
        return -1;
    }
    //////////////// get client name ////////////////

    // create a new thread to handle receive message
    HANDLE recv_thread;
    DWORD recv_thread_id;
    recv_thread = CreateThread(NULL, 0, handle_recv, (LPVOID)&client_sock, 0, &recv_thread_id);
    if (recv_thread == NULL)
    {
        printf("Failed to create receive thread\n");
        closesocket(client_sock);
        WSACleanup();
        return -1;
    }

    // get message and send
    while (1)
    {
        char message[BUFFER_LEN + 1];
        cin.get(message, BUFFER_LEN);
        int n = strlen(message);
        if (cin.eof())
        {
            // reset
            cin.clear();
            clearerr(stdin);
            continue;
        }
        // single enter
        else if (n == 0)
        {
            // reset
            cin.clear();
            clearerr(stdin);
        }
        // overflow
        if (n > BUFFER_LEN - 2)
        {
            // reset
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            printf("Reached the upper limit of the words!\n");
            continue;
        }
        cin.get();         // remove '\n' in stdin
        message[n] = '\n'; // add '\n'
        message[n + 1] = '\0';
        // the length of message now is n+1
        n++;
        printf("\n");
        // the length of message that has been sent
        int sent_len = 0;
        // calculate one transfer length
        int trans_len = BUFFER_LEN > n ? n : BUFFER_LEN;

        // send message
        while (n > 0)
        {
            int len = send(client_sock, message + sent_len, trans_len, 0);
            if (len < 0)
            {
                perror("send");
                closesocket(client_sock);
                WSACleanup();
                return -1;
            }
            // if one transfer has not sent the full message, then send the remain message
            n -= len;
            sent_len += len;
            trans_len = BUFFER_LEN > n ? n : BUFFER_LEN;
        }
        // clean the buffer
        memset(message, 0, sizeof(message));
    }

    // close the receive thread
    TerminateThread(recv_thread, 0);
    // shutdown the connection
    shutdown(client_sock, SD_BOTH);
    // cleanup
    closesocket(client_sock);
    WSACleanup();
    return 0;
}
