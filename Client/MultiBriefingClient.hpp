// Standard libraries and System library dependencies
#include "SystemDependency.hpp"

// Client config constants
#include "ClientConfigs.hpp"

// Include common types
#include "MultiBriefingTypes.hpp"

int __cdecl MSClient(void);
int ClientAlpha();

void Sender(SOCKET* ArgSocket);
void Receiver(SOCKET* ArgSocket);

SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress);
