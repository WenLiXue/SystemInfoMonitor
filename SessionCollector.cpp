// SessionCollector.cpp
#include "SessionCollector.h"
#include <iostream>

// ������������FILETIMEת��Ϊ�ַ���
std::wstring FileTimeToString(const FILETIME& ft) {
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);

    wchar_t buffer[26];
    swprintf_s(buffer, L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);

    return std::wstring(buffer);
}

SessionCollector::SessionCollector() : m_initialized(false) {}

SessionCollector::~SessionCollector() {
    Cleanup();
}

bool SessionCollector::Initialize() {
    // ���������ʼ��
    m_initialized = true;
    return true;
}

void SessionCollector::Cleanup() {
    m_initialized = false;
}

bool SessionCollector::CollectSessions(std::vector<SessionInfo>& sessions) {
    if (!m_initialized) {
        return false;
    }

    sessions.clear();

    // ö�ٻỰ
    DWORD sessionCount = 0;
    PWTS_SESSION_INFOW sessionInfo = NULL;

    if (!WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessionInfo, &sessionCount)) {
        return false;
    }

    // ����ÿ���Ự
    for (DWORD i = 0; i < sessionCount; ++i) {
        SessionInfo info;
        info.sessionId = sessionInfo[i].SessionId;
        info.state = sessionInfo[i].State;

        // ��ȡ�û���
        LPWSTR userName = NULL;
        DWORD userNameSize = 0; // �����������ڴ洢���ݴ�С
        if (WTSQuerySessionInformationW(
            WTS_CURRENT_SERVER_HANDLE,
            info.sessionId,
            WTSUserName,
            &userName,
            &userNameSize // ���ݱ�����ַ����NULL
        )) {
            info.userName = userName ? userName : L"unknown";
            if (userName) WTSFreeMemory(userName);
        }
        else {
            info.userName = L"unknown";
        }

        // ��ȡ����
        LPWSTR domainName = NULL;
        DWORD domainNameSize = 0; // �����������ڴ洢���ݴ�С
        if (WTSQuerySessionInformationW(
            WTS_CURRENT_SERVER_HANDLE,
            info.sessionId,
            WTSDomainName,
            &domainName,
            &domainNameSize // ���ݱ�����ַ����NULL
        )) {
            info.domain = domainName ? domainName : L"unknown";
            if (domainName) WTSFreeMemory(domainName);
        }
        else {
            info.domain = L"unknown";
        }

        // ��ȡ��¼ʱ�� (���ݾ�SDK�汾)
        FILETIME loginTime = { 0 };
        info.loginTime = L"unknown";

        // ����ʹ��WTSQuerySessionInformation��ȡ��¼ʱ��
        DWORD timeType = 0;
        LPWSTR timeStr = NULL;
        if (WTSQuerySessionInformationW(
            WTS_CURRENT_SERVER_HANDLE,
            info.sessionId,
            (WTS_INFO_CLASS)14, // ��ӦWTSConnectTime��ֵ(���SDK��δ����)
            &timeStr,
            &timeType // �����Ѿ���ȷ�����˱�����ַ
        )) {
            if (timeType == sizeof(FILETIME)) {
                FILETIME* pTime = reinterpret_cast<FILETIME*>(timeStr);
                info.loginTime = FileTimeToString(*pTime);
            }
            if (timeStr) WTSFreeMemory(timeStr);
        }

        sessions.push_back(info);
    }

    // �ͷŻỰ��Ϣ�ڴ�
    if (sessionInfo) {
        WTSFreeMemory(sessionInfo);
    }

    return true;
}