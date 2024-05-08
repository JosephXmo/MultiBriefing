#include "SystemDependency.hpp"
#include "MultiBriefingTypes.hpp"

struct MBClientNode {
    MBClient* mbClient;
    struct MBClientNode* next;  // Pointer to next MBClient
};

MBClientNode* createClientList(MBClient* mbClient);
MBClientNode* appendClient(MBClientNode* head, MBClientNode* newClient);
MBClientNode* dequeue(MBClientNode* head, MBClient* target);