// DataManager.cpp

#include "DataManager.h"

bool DataManager::InitGlobalInstance() {
    return GetInstance().Initialize();
}
// ����ģʽʵ��
DataManager& DataManager::GetInstance() {
    static DataManager instance;
    return instance;
}

// ���캯��
DataManager::DataManager()
    : m_refreshInterval(5),
    m_autoRefreshRunning(false),
    m_refreshThread(nullptr) {
    // �����ռ���ʵ��
    m_processCollector = std::make_unique<ProcessCollector>();
    m_serviceCollector = std::make_unique<ServiceCollector>();
    m_networkCollector = std::make_unique<NetworkCollector>();
    m_sessionCollector = std::make_unique<SessionCollector>();
    m_systemInfoCollector = std::make_unique<SystemInfoCollector>();
}

// ��������
DataManager::~DataManager() {
    StopAutoRefresh();
    Cleanup();
    if (m_refreshThread) {
        delete m_refreshThread;
    }
}

// ��ʼ�������ռ���
bool DataManager::Initialize() {
    bool result = true;

    // ��ʼ�������ռ���
    if (!m_processCollector->Initialize()) {
        std::cerr << "Failed to initialize process collector!" << std::endl;
        result = false;
    }

    // ��ʼ�������ռ���
    if (!m_serviceCollector->Initialize()) {
        std::cerr << "Failed to initialize service collector!" << std::endl;
        result = false;
    }

     // ��ʼ�������ռ���
    if (!m_networkCollector->Initialize()) {
        std::cerr << "Failed to initialize network collector!" << std::endl;
        result = false;
    }

    // ��ʼ���Ự�ռ���
    if (!m_sessionCollector->Initialize()) {
        std::cerr << "Failed to initialize session collector!" << std::endl;
        result = false;
    }

    // ��ʼ��ϵͳ��Ϣ�ռ���
    if (!m_systemInfoCollector->Initialize()) {
        std::cerr << "Failed to initialize system info collector!" << std::endl;
        result = false;
    }

    return result;
}

// ������Դ
void DataManager::Cleanup() {
    m_processCollector->Cleanup();
    m_serviceCollector->Cleanup();
    m_networkCollector->Cleanup();
    m_sessionCollector->Cleanup();
    m_systemInfoCollector->Cleanup();
}

// �ռ�������Ϣ
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

// �ռ�������Ϣ
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

// �ռ�����������Ϣ
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

// �ռ��Ự��Ϣ
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

// �ռ�ϵͳ��Ϣ
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

// ��ȡ������Ϣ
const std::vector<ProcessInfo>& DataManager::GetProcesses() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_processes;
}

// ��ȡ������Ϣ
const std::vector<ServiceInfo>& DataManager::GetServices() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_services;
}

// ��ȡ����������Ϣ
const std::vector<ConnectionInfo>& DataManager::GetConnections() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_connections;
}

// ��ȡ�Ự��Ϣ
const std::vector<SessionInfo>& DataManager::GetSessions() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_sessions;
}

// ��ȡϵͳ��Ϣ
const SystemInfo& DataManager::GetSystemInfo() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    if (!m_systemInfo) {
        static SystemInfo emptyInfo;
        return emptyInfo;
    }
    return *m_systemInfo;
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

// ֹͣ����
bool DataManager::StopService(const std::wstring& serviceName) {
    // �򿪷�����ƹ��������ݿ⣬��ȡ����
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        // ����ʧ�ܣ����������Ϣ������ false
        std::cerr << "OpenSCManager failed!" << std::endl;
        return false;
    }

    // ��ָ�����Ƶķ��񣬻�ȡ����
    SC_HANDLE schService = OpenService(schSCManager, serviceName.c_str(), SERVICE_ALL_ACCESS);
    if (schService == NULL) {
        // ����ʧ�ܣ����������Ϣ���رշ�����ƹ�������������� false
        std::cerr << "OpenService failed for service: " << WideToMultiByte(serviceName) << std::endl;
        CloseServiceHandle(schSCManager);
        return false;
    }

    // ����һ�� SERVICE_STATUS �ṹ�����洢����״̬��Ϣ
    SERVICE_STATUS serviceStatus = { 0 };
    // ���� ControlService ��������ֹͣ����
    BOOL result = ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus);
    if (!result && GetLastError() == ERROR_SERVICE_NOT_ACTIVE) {
        // �������Ѿ�ֹͣ�������ʾ��Ϣ���� result ��Ϊ TRUE
        std::wcout << L"Service " << serviceName << L" is already stopped." << std::endl;
        result = TRUE;
    }

    // �رշ�����
    CloseServiceHandle(schService);
    // �رշ�����ƹ��������
    CloseServiceHandle(schSCManager);
    // ����ֹͣ��������Ľ��
    return result != 0;
}



// ��������
bool DataManager::RestartService(const std::wstring& serviceName) {
    if (!StopService(serviceName)) {
        return false;
    }

    // �ȴ�����ֹͣ
    Sleep(1000);

    return StartTargetService(serviceName);
}


//// ����������Ϣ�� CSV
//bool DataManager::ExportProcessesToCSV(const std::wstring& filePath) const {
//    std::lock_guard<std::mutex> lock(m_dataMutex);
//
//    std::wofstream file(filePath);
//    if (!file.is_open()) {
//        std::cerr << "Failed to open file for writing: " << WideToMultiByte(filePath) << std::endl;
//        return false;
//    }
//
//    // ���� UTF-8 ����
//    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>()));
//
//    // д���ͷ
//    file << L"PID,�� PID,������,��ִ���ļ�·��,������,����ʱ��,�ڴ�ռ��,CPU ʱ��" << std::endl;
//
//    // д������
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
//// ����������Ϣ��CSV
//bool DataManager::ExportServicesToCSV(const std::wstring& filePath) const {
//    std::lock_guard<std::mutex> lock(m_dataMutex);
//
//    std::wofstream file(filePath);
//    if (!file.is_open()) {
//        std::cerr << "Failed to open file for writing: " << WideToMultiByte(filePath) << std::endl;
//        return false;
//    }
//
//    // ����UTF-8����
//    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>()));
//
//    // д���ͷ
//    file << L"������,��ʾ����,״̬,��������,��ִ���ļ�·��" << std::endl;
//
//    // д������
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

// ����ˢ�¼��
void DataManager::SetRefreshInterval(int seconds) {
    m_refreshInterval = seconds;
}

// ��ʼ�Զ�ˢ��
void DataManager::StartAutoRefresh() {
    if (m_autoRefreshRunning) {
        return;
    }

    m_autoRefreshRunning = true;
    m_refreshThread = new std::thread(&DataManager::RefreshThreadFunction, this);
}

// ֹͣ�Զ�ˢ��
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

// �ֶ�ˢ��
void DataManager::ManualRefresh() {
    CollectProcesses();
    CollectServices();
    CollectConnections();
    CollectSessions();
    CollectSystemInfo();
}

// ���˽�����Ϣ
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

// ���˷�����Ϣ
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

// ���������Ϣ
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

// ���������Ϣ
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

// �������������Ϣ
void DataManager::OutputConnections(const std::vector<ConnectionInfo>& connections) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    const auto& connectionList = connections.empty() ? m_connections : connections;
    std::wcout << std::left << std::setw(8) << L"Protocol"  // Э�� �� Protocol
        << std::setw(25) << L"Local Address"             // ���ص�ַ �� Local Address
        << std::setw(25) << L"Remote Address"            // Զ�̵�ַ �� Remote Address
        << std::setw(15) << L"State"                     // ״̬ �� State
        << std::setw(8) << L"PID"                        // PID���ֲ���
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

// ����Ự��Ϣ
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

// ���ϵͳ��Ϣ
void DataManager::OutputSystemInfo() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    if (!m_systemInfo) {
        std::wcout << L"ϵͳ��Ϣδ�ռ�" << std::endl;
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

// ˢ���̺߳���
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
