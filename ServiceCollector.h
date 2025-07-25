// ServiceCollector.h
#ifndef SERVICECOLLECTOR_H
#define SERVICECOLLECTOR_H

#include <windows.h>
#include <winsvc.h>
#include <vector>
#include <string>

#pragma comment(lib, "advapi32.lib")

struct ServiceInfo {
    std::wstring serviceName;
    std::wstring displayName;
    DWORD status;
    DWORD startType;
    std::wstring binaryPath;
};

class ServiceCollector {
public:
    ServiceCollector();
    ~ServiceCollector();
    
    bool Initialize();
    void Cleanup();
    
    bool CollectServices(std::vector<ServiceInfo>& services);
    
    bool StartService(const std::wstring& serviceName);
    bool StopService(const std::wstring& serviceName);
    bool RestartService(const std::wstring& serviceName);
    
private:
    SC_HANDLE m_scmHandle;
};

#endif // SERVICECOLLECTOR_H    