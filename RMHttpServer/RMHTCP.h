#ifndef _RMHTCP_H_
#define _RMHTCP_H_

#pragma once

// C Standard Library
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <thread>
#include <vector>

// Windows API Library
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// Project Constant Values & Enumerations
#include "RMHData.h"
#include "RMHFoundations.h"

// Definitions & Declarations
namespace ReturnMyHairHTTP {
	class Session;		// Make sure our reference to Session class can hit during compile

	class TCPServer {
	public:
		bool running = false;

		TCPServer();
		~TCPServer();

		int start(std::string port);
		int start(int port);
		int stop();

		void setLogLocation(std::string filepath);
		void setMaxConnection(short conNum);

	private:
		short connections = 0, maxConnections = 0;

		SOCKET ListenSocket = INVALID_SOCKET;
		WSADATA wsaData;
		std::ofstream *logFile;
		HANDLE iod = INVALID_HANDLE_VALUE;		// IO Protection Daemon.mutex
		HANDLE mainSequence = INVALID_HANDLE_VALUE;

		std::vector<Session*> sessionPool;

		static void DispatchMainSequence(TCPServer* serverContext);
		static int FullConnectReject(Session* sessionPtr);
		static int HandleConnection(Session* sessionPtr);
	};

	class Session {
	public:
		Session(
			bool active = true,
			SOCKET socket = INVALID_SOCKET,
			std::string address = "FAULT",
			int port = -1,
			HANDLE iod = INVALID_HANDLE_VALUE,
			TCPServer* parentServer = nullptr
		) :
			active(active),
			socket(socket),
			thread(INVALID_HANDLE_VALUE),
			address(address),
			port(port),
			iod(iod),
			parentServer(parentServer) {
		};

		bool active;
		SOCKET socket;
		HANDLE thread;
		std::string address;
		int port;
		HANDLE iod;		// IO Protection Daemon.mutex
		Request request;
		Response response;
		TCPServer* parentServer;
	};
}

#endif // !_RMHTCP_H_