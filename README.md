# MultiBriefing Project
### A Homebrew Chat Program Based on TCP with Win32, Extended from UIC-COMP3003 Assignment 2

A simple console chat program using TCP protocol, implemented with Win32 API.
Someone wants to port this app to Linux and macOS but he doesn't have any time.

Clients defaultly join lobby group when they first connect to server.
Server is capable of grouping clients from lobby, via console commands.

In future, we might implement a TUI by rendering console buffer. 
Portability will limit the function completeness.


Reference Webpages:

https://blog.csdn.net/ap1005834/article/details/127091142

https://blog.csdn.net/m0_48660921/article/details/122382490

https://blog.csdn.net/sinat_21107433/article/details/80869887

https://blog.csdn.net/qq1113673178/article/details/132178553

https://blog.csdn.net/m0_65544927/article/details/134001816

https://learn.microsoft.com/zh-cn/windows/console/console-screen-buffers

https://www.cnblogs.com/MakeView660/p/9237990.html

Windows provides Windows Socket API (a.k.a. WSA), Winsock, has 2 versions currently: WinSock 1.1 and WinSock 2.0

WinSock 1.1 and WinSock 2.0 are backward-compatible: both source code and binary code.
WinSock 2.0 is the extension of WinSock 1.1, including adding more asynchronized functions.
These extensions are for Windows programming, for example, making async I/O with Event Notification;
while WinSock 1.1 was ported from UNIX BSD socket.

To use WinSock 2, programmers simply need to include new header `WinSock2.h` and link `Ws2_32.lib` as below:
```C++
　　// WinSock1.1:
　　#include <winsock.h> 
　　#pragma comment(lib, "wsock32.lib")

　　// WinSock2.0:
　　#include <winsock2.h>
　　#pragma comment(lib, "ws2_32.lib")
```
