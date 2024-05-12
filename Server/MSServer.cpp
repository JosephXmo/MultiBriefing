#include "MultiBriefingServer.hpp"

SOCKET ListenSocket = INVALID_SOCKET;

bool LoopProceedFlag = true;

int __cdecl MSServer(void)
{
	WSADATA wsaData;
	int iResult;

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

	while (LoopProceedFlag) {
		// Server reaches maximum connection limit
		if (ClientsTable.counter == MAX_CONN) {
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
		if (ClientSocket == SOCKET_ERROR || ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			continue;
		}

		// Create MBClient Object
		MBClient* Client = (MBClient*)malloc(sizeof(MBClient));
		// ZeroMemory(&Client, sizeof(MBClient));
		

		Client->id = GenRandByTime();
		Client->name = FetchName(ClientSocket);
		Client->socket = ClientSocket;

		RegisterClient(&ClientsTable, Client);

		// Report client connection & information
		SOCKADDR TargetAddress;
		int TargetAddrLen = sizeof(TargetAddress);
		if (getpeername(ClientSocket, &TargetAddress, &TargetAddrLen) == 0) {
			SimpleAddress* saTargetAddress = ResolveAddress(&TargetAddress);

			if (saTargetAddress != nullptr) {
				printf(
					"Client connection from: %s->%d@%s\n",
					Client->name,
					Client->socket,
					saTargetAddress->address
				);
				ReportClientCounter(ClientsTable.counter);
			}
			else printf("WARNING: Address version is neither v4 nor v6. Unidentified address family!\n");
		}
		else {
			// Not only is name not successfully fetched, socket is closed or error. 
			// Prompt, Deregister, Continue loop
			printf_s("Socket error. Client connection failed.\n");
			DeregisterClient(&ClientsTable, Client);
			ReportClientCounter(ClientsTable.counter);

			// Reset used local variable and report
			ClientSocket = INVALID_SOCKET;
			Client = nullptr;

			printf("Waiting for client connection...\n\n");

			continue;
		}

		// Create client handler thread
		CreateThread(
			NULL, NULL,
			(LPTHREAD_START_ROUTINE)Communication,
			(LPVOID)Client,
			NULL, NULL
		);

		// Reset used local variable and report
		ClientSocket = INVALID_SOCKET;
		Client = nullptr;

		printf("Waiting for client connection...\n\n");
	}

	// Total cleanup
	// Attention that when performing a multi-client service, server needs to listen continuously.
	// Therefore the listening socket must be closed when server shut down.
	closesocket(ListenSocket);
	WSACleanup();


	return 0;
}

int FullConnectReject(SOCKET ClientSocket) {
	const char* FailMsg = "Server reached maximum connection limit. Try again later.";
	int iSendResult = send(ClientSocket, FailMsg, sizeof(FailMsg), 0);

	if (iSendResult == SOCKET_ERROR) {
		printf("Full Connect Rejection failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		return -1;
	}

	int iResult = shutdown(ClientSocket, SD_BOTH);
	closesocket(ClientSocket);

	return iSendResult;
}

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
			// free(targetClient->name);
			free(targetClient);

			return ClientsTable->counter;
		}
	}

	return -1;
}

void Communication(MBClient* Client) {
	int iResult, iSendResult = -1;
	char recvbuf[DEFAULT_BUFLEN];

	// Receive until the peer shuts down the connection
	do {
		ZeroMemory(recvbuf, sizeof(recvbuf));
		iResult = recv(Client->socket, recvbuf, DEFAULT_BUFLEN, 0);

		if (iResult > 0) {
			printf("%s: %s (%d Bytes)\n", Client->name, recvbuf, iResult);

			// Echo the buffer to all connected clients
			int ForwardCounter = 0;
			for (int i = 0; i < MAX_CONN; i++) {
				if (ClientsTable.table[i] == nullptr) continue;

				if (ClientsTable.table[i]->id == Client->id) continue;

				char sendbuf[DEFAULT_BUFLEN];
				ZeroMemory(sendbuf, DEFAULT_BUFLEN * sizeof(char));
				sprintf_s(
					sendbuf, DEFAULT_BUFLEN,
					"%s> %s",
					Client->name,
					recvbuf
				);

				iSendResult = send(ClientsTable.table[i]->socket, sendbuf, (int)strlen(sendbuf), 0);
				if (iSendResult == SOCKET_ERROR) printf("send failed with error: %d\n", WSAGetLastError());
				else ForwardCounter++;
			}
			printf("Echoed to %d clients.\n\n", ForwardCounter);
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
		}
		else {
			int WSAErrorCode = WSAGetLastError();
			if (WSAErrorCode == 10054)
				printf("Client forced quitted.\n");
			else
				printf("recv failed with error: %d\n", WSAGetLastError());

			// Client post-process
			closesocket(Client->socket);
			char tempname[MAX_NAME];
			strcpy_s(tempname, Client->name);
			DeregisterClient(&ClientsTable, Client);
			char AnnounceMsg[DEFAULT_BUFLEN];
			sprintf_s(AnnounceMsg, "%s left this chat.\n", tempname);
			Announcement(&ClientsTable, AnnounceMsg);
			ReportClientCounter(ClientsTable.counter);
			return;
		}
	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(Client->socket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		int WSAErrorCode = WSAGetLastError();
		if (WSAErrorCode == 10038)
			printf("Client forced quitted after transmission.\n");
		else
			printf("shutdown failed with error: %d\n", WSAGetLastError());
	}

	// Client post-process
	closesocket(Client->socket);
	char tempname[MAX_NAME];
	strcpy_s(tempname, Client->name);
	DeregisterClient(&ClientsTable, Client);
	char AnnounceMsg[DEFAULT_BUFLEN];
	sprintf_s(AnnounceMsg, "%s left this chat.\n", tempname);
	Announcement(&ClientsTable, AnnounceMsg);
	ReportClientCounter(ClientsTable.counter);
}
