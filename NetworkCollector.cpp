// NetworkCollector.cpp
#include "NetworkCollector.h"

NetworkCollector::NetworkCollector() : m_initialized(false) {}

NetworkCollector::~NetworkCollector() {
    Cleanup();
}

// �� NetworkCollector �� Initialize ������
bool NetworkCollector::Initialize() {
    WSADATA wsaData;
    // ��ʼ�� Winsock 2.2 �汾
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        // ������
        return false;
    }
    m_initialized = true;
    return true;
}

void NetworkCollector::Cleanup() {
    if (m_initialized) {
        WSACleanup();
        m_initialized = false;
    }
}

// ������������IPv4��ַת��Ϊ���ַ���
std::wstring FormatIpAddress(DWORD ipAddress) {
    char ipStr[46] = { 0 }; // �㹻�洢IPv6��ַ
    wchar_t ipWide[64] = { 0 };

    // ת��IP��ַΪ�ַ���
    inet_ntop(AF_INET, &ipAddress, ipStr, sizeof(ipStr));

    // ת��Ϊ���ַ�
    int len = MultiByteToWideChar(CP_UTF8, 0, ipStr, -1, ipWide, _countof(ipWide));
    if (len > 0) {
        return std::wstring(ipWide);
    }

    return L"0.0.0.0";
}

// ������������ʽ����ַ�Ͷ˿�
std::wstring FormatAddressAndPort(DWORD ipAddress, u_short port) {
    std::wostringstream oss;
    oss << FormatIpAddress(ipAddress) << L":" << ntohs(port);
    return oss.str();
}

// ������������ȡTCP״̬�Ŀ��ַ�����ʾ
std::wstring GetTcpStateDescription(DWORD state) {
    switch (state) {
    case MIB_TCP_STATE_CLOSED: return L"Closed";
    case MIB_TCP_STATE_LISTEN: return L"Listening";
    case MIB_TCP_STATE_SYN_SENT: return L"SYN Sent";
    case MIB_TCP_STATE_SYN_RCVD: return L"SYN Received";
    case MIB_TCP_STATE_ESTAB: return L"Established";
    case MIB_TCP_STATE_FIN_WAIT1: return L"FIN_WAIT1";
    case MIB_TCP_STATE_FIN_WAIT2: return L"FIN_WAIT2";
    case MIB_TCP_STATE_CLOSE_WAIT: return L"CLOSE_WAIT";
    case MIB_TCP_STATE_CLOSING: return L"CLOSING";
    case MIB_TCP_STATE_LAST_ACK: return L"LAST_ACK";
    case MIB_TCP_STATE_TIME_WAIT: return L"TIME_WAIT";
    default: return L"Unknown";
    }
}

bool NetworkCollector::CollectConnections(std::vector<ConnectionInfo>& connections) {
    if (!m_initialized) {
        return false;
    }

    connections.clear();

    // ��ȡTCP����
    DWORD size = 0;
    if (GetExtendedTcpTable(NULL, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) != ERROR_INSUFFICIENT_BUFFER) {
        std::cerr << "Failed to get TCP table size. Error: " << GetLastError() << std::endl;
        return false;
    }

    std::vector<unsigned char> buffer(size);
    MIB_TCPTABLE_OWNER_PID* tcpTable = reinterpret_cast<MIB_TCPTABLE_OWNER_PID*>(buffer.data());

    if (GetExtendedTcpTable(tcpTable, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) != NO_ERROR) {
        std::cerr << "Failed to get TCP table. Error: " << GetLastError() << std::endl;
        return false;
    }

    for (DWORD i = 0; i < tcpTable->dwNumEntries; ++i) {
        MIB_TCPROW_OWNER_PID row = tcpTable->table[i];

        ConnectionInfo conn;
        conn.protocol = IPPROTO_TCP;
        conn.localAddress = FormatAddressAndPort(row.dwLocalAddr, row.dwLocalPort);

        // ��ʽ��Զ�̵�ַ
        if (row.dwRemoteAddr == 0) {
            conn.remoteAddress = L"0.0.0.0:0";
        }
        else {
            conn.remoteAddress = FormatAddressAndPort(row.dwRemoteAddr, row.dwRemotePort);
        }

        // ��������״̬
        conn.state = GetTcpStateDescription(row.dwState);
        conn.pid = row.dwOwningPid;

        connections.push_back(conn);
    }

    // ��ȡUDP����
    size = 0;
    if (GetExtendedUdpTable(NULL, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0) != ERROR_INSUFFICIENT_BUFFER) {
        std::cerr << "Failed to get UDP table size. Error: " << GetLastError() << std::endl;
        return false;
    }

    buffer.resize(size);
    MIB_UDPTABLE_OWNER_PID* udpTable = reinterpret_cast<MIB_UDPTABLE_OWNER_PID*>(buffer.data());

    if (GetExtendedUdpTable(udpTable, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0) != NO_ERROR) {
        std::cerr << "Failed to get UDP table. Error: " << GetLastError() << std::endl;
        return false;
    }

    for (DWORD i = 0; i < udpTable->dwNumEntries; ++i) {
        MIB_UDPROW_OWNER_PID row = udpTable->table[i];

        ConnectionInfo conn;
        conn.protocol = IPPROTO_UDP;
        conn.localAddress = FormatAddressAndPort(row.dwLocalAddr, row.dwLocalPort);
        conn.remoteAddress = L"0.0.0.0:0";
        conn.state = L"Listening";
        conn.pid = row.dwOwningPid;

        connections.push_back(conn);
    }

    return true;
}