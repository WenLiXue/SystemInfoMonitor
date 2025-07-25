// SystemInfoCollector.cpp
#include "SystemInfoCollector.h"
#include <lmcons.h> // 包含UNLEN和相关常量定义
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <winternl.h>

#pragma comment(lib, "psapi.lib")

// 辅助函数：获取CPU信息
std::wstring GetCpuInfo() {
    std::wstring cpuInfo = L"未知";

    // 获取CPU信息
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    // 获取CPU名称
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        WCHAR cpuName[512] = { 0 };
        DWORD size = 512;
        if (RegQueryValueExW(hKey, L"ProcessorNameString", 0, NULL, (LPBYTE)cpuName, &size) == ERROR_SUCCESS) {
            cpuInfo = cpuName;
        }
        RegCloseKey(hKey);
    }

    return cpuInfo;
}

// 辅助函数：获取系统启动时间
std::wstring GetSystemUpTime() {
    FILETIME ftNow, ftSystemStart;
    GetSystemTimeAsFileTime(&ftNow);

    ULARGE_INTEGER uiNow, uiSystemStart;
    uiNow.LowPart = ftNow.dwLowDateTime;
    uiNow.HighPart = ftNow.dwHighDateTime;

    // 获取系统启动时间
    DWORD upTime = GetTickCount();
    ULONGLONG systemStartTime = uiNow.QuadPart - (upTime * 10000); // 转换为100纳秒为单位

    ftSystemStart.dwLowDateTime = systemStartTime & 0xFFFFFFFF;
    ftSystemStart.dwHighDateTime = systemStartTime >> 32;

    // 转换为本地时间
    SYSTEMTIME stLocal;
    FileTimeToLocalFileTime(&ftSystemStart, &ftSystemStart);
    FileTimeToSystemTime(&ftSystemStart, &stLocal);

    wchar_t buffer[26];
    swprintf_s(buffer, L"%04d-%02d-%02d %02d:%02d:%02d",
        stLocal.wYear, stLocal.wMonth, stLocal.wDay,
        stLocal.wHour, stLocal.wMinute, stLocal.wSecond);

    return std::wstring(buffer);
}

SystemInfoCollector::SystemInfoCollector() {}

SystemInfoCollector::~SystemInfoCollector() {}

bool SystemInfoCollector::Initialize() {
    // 无需特殊初始化
    return true;
}

void SystemInfoCollector::Cleanup() {
    // 无需特殊清理
}

std::unique_ptr<SystemInfo> SystemInfoCollector::CollectSystemInfo() {
    auto systemInfo = std::make_unique<SystemInfo>();

    // 获取操作系统版本（替代 GetVersionExW）
    systemInfo->osVersion = GetOsVersionString();

    // 获取主机名
    wchar_t hostName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
    DWORD hostNameSize = MAX_COMPUTERNAME_LENGTH + 1;

    if (!GetComputerNameW(hostName, &hostNameSize)) {
        systemInfo->hostName = L"未知主机";
        std::wcerr << L"GetComputerNameW failed with error: " << GetLastError() << std::endl;
    }
    else {
        systemInfo->hostName = hostName;
    }

    // 获取当前用户名
    wchar_t userName[UNLEN + 1] = { 0 };
    DWORD userNameSize = UNLEN + 1;

    if (!GetUserNameW(userName, &userNameSize)) {
        systemInfo->userName = L"未知用户";
        std::wcerr << L"GetUserNameW failed with error: " << GetLastError() << std::endl;
    }
    else {
        systemInfo->userName = userName;
    }

    // 获取系统启动时间
    systemInfo->systemUpTime = GetSystemUpTime();

    // 获取内存信息
    MEMORYSTATUSEX memInfo = { 0 };
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (!GlobalMemoryStatusEx(&memInfo)) {
        systemInfo->totalPhysicalMemory = 0;
        systemInfo->availablePhysicalMemory = 0;
        std::wcerr << L"GlobalMemoryStatusEx failed with error: " << GetLastError() << std::endl;
    }
    else {
        systemInfo->totalPhysicalMemory = memInfo.ullTotalPhys;
        systemInfo->availablePhysicalMemory = memInfo.ullAvailPhys;
    }

    // 获取CPU信息
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    systemInfo->cpuCores = sysInfo.dwNumberOfProcessors;
    systemInfo->cpuInfo = GetCpuInfo();

    return systemInfo;
}

// 替代 GetVersionExW 的函数
std::wstring SystemInfoCollector::GetOsVersionString() {
    // 使用 RtlGetVersion (NT 内部函数)
    typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

    // 动态加载 ntdll.dll 中的 RtlGetVersion 函数
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) {
        return L"无法获取操作系统信息";
    }

    RtlGetVersionPtr pRtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(
        GetProcAddress(hNtdll, "RtlGetVersion")
        );

    if (!pRtlGetVersion) {
        return L"无法获取操作系统信息";
    }

    RTL_OSVERSIONINFOW osInfo = { 0 };
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);

    // 调用 RtlGetVersion 获取系统版本
    NTSTATUS status = pRtlGetVersion(&osInfo);
    if (!NT_SUCCESS(status)) {
        return L"无法获取操作系统信息";
    }

    // 根据版本号构建友好的操作系统名称
    std::wostringstream osVersion;

    // 判断 Windows 版本
    if (osInfo.dwMajorVersion == 10) {
        // 检查是否是 Windows 11 (通过版本号或其他特征判断)
        // 注意: Windows 11 仍使用 major version 10，但 build number >= 22000
        if (osInfo.dwBuildNumber >= 22000) {
            osVersion << L"Windows 11";
        }
        else {
            osVersion << L"Windows 10";
        }
    }
    else if (osInfo.dwMajorVersion == 6) {
        switch (osInfo.dwMinorVersion) {
        case 3: osVersion << L"Windows 8.1"; break;
        case 2: osVersion << L"Windows 8"; break;
        case 1: osVersion << L"Windows 7"; break;
        case 0: osVersion << L"Windows Vista"; break;
        default: osVersion << L"Windows NT 6.x"; break;
        }
    }
    else if (osInfo.dwMajorVersion == 5) {
        switch (osInfo.dwMinorVersion) {
        case 2: osVersion << L"Windows Server 2003"; break;
        case 1: osVersion << L"Windows XP"; break;
        default: osVersion << L"Windows NT 5.x"; break;
        }
    }
    else {
        osVersion << L"Windows NT " << osInfo.dwMajorVersion << L"." << osInfo.dwMinorVersion;
    }

    // 添加版本号信息
    osVersion << L" (Build " << osInfo.dwBuildNumber << L")";

    return osVersion.str();
}