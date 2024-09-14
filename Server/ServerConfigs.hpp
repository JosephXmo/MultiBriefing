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

// How many connections can be established simultaneously.
#define MAX_CONN 10

// How long can a client's name be.
#define MAX_NAME 20

// Default buffer length
#define DEFAULT_BUFLEN 512

// Default port for connection
#define DEFAULT_PORT "27015"