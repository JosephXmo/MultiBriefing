#include "MBUtils.hpp"

void ReportClientCounter(int ClientCounter) {
    printf("\tTotal connections: %d\n\n", ClientCounter);
}

SimpleAddress* ResolveAddress(SOCKADDR* ParsedAddress) {
    int TargetAddrLen = sizeof(ParsedAddress);
    SimpleAddress* RetAddress = (SimpleAddress*)malloc(sizeof(SimpleAddress));
    RetAddress->address = nullptr;
    RetAddress->port = -1;

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

long GenRandByTime() {
    srand(time(NULL));

    int high = rand(), low = rand();
    long ret;

    memcpy(&ret, &low, sizeof(low));
    memcpy(&ret + sizeof(low), &high, sizeof(high));

    return ret;
}