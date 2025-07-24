#include "ProcessCollector.h"
#include <ranges>
#include <algorithm>

ProcessCollector::ProcessCollector() : hSnapshot(INVALID_HANDLE_VALUE), initialized(false) {}

ProcessCollector::~ProcessCollector() {
    Cleanup();
}

bool ProcessCollector::Initialize() {
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    initialized = true;
    return true;
}

void ProcessCollector::Cleanup() {
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        CloseHandle(hSnapshot);
        hSnapshot = INVALID_HANDLE_VALUE;
    }
    initialized = false;
}

bool ProcessCollector::CollectProcesses(std::vector<ProcessInfo>& processes) {
    if (!initialized) {
        if (!Initialize()) {
            return false;
        }
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        return false;
    }

    do {
        ProcessInfo info;
        info.pid = pe32.th32ProcessID;
        info.parentPid = pe32.th32ParentProcessID;
        info.processName = pe32.szExeFile;

        info.executablePath = GetProcessPath(info.pid);
        info.commandLine = GetCommandLine(info.pid);

        HANDLE hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            FALSE, info.pid
        );

        if (hProcess != NULL) {
            FILETIME createTime, exitTime, kernelTime, userTime;
            if (GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
                info.creationTime = FileTimeToString(createTime);
                info.kernelTime = kernelTime;
                info.userTime = userTime;
            }

            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                info.memoryUsage = pmc.WorkingSetSize;
            }

            CloseHandle(hProcess);
        }

        processes.push_back(info);
    } while (Process32Next(hSnapshot, &pe32));

    return true;
}

bool ProcessCollector::TerminateProcessByPid(DWORD pid)
{
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if(hProcess != NULL) {
        BOOL result = TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
        return result;
	}
    return false;
}

bool ProcessCollector::TerminateProcessByNameA(const std::string& processName) {
    // 1. ��std::stringת��Ϊstd::wstring��ʹ��ϵͳĬ�ϴ���ҳ��
    int requiredSize = MultiByteToWideChar(CP_ACP, 0, processName.c_str(), -1, NULL, 0);
    if (requiredSize == 0) {
        return false;
    }

    std::wstring wideName(requiredSize, 0);
    MultiByteToWideChar(CP_ACP, 0, processName.c_str(), -1, &wideName[0], requiredSize);

    // 2. ���ÿ��ַ��汾���߼�
    return TerminateProcessByNameW(wideName);
}

bool ProcessCollector::TerminateProcessByNameW(const std::wstring& processName) {
    // 1. �������̿���
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    // 2. ��������
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    bool success = false;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            // ת����������ΪСд�����ڲ����ִ�Сд�Ƚ�
            std::wstring currentName = pe32.szExeFile;
            std::transform(currentName.begin(), currentName.end(), currentName.begin(), ::towlower);

            std::wstring targetName = processName;
            std::transform(targetName.begin(), targetName.end(), targetName.begin(), ::towlower);

            // �����������Ƿ�ƥ��
            if (currentName == targetName) {
                // 3. �򿪽��̲���ȡ���
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                if (hProcess != NULL) {
                    // 4. ��ֹ����
                    if (TerminateProcess(hProcess, 0)) {
                        success = true;
                    }
                    CloseHandle(hProcess);
                }
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    // 5. �ͷſ��վ��
    CloseHandle(hSnapshot);
    return success;
}

std::wstring ProcessCollector::GetProcessPath(DWORD pid) {
    std::wstring path;
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE, pid
    );

    if (hProcess != NULL) {
        wchar_t buffer[MAX_PATH];
        if (GetModuleFileNameExW(hProcess, NULL, buffer, MAX_PATH)) {
            path = buffer;
        }
        CloseHandle(hProcess);
    }

    return path;
}

std::wstring ProcessCollector::GetCommandLine(DWORD pid) {
    std::wstring cmdLine;
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE, pid
    );

    if (hProcess != NULL) {
        HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
        if (hNtDll) {
            typedef NTSTATUS(WINAPI* pNtQueryInformationProcess)(
                HANDLE, DWORD, PVOID, ULONG, PULONG
                );

            pNtQueryInformationProcess NtQueryInformationProcess =
                (pNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess");

            if (NtQueryInformationProcess) {
                // ��ȡ���̻�����(PEB)��ַ
                PROCESS_BASIC_INFORMATION pbi = { 0 };
                ULONG returnLength = 0;
                NTSTATUS status = NtQueryInformationProcess(
                    hProcess,
                    ProcessBasicInformation,
                    &pbi,
                    sizeof(pbi),
                    &returnLength
                );

                if (NT_SUCCESS(status)) {
                    // ��ȡPEB�ṹ�е�ProcessParameters��Ա
                    struct {
                        ULONG Length;
                        BOOLEAN Unicode;
                        WCHAR Buffer[1];
                    } *commandLine = NULL;

                    // PEB.ProcessParameters ƫ����
                    PVOID processParameters = NULL;
                    SIZE_T bytesRead = 0;

                    // ��ȡ ProcessParameters ��ַ
                    if (ReadProcessMemory(
                        hProcess,
                        (PBYTE)pbi.PebBaseAddress + 0x10, // PEB.ProcessParameters ƫ����
                        &processParameters,
                        sizeof(processParameters),
                        &bytesRead
                    ) && bytesRead == sizeof(processParameters)) {

                        // ��ȡ RTL_USER_PROCESS_PARAMETERS �ṹ�е� CommandLine
                        UNICODE_STRING cmdLineUnicode = { 0 };
                        if (ReadProcessMemory(
                            hProcess,
                            (PBYTE)processParameters + 0x40, // ProcessParameters.CommandLine ƫ����
                            &cmdLineUnicode,
                            sizeof(cmdLineUnicode),
                            &bytesRead
                        ) && bytesRead == sizeof(cmdLineUnicode)) {

                            // �����ڴ�洢�������ַ���
                            wchar_t* cmdLineBuffer = new (std::nothrow) wchar_t[cmdLineUnicode.Length / sizeof(wchar_t) + 1];
                            if (cmdLineBuffer) {
                                if (ReadProcessMemory(
                                    hProcess,
                                    cmdLineUnicode.Buffer,
                                    cmdLineBuffer,
                                    cmdLineUnicode.Length,
                                    &bytesRead
                                ) && bytesRead == cmdLineUnicode.Length) {
                                    cmdLineBuffer[cmdLineUnicode.Length / sizeof(wchar_t)] = L'\0';
                                    cmdLine = cmdLineBuffer;
                                }
                                delete[] cmdLineBuffer;
                            }
                        }
                    }
                }
            }
        }
        CloseHandle(hProcess);
    }

    return cmdLine;
}

std::wstring ProcessCollector::FileTimeToString(const FILETIME& ft) {
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);

    wchar_t buffer[26];
    swprintf_s(buffer, L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);

    return std::wstring(buffer);
}