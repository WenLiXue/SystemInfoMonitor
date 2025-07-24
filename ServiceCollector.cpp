// ServiceCollector.cpp
#include "ServiceCollector.h"
#include <iostream>
#include <string>

ServiceCollector::ServiceCollector() : m_scmHandle(NULL) {}

ServiceCollector::~ServiceCollector() {
    Cleanup();
}

bool ServiceCollector::Initialize() {
    m_scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    return m_scmHandle != NULL;
}

void ServiceCollector::Cleanup() {
    if (m_scmHandle) {
        CloseServiceHandle(m_scmHandle);
        m_scmHandle = NULL;
    }
}

bool ServiceCollector::CollectServices(std::vector<ServiceInfo>& services) {
    if (!m_scmHandle) {
        return false;
    }

    services.clear();

    // ��ȡ���������ʹ�С
    DWORD bytesNeeded = 0;
    DWORD servicesReturned = 0;
    DWORD resumeHandle = 0;

    EnumServicesStatusExW(
        m_scmHandle,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32 | SERVICE_DRIVER,
        SERVICE_STATE_ALL,
        NULL,
        0,
        &bytesNeeded,
        &servicesReturned,
        &resumeHandle,
        NULL
    );

    if (GetLastError() != ERROR_MORE_DATA) {
        return false;
    }

    // �����ڴ�
    std::vector<unsigned char> buffer(bytesNeeded);

    // �ٴε��û�ȡ������Ϣ
    if (!EnumServicesStatusExW(
        m_scmHandle,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32 | SERVICE_DRIVER,
        SERVICE_STATE_ALL,
        reinterpret_cast<LPBYTE>(buffer.data()),  // ��ʽ����ת��
        bytesNeeded,
        &bytesNeeded,
        &servicesReturned,
        &resumeHandle,
        NULL
    )) {
        return false;
    }

    // ����ÿ������
    ENUM_SERVICE_STATUS_PROCESSW* servicesBuffer =
        reinterpret_cast<ENUM_SERVICE_STATUS_PROCESSW*>(buffer.data());  // ��ʽ����ת��

    for (DWORD i = 0; i < servicesReturned; ++i) {
        ServiceInfo service;
        service.serviceName = servicesBuffer[i].lpServiceName;
        service.displayName = servicesBuffer[i].lpDisplayName;
        service.status = servicesBuffer[i].ServiceStatusProcess.dwCurrentState;

        // ��ȡ�����������ͺͶ�����·��
        SC_HANDLE serviceHandle = OpenServiceW(
            m_scmHandle,
            servicesBuffer[i].lpServiceName,
            SERVICE_QUERY_CONFIG
        );

        if (serviceHandle) {
            // ��ȡ����������Ϣ
            DWORD configBytesNeeded = 0;
            QueryServiceConfigW(serviceHandle, NULL, 0, &configBytesNeeded);

            std::vector<unsigned char> configBuffer(configBytesNeeded);
            LPQUERY_SERVICE_CONFIGW config =
                reinterpret_cast<LPQUERY_SERVICE_CONFIGW>(configBuffer.data());  // ��ʽ����ת��

            if (QueryServiceConfigW(serviceHandle, config, configBytesNeeded, &configBytesNeeded)) {
                service.startType = config->dwStartType;
                service.binaryPath = config->lpBinaryPathName;
            }

            CloseServiceHandle(serviceHandle);
        }

        services.push_back(service);
    }

    return true;
}

bool ServiceCollector::StartService(const std::wstring& serviceName) {
    if (!m_scmHandle) {
        return false;
    }

    SC_HANDLE serviceHandle = OpenServiceW(
        m_scmHandle,
        serviceName.c_str(),
        SERVICE_START
    );

    if (!serviceHandle) {
        return false;
    }

    bool result = ::StartServiceW(serviceHandle, 0, NULL) != 0;
    CloseServiceHandle(serviceHandle);
    return result;
}

bool ServiceCollector::StopService(const std::wstring& serviceName) {
    if (!m_scmHandle) {
        return false;
    }

    SC_HANDLE serviceHandle = OpenServiceW(
        m_scmHandle,
        serviceName.c_str(),
        SERVICE_STOP | SERVICE_QUERY_STATUS
    );

    if (!serviceHandle) {
        return false;
    }

    SERVICE_STATUS serviceStatus;
    bool result = ControlService(serviceHandle, SERVICE_CONTROL_STOP, &serviceStatus) != 0;
    CloseServiceHandle(serviceHandle);
    return result;
}

bool ServiceCollector::RestartService(const std::wstring& serviceName) {
    if (!StopService(serviceName)) {
        return false;
    }

    // �ȴ�����ֹͣ
    Sleep(1000);

    return StartService(serviceName);
}