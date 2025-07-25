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
// 前置声明
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

// 数据管理类 - 负责数据收集、存储和管理系统相关信息
class DataManager {
public:
    static bool InitGlobalInstance();
    // 单例模式实现
    static DataManager& GetInstance();

    // 初始化所有收集器
    bool Initialize();
    // 清理资源
    void Cleanup();

    // 数据收集方法
    bool CollectProcesses();
    bool CollectServices();
    bool CollectConnections();
    bool CollectSessions();
    bool CollectSystemInfo();

    // 数据获取方法
    const std::vector<ProcessInfo>& GetProcesses() const;
    const std::vector<ServiceInfo>& GetServices() const;
    const std::vector<ConnectionInfo>& GetConnections() const;
    const std::vector<SessionInfo>& GetSessions() const;
    const SystemInfo& GetSystemInfo() const;

    // 进程操作
    bool TerminateTargetProcessByPid(DWORD pid);
    bool TerminateTargetProcessByName(const std::string& processName);

    // 服务操作
    bool StartTargetService(const std::wstring& serviceName);
    bool StopService(const std::wstring& serviceName);
    bool RestartService(const std::wstring& serviceName);

    // 数据导出
    //bool ExportProcessesToCSV(const std::wstring& filePath) const;
    //bool ExportServicesToCSV(const std::wstring& filePath) const;

    // 刷新设置
    void SetRefreshInterval(int seconds);
    void StartAutoRefresh();
    void StopAutoRefresh();
    void ManualRefresh();

    // 数据过滤
    std::vector<ProcessInfo> FilterProcesses(const std::wstring& searchText) const;
    std::vector<ServiceInfo> FilterServices(const std::wstring& searchText) const;

    // 数据展示
    void OutputProcesses(const std::vector<ProcessInfo>& processes = {}) const;
    void OutputServices(const std::vector<ServiceInfo>& services = {}) const;
    void OutputConnections(const std::vector<ConnectionInfo>& connections = {}) const;
    void OutputSessions(const std::vector<SessionInfo>& sessions = {}) const;
    void OutputSystemInfo() const;

private:
    // 私有构造函数，防止外部实例化
    DataManager();
    ~DataManager();

    // 禁止拷贝和赋值
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    // 数据存储
    std::vector<ProcessInfo> m_processes;
    std::vector<ServiceInfo> m_services;
    std::vector<ConnectionInfo> m_connections;
    std::vector<SessionInfo> m_sessions;
    std::unique_ptr<SystemInfo> m_systemInfo;

    // 收集器
    std::unique_ptr<ProcessCollector> m_processCollector;
    std::unique_ptr<ServiceCollector> m_serviceCollector;
    std::unique_ptr<NetworkCollector> m_networkCollector;
    std::unique_ptr<SessionCollector> m_sessionCollector;
    std::unique_ptr<SystemInfoCollector> m_systemInfoCollector;

    // 刷新设置
    int m_refreshInterval;
    bool m_autoRefreshRunning;
    std::thread* m_refreshThread;
    mutable std::mutex m_dataMutex;

    // 刷新线程函数
    void RefreshThreadFunction();
};