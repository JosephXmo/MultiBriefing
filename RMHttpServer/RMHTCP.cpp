#include "RMHTCP.h"
#include "RMHUtils.h"

namespace ReturnMyHairHTTP {
	TCPServer::TCPServer() {
		this->logFile = &GlobalLogLocation;
		this->iod = CreateMutex(NULL, NULL, "IO Daemon");
	}

	TCPServer::~TCPServer() {
		this->stop();
	}

	int TCPServer::start(std::string port) {
		int iResult;

		struct addrinfo* result = NULL;
		struct addrinfo hints;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &this->wsaData);
		if (iResult != 0) {
			log("WSAStartup failed with error: " + iResult, *this->logFile);
			return 1;
		}
		log("WSA Startup OK", *this->logFile);

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;
		log("Port Configure Ok", *this->logFile);

		// Resolve server port
		iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
		if (iResult != 0) {
			log("getaddrinfo failed with error: " + iResult, *this->logFile);
			WSACleanup();
			return 1;
		}
		log("Port info binding Ok", *this->logFile);

		// Create a SOCKET for the server to listen for client connections.
		this->ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (this->ListenSocket == INVALID_SOCKET) {
			log("socket failed with error: " + WSAGetLastError(), *this->logFile);
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}
		log("Socket construction Ok", *this->logFile);

		// Setup the TCP listening socket
		iResult = bind(this->ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			log("bind failed with error: " + WSAGetLastError(), *this->logFile);
			freeaddrinfo(result);
			closesocket(this->ListenSocket);
			WSACleanup();
			return 1;
		}
		log("Socket Binding Ok", *this->logFile);

		freeaddrinfo(result);
		log("Port info data struct released", *this->logFile);

		iResult = listen(this->ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			log("listen failed with error: " + WSAGetLastError(), *this->logFile);
			closesocket(this->ListenSocket);
			WSACleanup();
			return 1;
		}
		this->running = true;
		log("Ready. Listening port " + port, *this->logFile);

		this->mainSequence = CreateThread(
			NULL, NULL,
			(LPTHREAD_START_ROUTINE)TCPServer::DispatchMainSequence,
			(LPVOID)this,
			NULL, NULL
		);
	}

	int TCPServer::start(int port) {
		return TCPServer::start(std::to_string(port).c_str());
	}

	int TCPServer::stop() {
		this->running = false;

		closesocket(this->ListenSocket);
		WaitForSingleObject(this->mainSequence, 5000);
		TerminateThread(this->mainSequence, 0);
		WSACleanup();

		this->logFile->close();

		return 0;
	}

	void TCPServer::setLogLocation(std::string filepath) {
		if (filepath == "default") {
			this->logFile = &GlobalLogLocation;
			return;
		}

		if (!(this->logFile == &GlobalLogLocation))
			delete this->logFile;
		this->logFile = new std::ofstream();
		this->logFile->open(filepath, std::ios::app);
	}

	void TCPServer::setMaxConnection(short conNum) {
		this->maxConnections = conNum;
	}

	void TCPServer::DispatchMainSequence(TCPServer* serverContext) {
		while (serverContext->running) {
			// Server has spare resource for new client connection
			// Accept a client socket
			SOCKADDR ClientAddress;
			SOCKET ClientSocket = accept(serverContext->ListenSocket, &ClientAddress, NULL);

			// Handle accept failure
			if (ClientSocket == SOCKET_ERROR || ClientSocket == INVALID_SOCKET) {
				WaitForSingleObject(serverContext->iod, INFINITE);
				log("accept failed with error: " + WSAGetLastError(), *serverContext->logFile);
				ReleaseMutex(serverContext->iod);
				ClientSocket = INVALID_SOCKET;
				continue;
			}

			// Report client connection & information
			SOCKADDR TargetAddress;
			int TargetAddrLen = sizeof(SOCKADDR);
			std::pair<std::string, int> ResolvedAddress;
			if (getpeername(ClientSocket, &TargetAddress, &TargetAddrLen) == 0) {
				ResolvedAddress = resolveAddress(&TargetAddress);
				if (ResolvedAddress.first != "FAULT") {
					WaitForSingleObject(serverContext->iod, INFINITE);
					// log("Client connection from: " + ResolvedAddress.first + ':' + std::to_string(ResolvedAddress.second) + "->" + std::to_string(ClientSocket), *serverContext->logFile);
					ReleaseMutex(serverContext->iod);
				}
				else {
					WaitForSingleObject(serverContext->iod, INFINITE);
					log("WARNING: Address version is neither v4 nor v6. Unidentified address family.", *serverContext->logFile);
					ReleaseMutex(serverContext->iod);
				}
			}
			else {
				// Target information acquiring failed due to socket is closed or error
				WaitForSingleObject(serverContext->iod, INFINITE);
				log("Socket error. Client connection failed.", *serverContext->logFile);
				ReleaseMutex(serverContext->iod);
				continue;
			}

			// Information is enough to create the session
			auto session = new Session(true, ClientSocket, ResolvedAddress.first, ResolvedAddress.second, serverContext->iod, serverContext);

			// Server reaches maximum connection limit
			if (serverContext->connections >= serverContext->maxConnections) {
				CreateThread(
					NULL, NULL,
					(LPTHREAD_START_ROUTINE)TCPServer::FullConnectReject,
					(LPVOID)session,
					NULL, NULL
				);
				continue;
			}

			// Create client handler thread
			WaitForSingleObject(serverContext->iod, INFINITE);
			serverContext->sessionPool.push_back(session);
			ReleaseMutex(serverContext->iod);
			session->thread = CreateThread(
				NULL, NULL,
				(LPTHREAD_START_ROUTINE)TCPServer::HandleConnection,
				(LPVOID)session,
				NULL, NULL
			);
		}
	}

	int TCPServer::FullConnectReject(Session* session) {
		auto FailMsg = new Response(HTTPVersion::_1_1, 503);
		FailMsg->setBody("Server reached maximum connection limit. Try again later.");
		int iSendResult = send(session->socket, FailMsg->toString().c_str(), sizeof(FailMsg), 0);
		// log(session->response.getHead(), *session->parentServer->logFile);

		if (iSendResult == SOCKET_ERROR) {
			WaitForSingleObject(session->iod, INFINITE);
			session->parentServer->sessionPool.erase(std::remove(session->parentServer->sessionPool.begin(), session->parentServer->sessionPool.end(), session), session->parentServer->sessionPool.end());
			session->parentServer->connections--;
			log("Full Connect Rejection failed with error: " + WSAGetLastError(), *session->parentServer->logFile);
			ReleaseMutex(session->iod);
			closesocket(session->socket);
			delete session;
			
			return -1;
		}

		int iResult = shutdown(session->socket, SD_BOTH);
		closesocket(session->socket);
		session->active = false;
		WaitForSingleObject(session->iod, INFINITE);
		session->parentServer->sessionPool.erase(std::remove(session->parentServer->sessionPool.begin(), session->parentServer->sessionPool.end(), session), session->parentServer->sessionPool.end());
		session->parentServer->connections--;
		ReleaseMutex(session->iod);
		delete session;

		return iResult;
	}

	int TCPServer::HandleConnection(Session* session)
	{
		session->active = true;
		session->parentServer->connections++;

		int iResult, iSendResult = -1;
		char recvbuf[BUFLEN];
		std::string buffer;

		short loopCounter = 0;
		do {
			ZeroMemory(recvbuf, sizeof(recvbuf));
			iResult = recv(session->socket, recvbuf, BUFLEN, 0);
			if (iResult < 0) {
				// Server connection lost
				int WSAErrorCode = WSAGetLastError();
				if (WSAErrorCode == 10054)
					printf("Client forced quitted.\n");
				else
					printf("recv failed with error: %d\n", WSAGetLastError());

				// Client post-process
				closesocket(session->socket);
				session->active = false;
			
				WaitForSingleObject(session->iod, INFINITE);
				session->parentServer->sessionPool.erase(std::remove(session->parentServer->sessionPool.begin(), session->parentServer->sessionPool.end(), session), session->parentServer->sessionPool.end());
				session->parentServer->connections--;
				// log(session->address + ':' + std::to_string(session->port) + "->" + std::to_string(session->socket) + " connection lost.", *session->parentServer->logFile);
				ReleaseMutex(session->iod);
				delete session;

				return iResult;
			}
			buffer += std::string(recvbuf, iResult);

			// Divide by request
			// int requestEnd = buffer.find("\r\n\r\n") + 4;
			// std::string singleRequest = buffer.substr(0, requestEnd);
			// buffer = buffer.substr(requestEnd);

			// while (buffer.find("\r\n\r\n") != std::string::npos) {		// Until above bool

			// Resolve single request
			try {
				session->request.resolveRequest(buffer);
			}
			catch (const std::exception e) {
				if (std::string(e.what()).starts_with("canonical"))
					session->response.setHTTPStatusCode(404);
			}
			
			log(session->request.getHead(), *session->parentServer->logFile);
			session->response.setHTTPVersion(session->request.getHTTPVersion());

			/*
			 * We are using a pre-processing mechanism. Request can be decided which special case we are in.
			 * Then the session can be pre-labeled by assigning the response's status code.
			 * Any status processor should check if response's status code is already assigned.
			 * If is pre-assigned, proceed the corresponding procedure immediately.
			 */

			// Check if request is a ../ attack
			if (session->request.getPagePath() == "" || session->response.getHTTPStatusCode() == 403) {
				session->response.setHTTPStatusCode(403);
				session->response.setHeader("Content-Length", "0");
				session->response.setHeader("Location", "https://http.cat/" + session->response.getHTTPStatusCode());
					
				iSendResult = send(session->socket, session->response.toString().c_str(), session->response.toString().length(), 0);
				// log(session->response.getHead(), *session->parentServer->logFile);
				continue;
			}

			if (session->request.getMethod() == HTTPMethod::_GET) {
				std::ifstream file(session->request.getPagePath(), std::ios::binary);
				// Check if file exists
				if (!file || session->response.getHTTPStatusCode() == 404) {
					session->response.setHTTPStatusCode(404);
					session->response.setHeader("Content-Length", "0");
					session->response.setHeader("Location", "https://http.cat/" + std::to_string(session->response.getHTTPStatusCode()));
				}
				else {
					// Read file
					std::ostringstream contentStream;
					contentStream << file.rdbuf();
					session->response.setHTTPStatusCode(200);
					session->response.setBody(contentStream.str());
					session->response.setHeader("Content-Length", std::to_string(session->response.getBody().length()));
					session->response.setHeader("Content-Type", resolveFileMIME(session->request.getPagePath()));
					std::ostringstream().swap(contentStream);
				}

				iSendResult = send(session->socket, session->response.toString().c_str(), session->response.toString().length(), 0);
				session->response.setBody("");
				// log(session->response.getHead(), *session->parentServer->logFile);
			}

			bool isKeepAlive = session->request.getHTTPVersion() > _1_1 && session->request.getHeader("Connection") != "close";
			if (!isKeepAlive)
				break;

			loopCounter++;
		} while (loopCounter <= SAFELOOP);

		// shutdown the connection
		iResult = shutdown(session->socket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			int WSAErrorCode = WSAGetLastError();
			if (WSAErrorCode == 10038) {
				WaitForSingleObject(session->iod, INFINITE);
				session->parentServer->sessionPool.erase(std::remove(session->parentServer->sessionPool.begin(), session->parentServer->sessionPool.end(), session), session->parentServer->sessionPool.end());
				session->parentServer->connections--;
				log("Client forced quitted after transmission.", *session->parentServer->logFile);
				ReleaseMutex(session->iod);
			}
			else {
				WaitForSingleObject(session->iod, INFINITE);
				session->parentServer->sessionPool.erase(std::remove(session->parentServer->sessionPool.begin(), session->parentServer->sessionPool.end(), session), session->parentServer->sessionPool.end());
				session->parentServer->connections--;
				log("shutdown failed with error: " + WSAGetLastError(), *session->parentServer->logFile);
				ReleaseMutex(session->iod);
			}
				
		}

		closesocket(session->socket);
		session->active = false;
		WaitForSingleObject(session->iod, INFINITE);
		session->parentServer->sessionPool.erase(std::remove(session->parentServer->sessionPool.begin(), session->parentServer->sessionPool.end(), session), session->parentServer->sessionPool.end());
		session->parentServer->connections--;
		// log(session->address + ':' + std::to_string(session->port) + "->" + std::to_string(session->socket) + " disconnected.", *session->parentServer->logFile);
		ReleaseMutex(session->iod);
		delete session;

		return iResult;
	}
}