// SystemInfoCollector.cpp
#include "SystemInfoCollector.h"
#include <lmcons.h> // ����UNLEN����س�������
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <winternl.h>

#pragma comment(lib, "psapi.lib")

// ������������ȡCPU��Ϣ
std::wstring GetCpuInfo() {
    std::wstring cpuInfo = L"δ֪";

    // ��ȡCPU��Ϣ
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    // ��ȡCPU����
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

// ������������ȡϵͳ����ʱ��
std::wstring GetSystemUpTime() {
    FILETIME ftNow, ftSystemStart;
    GetSystemTimeAsFileTime(&ftNow);

    ULARGE_INTEGER uiNow, uiSystemStart;
    uiNow.LowPart = ftNow.dwLowDateTime;
    uiNow.HighPart = ftNow.dwHighDateTime;

    // ��ȡϵͳ����ʱ��
    DWORD upTime = GetTickCount();
    ULONGLONG systemStartTime = uiNow.QuadPart - (upTime * 10000); // ת��Ϊ100����Ϊ��λ

    ftSystemStart.dwLowDateTime = systemStartTime & 0xFFFFFFFF;
    ftSystemStart.dwHighDateTime = systemStartTime >> 32;

    // ת��Ϊ����ʱ��
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
    // ���������ʼ��
    return true;
}

void SystemInfoCollector::Cleanup() {
    // ������������
}

std::unique_ptr<SystemInfo> SystemInfoCollector::CollectSystemInfo() {
    auto systemInfo = std::make_unique<SystemInfo>();

    // ��ȡ����ϵͳ�汾����� GetVersionExW��
    systemInfo->osVersion = GetOsVersionString();

    // ��ȡ������
    wchar_t hostName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
    DWORD hostNameSize = MAX_COMPUTERNAME_LENGTH + 1;

    if (!GetComputerNameW(hostName, &hostNameSize)) {
        systemInfo->hostName = L"δ֪����";
        std::wcerr << L"GetComputerNameW failed with error: " << GetLastError() << std::endl;
    }
    else {
        systemInfo->hostName = hostName;
    }

    // ��ȡ��ǰ�û���
    wchar_t userName[UNLEN + 1] = { 0 };
    DWORD userNameSize = UNLEN + 1;

    if (!GetUserNameW(userName, &userNameSize)) {
        systemInfo->userName = L"δ֪�û�";
        std::wcerr << L"GetUserNameW failed with error: " << GetLastError() << std::endl;
    }
    else {
        systemInfo->userName = userName;
    }

    // ��ȡϵͳ����ʱ��
    systemInfo->systemUpTime = GetSystemUpTime();

    // ��ȡ�ڴ���Ϣ
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

    // ��ȡCPU��Ϣ
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    systemInfo->cpuCores = sysInfo.dwNumberOfProcessors;
    systemInfo->cpuInfo = GetCpuInfo();

    return systemInfo;
}

// ��� GetVersionExW �ĺ���
std::wstring SystemInfoCollector::GetOsVersionString() {
    // ʹ�� RtlGetVersion (NT �ڲ�����)
    typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

    // ��̬���� ntdll.dll �е� RtlGetVersion ����
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) {
        return L"�޷���ȡ����ϵͳ��Ϣ";
    }

    RtlGetVersionPtr pRtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(
        GetProcAddress(hNtdll, "RtlGetVersion")
        );

    if (!pRtlGetVersion) {
        return L"�޷���ȡ����ϵͳ��Ϣ";
    }

    RTL_OSVERSIONINFOW osInfo = { 0 };
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);

    // ���� RtlGetVersion ��ȡϵͳ�汾
    NTSTATUS status = pRtlGetVersion(&osInfo);
    if (!NT_SUCCESS(status)) {
        return L"�޷���ȡ����ϵͳ��Ϣ";
    }

    // ���ݰ汾�Ź����ѺõĲ���ϵͳ����
    std::wostringstream osVersion;

    // �ж� Windows �汾
    if (osInfo.dwMajorVersion == 10) {
        // ����Ƿ��� Windows 11 (ͨ���汾�Ż����������ж�)
        // ע��: Windows 11 ��ʹ�� major version 10���� build number >= 22000
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

    // ��Ӱ汾����Ϣ
    osVersion << L" (Build " << osInfo.dwBuildNumber << L")";

    return osVersion.str();
}