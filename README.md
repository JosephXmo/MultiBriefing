# RMHttpServer
### A Homebrew HTTP server program based on Win32 WinSock 2 TCP, Extended from UIC-COMP4093 Assignment 2

---

## How to build?

If your Visual Studio is installed on `C:\` default location, simply run `automake.bat`.
The script will automatically make the program using Visual Studio Developer Prompt.
However, it doesn't always start up server correctly. You still need to run it manually sometimes.

If you customized your cmake installation path, you may need to make the program by yourself.
In that case, you **MUST** make sure the executable is at the location of:

	`<root>\src\build\Release`

which means, the program's path is:

	`<root>\src\build\Release\RMHttpServer.exe`

Also, you are **HIGHLY RECOMMENDED** to run as Administrator.
Otherwise, some files may not be created as expected.


Program is built on Windows Socket 2, which means this program cannot be ported to any other platform,
even I chose CMake as compile platform.