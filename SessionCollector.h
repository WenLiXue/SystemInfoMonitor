// SessionCollector.h
#ifndef SESSIONCOLLECTOR_H
#define SESSIONCOLLECTOR_H

#include <windows.h>
#include <wtsapi32.h>
#include <vector>
#include <string>

#pragma comment(lib, "wtsapi32.lib")

struct SessionInfo {
    DWORD sessionId;
    std::wstring userName;
    std::wstring domain;
    std::wstring loginTime;
    WTS_CONNECTSTATE_CLASS state;
};

class SessionCollector {
public:
    SessionCollector();
    ~SessionCollector();

    bool Initialize();
    void Cleanup();

    bool CollectSessions(std::vector<SessionInfo>& sessions);

private:
    bool m_initialized;
};

#endif // SESSIONCOLLECTOR_H    