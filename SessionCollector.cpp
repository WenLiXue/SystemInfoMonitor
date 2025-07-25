// SessionCollector.cpp
#include "SessionCollector.h"
#include <iostream>

// 辅助函数：将FILETIME转换为字符串
inline std::wstring FileTimeToString(const FILETIME& ft) {
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
    // 无需特殊初始化
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

    // 枚举会话
    DWORD sessionCount = 0;
    PWTS_SESSION_INFOW sessionInfo = NULL;

    if (!WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessionInfo, &sessionCount)) {
        return false;
    }

    // 处理每个会话
    for (DWORD i = 0; i < sessionCount; ++i) {
        SessionInfo info;
        info.sessionId = sessionInfo[i].SessionId;
        info.state = sessionInfo[i].State;

        // 获取用户名
        LPWSTR userName = NULL;
        DWORD userNameSize = 0; // 新增变量用于存储数据大小
        if (WTSQuerySessionInformationW(
            WTS_CURRENT_SERVER_HANDLE,
            info.sessionId,
            WTSUserName,
            &userName,
            &userNameSize // 传递变量地址而非NULL
        )) {
            info.userName = userName ? userName : L"unknown";
            if (userName) WTSFreeMemory(userName);
        }
        else {
            info.userName = L"unknown";
        }

        // 获取域名
        LPWSTR domainName = NULL;
        DWORD domainNameSize = 0; // 新增变量用于存储数据大小
        if (WTSQuerySessionInformationW(
            WTS_CURRENT_SERVER_HANDLE,
            info.sessionId,
            WTSDomainName,
            &domainName,
            &domainNameSize // 传递变量地址而非NULL
        )) {
            info.domain = domainName ? domainName : L"unknown";
            if (domainName) WTSFreeMemory(domainName);
        }
        else {
            info.domain = L"unknown";
        }

        // 获取登录时间 (兼容旧SDK版本)
        FILETIME loginTime = { 0 };
        info.loginTime = L"unknown";

        // 尝试使用WTSQuerySessionInformation获取登录时间
        DWORD timeType = 0;
        LPWSTR timeStr = NULL;
        if (WTSQuerySessionInformationW(
            WTS_CURRENT_SERVER_HANDLE,
            info.sessionId,
            (WTS_INFO_CLASS)14, // 对应WTSConnectTime的值(如果SDK中未定义)
            &timeStr,
            &timeType // 这里已经正确传递了变量地址
        )) {
            if (timeType == sizeof(FILETIME)) {
                FILETIME* pTime = reinterpret_cast<FILETIME*>(timeStr);
                info.loginTime = FileTimeToString(*pTime);
            }
            if (timeStr) WTSFreeMemory(timeStr);
        }

        sessions.push_back(info);
    }

    // 释放会话信息内存
    if (sessionInfo) {
        WTSFreeMemory(sessionInfo);
    }

    return true;
}