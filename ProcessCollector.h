#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <ctime>
#include<winternl.h>
#pragma comment(lib, "psapi.lib")

struct ProcessInfo {
    DWORD pid;
    DWORD parentPid;
    std::wstring processName;
    std::wstring executablePath;
    std::wstring commandLine;
    std::wstring creationTime;
    SIZE_T memoryUsage;
    FILETIME kernelTime;
    FILETIME userTime;
};

class ProcessCollector {
public:
    ProcessCollector();
    ~ProcessCollector();

    bool Initialize();
    void Cleanup();
    bool CollectProcesses(std::vector<ProcessInfo>& processes);
	bool TerminateProcessByPid(DWORD pid);
    bool TerminateProcessByNameA(const std::string& processName);
	bool TerminateProcessByNameW(const std::wstring& processName);
private:
    HANDLE hSnapshot;
    bool initialized;

    std::wstring GetProcessPath(DWORD pid);
    std::wstring GetCommandLine(DWORD pid);
    std::wstring FileTimeToString(const FILETIME& ft);
};