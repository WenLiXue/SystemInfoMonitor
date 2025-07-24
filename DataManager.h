// DataManager.h
#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include<Windows.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <fstream>
//
#include"ProcessCollector.h"
#include"ServiceCollector.h"
#include"NetworkCollector.h"
#include"SessionCollector.h"
#include"SystemInfoCollector.h"
// ǰ������
struct ProcessInfo;
struct ServiceInfo;
struct ConnectionInfo;
struct SessionInfo;
struct SystemInfo;

class ProcessCollector;
class ServiceCollector;
class NetworkCollector;
class SessionCollector;
class SystemInfoCollector;
std::string WideToMultiByte(const std::wstring& wstr);

// ���ݹ����� - ���������ռ����洢�͹���ϵͳ�����Ϣ
class DataManager {
public:
    static bool InitGlobalInstance();
    // ����ģʽʵ��
    static DataManager& GetInstance();

    // ��ʼ�������ռ���
    bool Initialize();
    // ������Դ
    void Cleanup();

    // �����ռ�����
    bool CollectProcesses();
    bool CollectServices();
    bool CollectConnections();
    bool CollectSessions();
    bool CollectSystemInfo();

    // ���ݻ�ȡ����
    const std::vector<ProcessInfo>& GetProcesses() const;
    const std::vector<ServiceInfo>& GetServices() const;
    const std::vector<ConnectionInfo>& GetConnections() const;
    const std::vector<SessionInfo>& GetSessions() const;
    const SystemInfo& GetSystemInfo() const;

    // ���̲���
    bool TerminateTargetProcessByPid(DWORD pid);
    bool TerminateTargetProcessByName(const std::string& processName);

    // �������
    bool StartTargetService(const std::wstring& serviceName);
    bool StopService(const std::wstring& serviceName);
    bool RestartService(const std::wstring& serviceName);

    // ���ݵ���
    //bool ExportProcessesToCSV(const std::wstring& filePath) const;
    //bool ExportServicesToCSV(const std::wstring& filePath) const;

    // ˢ������
    void SetRefreshInterval(int seconds);
    void StartAutoRefresh();
    void StopAutoRefresh();
    void ManualRefresh();

    // ���ݹ���
    std::vector<ProcessInfo> FilterProcesses(const std::wstring& searchText) const;
    std::vector<ServiceInfo> FilterServices(const std::wstring& searchText) const;

    // ����չʾ
    void OutputProcesses(const std::vector<ProcessInfo>& processes = {}) const;
    void OutputServices(const std::vector<ServiceInfo>& services = {}) const;
    void OutputConnections(const std::vector<ConnectionInfo>& connections = {}) const;
    void OutputSessions(const std::vector<SessionInfo>& sessions = {}) const;
    void OutputSystemInfo() const;

private:
    // ˽�й��캯������ֹ�ⲿʵ����
    DataManager();
    ~DataManager();

    // ��ֹ�����͸�ֵ
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    // ���ݴ洢
    std::vector<ProcessInfo> m_processes;
    std::vector<ServiceInfo> m_services;
    std::vector<ConnectionInfo> m_connections;
    std::vector<SessionInfo> m_sessions;
    std::unique_ptr<SystemInfo> m_systemInfo;

    // �ռ���
    std::unique_ptr<ProcessCollector> m_processCollector;
    std::unique_ptr<ServiceCollector> m_serviceCollector;
    std::unique_ptr<NetworkCollector> m_networkCollector;
    std::unique_ptr<SessionCollector> m_sessionCollector;
    std::unique_ptr<SystemInfoCollector> m_systemInfoCollector;

    // ˢ������
    int m_refreshInterval;
    bool m_autoRefreshRunning;
    std::thread* m_refreshThread;
    mutable std::mutex m_dataMutex;

    // ˢ���̺߳���
    void RefreshThreadFunction();
};