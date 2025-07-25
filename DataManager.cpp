// DataManager.cpp

#include "DataManager.h"

bool DataManager::InitGlobalInstance() {
    return GetInstance().Initialize();
}
// 单例模式实现
DataManager& DataManager::GetInstance() {
    static DataManager instance;
    return instance;
}

// 析构函数
DataManager::~DataManager() {
    StopAutoRefresh();
    Cleanup();
    if (m_refreshThread) {
        delete m_refreshThread;
    }
}

// 构造函数 - 只负责创建对象，不执行耗时或可能失败的操作
DataManager::DataManager()
    : m_refreshInterval(5),
    m_autoRefreshRunning(false),
    m_refreshThread(nullptr),
    m_initialized(false) {

    // 创建收集器实例
    m_processCollector = std::make_unique<ProcessCollector>();
    m_serviceCollector = std::make_unique<ServiceCollector>();
    m_networkCollector = std::make_unique<NetworkCollector>();
    m_sessionCollector = std::make_unique<SessionCollector>();
    m_systemInfoCollector = std::make_unique<SystemInfoCollector>();
}

// 初始化所有收集器 - 线程安全版本
bool DataManager::Initialize() {
    // 使用互斥锁保证线程安全
    std::lock_guard<std::mutex> lock(m_initMutex);

    // 如果已经初始化过，直接返回成功
    if (m_initialized) return true;

    // 按顺序初始化各个收集器，任何一个失败则终止并返回错误
    if (!m_processCollector->Initialize()) {
        std::cerr << "Failed to initialize process collector!" << std::endl;
        Cleanup(); // 清理已初始化的资源
        return false;
    }

    if (!m_serviceCollector->Initialize()) {
        std::cerr << "Failed to initialize service collector!" << std::endl;
        Cleanup();
        return false;
    }

    if (!m_networkCollector->Initialize()) {
        std::cerr << "Failed to initialize network collector!" << std::endl;
        Cleanup();
        return false;
    }

    if (!m_sessionCollector->Initialize()) {
        std::cerr << "Failed to initialize session collector!" << std::endl;
        Cleanup();
        return false;
    }

    if (!m_systemInfoCollector->Initialize()) {
        std::cerr << "Failed to initialize system info collector!" << std::endl;
        Cleanup();
        return false;
    }

    // 所有收集器都初始化成功
    m_initialized = true;
    return true;
}

// 清理资源
void DataManager::Cleanup() {
    // 按相反顺序释放资源
    if (m_systemInfoCollector) m_systemInfoCollector->Cleanup();
    if (m_sessionCollector) m_sessionCollector->Cleanup();
    if (m_networkCollector) m_networkCollector->Cleanup();
    if (m_serviceCollector) m_serviceCollector->Cleanup();
    if (m_processCollector) m_processCollector->Cleanup();
    // 重置初始化状态
    m_initialized = false;
}

// 收集进程信息
bool DataManager::CollectProcesses() {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    std::vector<ProcessInfo> processes;
    if (!m_processCollector->CollectProcesses(processes)) {
        std::cerr << "Failed to collect processes!" << std::endl;
        return false;
    }

    m_processes = std::move(processes);
    return true;
}

// 收集服务信息
bool DataManager::CollectServices() {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    std::vector<ServiceInfo> services;
    if (!m_serviceCollector->CollectServices(services)) {
        std::cerr << "Failed to collect services!" << std::endl;
        return false;
    }

    m_services = std::move(services);
    return true;
}

// 收集网络连接信息
bool DataManager::CollectConnections() {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    std::vector<ConnectionInfo> connections;
    if (!m_networkCollector->CollectConnections(connections)) {
        std::cerr << "Failed to collect connections!" << std::endl;
        return false;
    }

    m_connections = std::move(connections);
    return true;
}

// 收集会话信息
bool DataManager::CollectSessions() {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    std::vector<SessionInfo> sessions;
    if (!m_sessionCollector->CollectSessions(sessions)) {
        std::cerr << "Failed to collect sessions!" << std::endl;
        return false;
    }

    m_sessions = std::move(sessions);
    return true;
}

// 收集系统信息
bool DataManager::CollectSystemInfo() {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    std::unique_ptr<SystemInfo> systemInfo = m_systemInfoCollector->CollectSystemInfo();
    if (!systemInfo) {
        std::cerr << "Failed to collect system info!" << std::endl;
        return false;
    }

    m_systemInfo = std::move(systemInfo);
    return true;
}

// 获取进程信息
const std::vector<ProcessInfo>& DataManager::GetProcesses() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_processes;
}

// 获取服务信息
const std::vector<ServiceInfo>& DataManager::GetServices() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_services;
}

// 获取网络连接信息
const std::vector<ConnectionInfo>& DataManager::GetConnections() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_connections;
}

// 获取会话信息
const std::vector<SessionInfo>& DataManager::GetSessions() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_sessions;
}

// 获取系统信息
const SystemInfo& DataManager::GetSystemInfo() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    if (!m_systemInfo) {
        static SystemInfo emptyInfo;
        return emptyInfo;
    }
    return *m_systemInfo;
}

// 辅助函数：将FILETIME转换为64位整数（单位：100纳秒）
ULONGLONG FileTimeToUInt64(const FILETIME& ft) {
    ULARGE_INTEGER ul;
    ul.LowPart = ft.dwLowDateTime;
    ul.HighPart = ft.dwHighDateTime;
    return ul.QuadPart;
}

// 计算CPU使用率
const double DataManager::GetCpuUsage() const {
    std::lock_guard<std::mutex> lock(m_dataMutex); // 确保线程安全

    // 检查系统信息是否有效
    if (!m_systemInfo) {
        return 0.0;
    }

    const SystemInfo& currentInfo = *m_systemInfo;

    // 首次调用时，仅记录当前CPU时间，不计算使用率
    static FILETIME s_lastIdleTime = { 0 };
    static FILETIME s_lastKernelTime = { 0 };
    static FILETIME s_lastUserTime = { 0 };
    static bool s_isFirstCall = true;

    if (s_isFirstCall) {
        // 记录首次采集的时间
        s_lastIdleTime = currentInfo.idleTime;
        s_lastKernelTime = currentInfo.kernelTime;
        s_lastUserTime = currentInfo.userTime;
        s_isFirstCall = false;
        return 0.0; // 首次调用无法计算，返回0
    }

    // 计算两次采集的时间差值（单位：100纳秒）
    ULONGLONG idleDiff = FileTimeToUInt64(currentInfo.idleTime) - FileTimeToUInt64(s_lastIdleTime);
    ULONGLONG kernelDiff = FileTimeToUInt64(currentInfo.kernelTime) - FileTimeToUInt64(s_lastKernelTime);
    ULONGLONG userDiff = FileTimeToUInt64(currentInfo.userTime) - FileTimeToUInt64(s_lastUserTime);

    // 更新上次采集的时间（用于下次计算）
    s_lastIdleTime = currentInfo.idleTime;
    s_lastKernelTime = currentInfo.kernelTime;
    s_lastUserTime = currentInfo.userTime;

    // 总CPU时间 = 内核时间差 + 用户时间差（系统总消耗的CPU时间）
    ULONGLONG totalDiff = kernelDiff + userDiff;

    // 避免除零错误（时间差为0时返回0）
    if (totalDiff == 0) {
        return 0.0;
    }

    // CPU使用率 = (1 - 空闲时间差 / 总时间差) × 100%
    double usage = (1.0 - static_cast<double>(idleDiff) / totalDiff) * 100.0;

    // 确保使用率在0-100之间（避免浮点计算误差）
    return max(0.0, min(100.0, usage));
}

bool DataManager::TerminateTargetProcessByPid(DWORD pid)
{
    return m_processCollector->TerminateProcessByPid(pid);
}

bool DataManager::TerminateTargetProcessByName(const std::string& processName)
{
    return m_processCollector->TerminateProcessByNameA(processName);
}


bool DataManager::StartTargetService(const std::wstring& serviceName) {
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        std::cerr << "OpenSCManager failed!" << std::endl;
        return false;
    }

    SC_HANDLE schService = OpenServiceW(schSCManager, serviceName.c_str(), SERVICE_ALL_ACCESS);
    if (schService == NULL) {
        std::cerr << "OpenServiceW failed for service: " << WideToMultiByte(serviceName) << std::endl;
        CloseServiceHandle(schSCManager);
        return false;
    }

    BOOL result = ::StartServiceW(schService, 0, NULL);
    if (!result && GetLastError() == ERROR_SERVICE_ALREADY_RUNNING) {
        std::wcout << L"Service " << serviceName << L" is already running." << std::endl;
        result = TRUE;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return result != 0;
}

// 停止服务
bool DataManager::StopService(const std::wstring& serviceName) {
    // 打开服务控制管理器数据库，获取其句柄
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        // 若打开失败，输出错误信息并返回 false
        std::cerr << "OpenSCManager failed!" << std::endl;
        return false;
    }

    // 打开指定名称的服务，获取其句柄
    SC_HANDLE schService = OpenService(schSCManager, serviceName.c_str(), SERVICE_ALL_ACCESS);
    if (schService == NULL) {
        // 若打开失败，输出错误信息，关闭服务控制管理器句柄并返回 false
        std::cerr << "OpenService failed for service: " << WideToMultiByte(serviceName) << std::endl;
        CloseServiceHandle(schSCManager);
        return false;
    }

    // 定义一个 SERVICE_STATUS 结构体来存储服务状态信息
    SERVICE_STATUS serviceStatus = { 0 };
    // 调用 ControlService 函数尝试停止服务
    BOOL result = ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus);
    if (!result && GetLastError() == ERROR_SERVICE_NOT_ACTIVE) {
        // 若服务已经停止，输出提示信息并将 result 设为 TRUE
        std::wcout << L"Service " << serviceName << L" is already stopped." << std::endl;
        result = TRUE;
    }

    // 关闭服务句柄
    CloseServiceHandle(schService);
    // 关闭服务控制管理器句柄
    CloseServiceHandle(schSCManager);
    // 返回停止服务操作的结果
    return result != 0;
}



// 重启服务
bool DataManager::RestartService(const std::wstring& serviceName) {
    if (!StopService(serviceName)) {
        return false;
    }

    // 等待服务停止
    Sleep(1000);

    return StartTargetService(serviceName);
}


//// 导出进程信息到 CSV
//bool DataManager::ExportProcessesToCSV(const std::wstring& filePath) const {
//    std::lock_guard<std::mutex> lock(m_dataMutex);
//
//    std::wofstream file(filePath);
//    if (!file.is_open()) {
//        std::cerr << "Failed to open file for writing: " << WideToMultiByte(filePath) << std::endl;
//        return false;
//    }
//
//    // 设置 UTF-8 编码
//    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>()));
//
//    // 写入表头
//    file << L"PID,父 PID,进程名,可执行文件路径,命令行,创建时间,内存占用,CPU 时间" << std::endl;
//
//    // 写入数据
//    for (const auto& process : m_processes) {
//        file << process.pid << L","
//            << process.parentPid << L","
//            << process.processName << L","
//            << process.executablePath << L","
//            << process.commandLine << L","
//            << process.creationTime << L","
//            << process.memoryUsage << L","
//            << std::endl;
//    }
//
//    file.close();
//    return true;
//}
//
//
//// 导出服务信息到CSV
//bool DataManager::ExportServicesToCSV(const std::wstring& filePath) const {
//    std::lock_guard<std::mutex> lock(m_dataMutex);
//
//    std::wofstream file(filePath);
//    if (!file.is_open()) {
//        std::cerr << "Failed to open file for writing: " << WideToMultiByte(filePath) << std::endl;
//        return false;
//    }
//
//    // 设置UTF-8编码
//    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>()));
//
//    // 写入表头
//    file << L"服务名,显示名称,状态,启动类型,可执行文件路径" << std::endl;
//
//    // 写入数据
//    for (const auto& service : m_services) {
//        file << service.serviceName << L","
//            << service.displayName << L","
//            << std::to_wstring(service.status) << L","
//            << std::to_wstring(service.startType) << L","
//            << service.binaryPath << std::endl;
//    }
//
//    file.close();
//    return true;
//}

// 设置刷新间隔
void DataManager::SetRefreshInterval(int seconds) {
    m_refreshInterval = seconds;
}

// 开始自动刷新
void DataManager::StartAutoRefresh() {
    if (m_autoRefreshRunning) {
        return;
    }

    m_autoRefreshRunning = true;
    m_refreshThread = new std::thread(&DataManager::RefreshThreadFunction, this);
}

// 停止自动刷新
void DataManager::StopAutoRefresh() {
    if (!m_autoRefreshRunning) {
        return;
    }

    m_autoRefreshRunning = false;
    if (m_refreshThread && m_refreshThread->joinable()) {
        m_refreshThread->join();
        delete m_refreshThread;
        m_refreshThread = nullptr;
    }
}

// 手动刷新
void DataManager::ManualRefresh() {
    CollectProcesses();
    CollectServices();
    CollectConnections();
    CollectSessions();
    CollectSystemInfo();
}

// 过滤进程信息
std::vector<ProcessInfo> DataManager::FilterProcesses(const std::wstring& searchText) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    std::vector<ProcessInfo> result;
    if (searchText.empty()) {
        result = m_processes;
        return result;
    }

    std::wstring lowerSearchText = searchText;
    std::transform(lowerSearchText.begin(), lowerSearchText.end(), lowerSearchText.begin(), ::towlower);

    for (const auto& process : m_processes) {
        std::wstring lowerProcessName = process.processName;
        std::transform(lowerProcessName.begin(), lowerProcessName.end(), lowerProcessName.begin(), ::towlower);

        std::wstring pidStr = std::to_wstring(process.pid);

        if (lowerProcessName.find(lowerSearchText) != std::wstring::npos ||
            pidStr.find(lowerSearchText) != std::wstring::npos) {
            result.push_back(process);
        }
    }

    return result;
}

// 过滤服务信息
std::vector<ServiceInfo> DataManager::FilterServices(const std::wstring& searchText) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    std::vector<ServiceInfo> result;
    if (searchText.empty()) {
        result = m_services;
        return result;
    }

    std::wstring lowerSearchText = searchText;
    std::transform(lowerSearchText.begin(), lowerSearchText.end(), lowerSearchText.begin(), ::towlower);

    for (const auto& service : m_services) {
        std::wstring lowerServiceName = service.serviceName;
        std::transform(lowerServiceName.begin(), lowerServiceName.end(), lowerServiceName.begin(), ::towlower);

        std::wstring lowerDisplayName = service.displayName;
        std::transform(lowerDisplayName.begin(), lowerDisplayName.end(), lowerDisplayName.begin(), ::towlower);

        std::wstring lowerStatus = std::to_wstring(service.status);
        std::transform(lowerStatus.begin(), lowerStatus.end(), lowerStatus.begin(), ::towlower);

        if (lowerServiceName.find(lowerSearchText) != std::wstring::npos ||
            lowerDisplayName.find(lowerSearchText) != std::wstring::npos ||
            lowerStatus.find(lowerSearchText) != std::wstring::npos) {
            result.push_back(service);
        }
    }

    return result;
}

// 输出进程信息
void DataManager::OutputProcesses(const std::vector<ProcessInfo>& processes) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    const auto& processList = processes.empty() ? m_processes : processes;

    std::wcout << std::left << std::setw(8) << L"PID"
        << std::setw(8) << L"PPID"
        << std::setw(25) << L"Process Name"
        << std::setw(20) << L"Memory Usage"
        << std::setw(20) << L"Creation Time"
        << std::endl;

    std::wcout << std::wstring(93, L'-') << std::endl;

    for (const auto& process : processList) {
        std::wcout << std::left << std::setw(8) << process.pid
            << std::setw(8) << process.parentPid
            << std::setw(25) << process.processName.substr(0, 22)
            << std::setw(20) << process.memoryUsage
            << std::setw(20) << process.creationTime
            << std::endl;
    }
}

// 输出服务信息
void DataManager::OutputServices(const std::vector<ServiceInfo>& services) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    const auto& serviceList = services.empty() ? m_services : services;

    std::wcout << std::left << std::setw(25) << L"Service Name"
        << std::setw(30) << L"Display Name"
        << std::setw(15) << L"Status"
        << std::setw(15) << L"Start Type"
        << std::endl;

    std::wcout << std::wstring(85, L'-') << std::endl;

    for (const auto& service : serviceList) {
        std::wcout << std::left << std::setw(25) << service.serviceName.substr(0, 22)
            << std::setw(30) << service.displayName.substr(0, 27)
            << std::setw(15) << std::to_wstring(service.status)
            << std::setw(15) << std::to_wstring(service.startType)
            << std::endl;
    }
}

// 输出网络连接信息
void DataManager::OutputConnections(const std::vector<ConnectionInfo>& connections) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    const auto& connectionList = connections.empty() ? m_connections : connections;
    std::wcout << std::left << std::setw(8) << L"Protocol"  // 协议 → Protocol
        << std::setw(25) << L"Local Address"             // 本地地址 → Local Address
        << std::setw(25) << L"Remote Address"            // 远程地址 → Remote Address
        << std::setw(15) << L"State"                     // 状态 → State
        << std::setw(8) << L"PID"                        // PID保持不变
        << std::endl;

    std::wcout << std::wstring(81, L'-') << std::endl;

    for (const auto& connection : connectionList) {
        std::wcout << std::left << std::setw(8) << (connection.protocol == IPPROTO_TCP ? L"TCP" : L"UDP")
            << std::setw(25) << connection.localAddress
            << std::setw(25) << connection.remoteAddress
            << std::setw(15) << connection.state
            << std::setw(8) << connection.pid
            << std::endl;
    }
}

// 输出会话信息
void DataManager::OutputSessions(const std::vector<SessionInfo>& sessions) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    const auto& sessionList = sessions.empty() ? m_sessions : sessions;

    std::wcout << std::left << std::setw(10) << L"Session ID"
        << std::setw(20) << L"Username"
        << std::setw(20) << L"Domain"
        << std::setw(20) << L"Login Time"
        << std::setw(15) << L"Status"
        << std::endl;

    std::wcout << std::wstring(85, L'-') << std::endl;

    for (const auto& session : sessionList) {
        std::wcout << std::left << std::setw(10) << session.sessionId
            << std::setw(20) << session.userName
            << std::setw(20) << session.domain
            << std::setw(20) << session.loginTime
            << std::setw(15) << std::to_wstring(session.state)
            << std::endl;
    }
}

// 输出系统信息
void DataManager::OutputSystemInfo() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    if (!m_systemInfo) {
        std::wcout << L"系统信息未收集" << std::endl;
        return;
    }

    const auto& info = *m_systemInfo;

    std::wcout << L"OS Version: " << info.osVersion << std::endl;
    std::wcout << L"Hostname: " << info.hostName << std::endl;
    std::wcout << L"Current User: " << info.userName << std::endl;
    std::wcout << L"System Uptime: " << info.systemUpTime << std::endl;
    std::wcout << L"Physical Memory: " << std::to_wstring(info.totalPhysicalMemory)
        << L" (Available: " << std::to_wstring(info.availablePhysicalMemory) << L")" << std::endl;
    std::wcout << L"CPU: " << info.cpuInfo << L" (" << info.cpuCores << L" cores)" << std::endl;
}

// 刷新线程函数
void DataManager::RefreshThreadFunction() {
    while (m_autoRefreshRunning) {
        ManualRefresh();
        std::this_thread::sleep_for(std::chrono::seconds(m_refreshInterval));
    }
}

std::string WideToMultiByte(const std::wstring& wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
