#ifndef NETWORKCOLLECTOR_H
#define NETWORKCOLLECTOR_H


#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <locale>
#include <codecvt>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")



struct ConnectionInfo {
    int protocol; // IPPROTO_TCP or IPPROTO_UDP
    std::wstring localAddress;
    std::wstring remoteAddress;
    std::wstring state;
    DWORD pid;
};

class NetworkCollector {
public:
    NetworkCollector();
    ~NetworkCollector();

    bool Initialize();
    void Cleanup();

    bool CollectConnections(std::vector<ConnectionInfo>& connections);

private:
    bool m_initialized;
};

#endif // NETWORKCOLLECTOR_H