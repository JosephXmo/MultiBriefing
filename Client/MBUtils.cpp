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

SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress) {
    int TargetAddrLen = sizeof(ParsedAddress);
    SimpleAddress* RetAddress = (SimpleAddress*)malloc(sizeof(SimpleAddress));

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

MBMessage ResolveMsg(MBClient sender, SYSTEMTIME time, char* msg) {
    MBMessage message{ sender, time, message.TEXT, NULL };

    if (msg[0] = '/') {
        message.type = message.COMMAND;
        for (int i = 0; msg[i] != '\0'; i++) {
            msg[i] = msg[i + 1];
        }
    }

    message.message = msg;

    return message;
}

int Log(char* msg) {
    BOOL writeOK = false;

    HANDLE hLogFile = CreateFile(
        "MBLog.log",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_NEW,
        FILE_APPEND_DATA,
        NULL
    );
    writeOK = WriteFile(hLogFile, NULL, NULL, nullptr, nullptr);
    CloseHandle(hLogFile);

    return writeOK;
}