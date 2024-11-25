#include <iostream>
#include <filesystem>

#include "RMHTCP.h"
#include "RMHData.h"
#include "RMHUtils.h"
#include "RMHFoundations.h"

int main()
{
	int maxconn = 0, port = 80;

	// Easy pre-configured mode?
	std::string inbuf;
	std::cout << "Are you a teacher or TA who is testing & scoring this project? (y/n) ";
	std::getline(std::cin, inbuf);

	// !
	if (inbuf == "y" || inbuf == "Y" || inbuf == "") {
		// Get local time
		std::time_t currentTime = std::time(nullptr);
		std::tm* localTime = std::localtime(&currentTime);
		char buffer[32];
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", localTime);

		// Check if log parent path exists
		std::filesystem::path logFile = std::filesystem::absolute("../log/session_" + std::string(buffer) + ".log");
		std::filesystem::path logRoot = logFile.parent_path();
		if (!logRoot.empty() && !std::filesystem::exists(logRoot))
			std::filesystem::create_directories(logRoot);

		// After handling no log path, create log file
		ReturnMyHairHTTP::GlobalLogLocation.open(std::filesystem::absolute(logFile).string(), std::ios::app);
		ReturnMyHairHTTP::log("Log file location: " + std::filesystem::absolute("../log/").string(), ReturnMyHairHTTP::GlobalLogLocation);

		// Open web root
		ReturnMyHairHTTP::GlobalWebRootPath = "../www";
		ReturnMyHairHTTP::log("Web root path: " + std::filesystem::absolute("../www").string(), ReturnMyHairHTTP::GlobalLogLocation);

		// Get max connection
		std::cout << "How many connections are you going to handle at most? (5 as default) ";
		std::getline(std::cin, inbuf);
		try {
			maxconn = std::stoi(inbuf);
		}
		catch (const std::invalid_argument& e) {
			maxconn = 5;
		}
		ReturnMyHairHTTP::log("Set max connection count to " + std::to_string(maxconn), ReturnMyHairHTTP::GlobalLogLocation);

		// Get HTTP listen port
		std::cout << "Establish HTTP on wich port? (80 as default) ";
		std::getline(std::cin, inbuf);
		try {
			port = std::stoi(inbuf);
		}
		catch (const std::invalid_argument& e) {
			port = 80;
		}
		ReturnMyHairHTTP::log("Set port number to " + std::to_string(port), ReturnMyHairHTTP::GlobalLogLocation);
	}
	// Standard mode
	else{
		std::cout << "Where do you want to save your log file?" << std::endl << "(Default: <home>\\log\\RMHTTP_session_<date>.log): ";
		std::string logFileName;
		std::getline(std::cin, logFileName);

		std::time_t currentTime = std::time(nullptr);
		std::tm* localTime = std::localtime(&currentTime);
		char buffer[32];
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", localTime);

		if (logFileName.empty()) logFileName = std::string(std::getenv("USERPROFILE")) + "\\log\\RMHTTP_session_" + std::string(buffer) + ".log";
		std::filesystem::path logRoot = std::filesystem::absolute(logFileName).parent_path();
		if (!logRoot.empty() && !std::filesystem::exists(logRoot))
			std::filesystem::create_directories(logRoot);
		logFileName = std::filesystem::absolute(logFileName).string();
		ReturnMyHairHTTP::GlobalLogLocation.open(logFileName, std::ios::app);
		if (!ReturnMyHairHTTP::GlobalLogLocation.is_open()) {
			std::cout << "Failed to open log file: " << logFileName << std::endl;
			return 1;
		}
		ReturnMyHairHTTP::log("Log file location: " + logFileName, ReturnMyHairHTTP::GlobalLogLocation);

		std::cout << "Where is your web root path?" << std::endl << "(Default: C:\\ProgramData\\www): ";
		std::string webRoot;
		std::getline(std::cin, webRoot);
		if (webRoot.empty()) webRoot = "C:/ProgramData/www";
		std::filesystem::path webRootPath = std::filesystem::absolute(webRoot);
		if (!std::filesystem::exists(webRootPath)) {
			ReturnMyHairHTTP::log("Web root not found. Exit the program.", ReturnMyHairHTTP::GlobalLogLocation);
			return 0;
		}
		ReturnMyHairHTTP::GlobalWebRootPath = webRoot;
		ReturnMyHairHTTP::log("Web root path: " + std::filesystem::absolute(webRoot).string(), ReturnMyHairHTTP::GlobalLogLocation);
	}

	auto tcpServer = new ReturnMyHairHTTP::TCPServer();
	tcpServer->setMaxConnection(maxconn);
	tcpServer->start(port);

	// TODO: Remove line below when project is mature
	std::cout << "\t<<<Hint>>>\tYou can type \"quit\" to terminate the server." << std::endl;

	std::string command = "";
	do {
		std::cin >> command;
		if (command != "quit")
			std::cout << "No such command" << std::endl;
	} while (command != "quit");

	tcpServer->running = false;
	tcpServer->stop();
	delete tcpServer;

	return 0;
}

// For DEBUG, not used in release version
void testDataTypes(void) {
	std::string rawRequest = "POST /upload HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 13\r\n"
		"\r\n"
		"Hello, World!";
	ReturnMyHairHTTP::Request myRequest;
	myRequest.resolveRequest(rawRequest);
	myRequest.report();

	ReturnMyHairHTTP::Response myResponse;
	myResponse.setHTTPVersion(ReturnMyHairHTTP::HTTPVersion::_3_0);
	myResponse.setHTTPStatusCode(404);
	myResponse.setHeader("Content-Type", "text/html; charset=utf-8");
	myResponse.setBody("Nothing here...");
	myResponse.report();
	std::cout << myResponse.toString() << std::endl;
}
