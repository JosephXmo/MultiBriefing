/*
     MultiBriefing Communicator - An experimental program implementing chat over IP, and more.
     Copyright (C) 2024  Sibo Qiu, Runjie Miao, Yucheng Tong, and ? ?

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

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

// Win32 API
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// C standards
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")