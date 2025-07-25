// ServiceCollector.h
#ifndef SERVICECOLLECTOR_H
#define SERVICECOLLECTOR_H

#include <windows.h>
#include <winsvc.h>
#include <vector>
#include <string>
// 安全检查：仅在未定义时添加
#ifndef SERVICE_AUTO_START_DELAYED
#define SERVICE_AUTO_START_DELAYED 0x00000020
#endif
// 定义自定义的"未知"启动类型值（使用未被系统使用的DWORD值）
#ifndef SERVICE_TYPE_UNKNOWN
#define SERVICE_TYPE_UNKNOWN 0xFFFFFFFF
#endif

#pragma comment(lib, "advapi32.lib")



struct ServiceInfo {
    std::wstring serviceName;
    std::wstring displayName;
    DWORD status;
    DWORD startType;
    std::wstring startTypeStr;
    std::wstring binaryPath;

};

class ServiceCollector {
public:
    ServiceCollector();
    ~ServiceCollector();
    std::wstring StartTypeToString(DWORD startType);
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