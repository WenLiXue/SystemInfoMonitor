// SystemInfoCollector.h
#ifndef SYSTEMINFOCOLLECTOR_H
#define SYSTEMINFOCOLLECTOR_H

#include <windows.h>
#include <vector>
#include <string>
#include<memory>
struct SystemInfo {
    std::wstring osVersion;
    std::wstring hostName;
    std::wstring userName;
    std::wstring systemUpTime;
    ULONGLONG totalPhysicalMemory;
    ULONGLONG availablePhysicalMemory;
    std::wstring cpuInfo;
    DWORD cpuCores;

    FILETIME idleTime;      // 空闲 CPU 时间
    FILETIME kernelTime;    // 内核模式 CPU 时间
    FILETIME userTime;      // 用户模式 CPU 时间
};

class SystemInfoCollector {
public:
    SystemInfoCollector();
    ~SystemInfoCollector();

    bool Initialize();
    void Cleanup();

    std::unique_ptr<SystemInfo> CollectSystemInfo();

    std::wstring GetOsVersionString();
    
};

#endif // SYSTEMINFOCOLLECTOR_H    