#include "systeminfowidget.h"
#include "ui_systeminfowidget.h"
#include <QDateTime>
#include <sstream>

// 辅助函数：将字节转换为GB（保留2位小数）
QString bytesToGB(ULONGLONG bytes) {
    double gb = static_cast<double>(bytes) / (1024 * 1024 * 1024);
    return QString::number(gb, 'f', 2) + " GB";
}

SystemInfoWidget::SystemInfoWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SystemInfoWidget),
    m_dataManager(DataManager::GetInstance()) // 获取DataManager单例
{
    ui->setupUi(this);
    SystemInfoWidget::initUI();       // 初始化UI组件
    refreshSystemInfo(); // 初始加载数据

    // 绑定刷新按钮事件
    connect(m_refreshBtn, &QPushButton::clicked, this, &SystemInfoWidget::onRefreshClicked);
}

SystemInfoWidget::~SystemInfoWidget() {
    delete ui;
}

// 初始化UI组件
void SystemInfoWidget::initUI() {
    // ===== 主布局 =====
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20); // 组间间距

    // ===== 1. 操作系统信息组 =====
    m_osGroup = new QGroupBox("操作系统信息", this);
    QVBoxLayout* osLayout = new QVBoxLayout(m_osGroup);
    osLayout->setSpacing(10); // 项间距

    // 操作系统版本
    QHBoxLayout* osVersionLayout = new QHBoxLayout();
    osVersionLayout->addWidget(new QLabel("操作系统版本：", this));
    m_osVersionLabel = new QLabel("-", this); // 初始占位符
    osVersionLayout->addWidget(m_osVersionLabel);
    osVersionLayout->addStretch(); // 右对齐拉伸
    osLayout->addLayout(osVersionLayout);

    // 主机名
    QHBoxLayout* hostNameLayout = new QHBoxLayout();
    hostNameLayout->addWidget(new QLabel("主机名：", this));
    m_hostNameLabel = new QLabel("-", this);
    hostNameLayout->addWidget(m_hostNameLabel);
    hostNameLayout->addStretch();
    osLayout->addLayout(hostNameLayout);

    // 用户名
    QHBoxLayout* userNameLayout = new QHBoxLayout();
    userNameLayout->addWidget(new QLabel("当前用户：", this));
    m_userNameLabel = new QLabel("-", this);
    userNameLayout->addWidget(m_userNameLabel);
    userNameLayout->addStretch();
    osLayout->addLayout(userNameLayout);

    // 系统运行时间
    QHBoxLayout* upTimeLayout = new QHBoxLayout();
    upTimeLayout->addWidget(new QLabel("系统运行时间：", this));
    m_systemUpTimeLabel = new QLabel("-", this);
    upTimeLayout->addWidget(m_systemUpTimeLabel);
    upTimeLayout->addStretch();
    osLayout->addLayout(upTimeLayout);

    m_osGroup->setLayout(osLayout);
    mainLayout->addWidget(m_osGroup);

    // ===== 2. 硬件信息组 =====
    m_hardwareGroup = new QGroupBox("硬件信息", this);
    QVBoxLayout* hardwareLayout = new QVBoxLayout(m_hardwareGroup);
    hardwareLayout->setSpacing(10);

    // CPU信息
    QHBoxLayout* cpuInfoLayout = new QHBoxLayout();
    cpuInfoLayout->addWidget(new QLabel("CPU信息：", this));
    m_cpuInfoLabel = new QLabel("-", this);
    cpuInfoLayout->addWidget(m_cpuInfoLabel);
    cpuInfoLayout->addStretch();
    hardwareLayout->addLayout(cpuInfoLayout);

    // CPU核心数
    QHBoxLayout* cpuCoresLayout = new QHBoxLayout();
    cpuCoresLayout->addWidget(new QLabel("CPU核心数：", this));
    m_cpuCoresLabel = new QLabel("-", this);
    cpuCoresLayout->addWidget(m_cpuCoresLabel);
    cpuCoresLayout->addStretch();
    hardwareLayout->addLayout(cpuCoresLayout);

    m_hardwareGroup->setLayout(hardwareLayout);
    mainLayout->addWidget(m_hardwareGroup);

    // ===== 3. 内存信息组 =====
    m_memoryGroup = new QGroupBox("内存信息", this);
    QVBoxLayout* memoryLayout = new QVBoxLayout(m_memoryGroup);
    memoryLayout->setSpacing(10);

    // 总物理内存
    QHBoxLayout* totalMemLayout = new QHBoxLayout();
    totalMemLayout->addWidget(new QLabel("总物理内存：", this));
    m_totalMemoryLabel = new QLabel("-", this);
    totalMemLayout->addWidget(m_totalMemoryLabel);
    totalMemLayout->addStretch();
    memoryLayout->addLayout(totalMemLayout);

    // 可用物理内存
    QHBoxLayout* availableMemLayout = new QHBoxLayout();
    availableMemLayout->addWidget(new QLabel("可用物理内存：", this));
    m_availableMemoryLabel = new QLabel("-", this);
    availableMemLayout->addWidget(m_availableMemoryLabel);
    availableMemLayout->addStretch();
    memoryLayout->addLayout(availableMemLayout);

    // 内存使用率
    QHBoxLayout* usageMemLayout = new QHBoxLayout();
    usageMemLayout->addWidget(new QLabel("内存使用率：", this));
    m_memoryUsageLabel = new QLabel("-", this);
    usageMemLayout->addWidget(m_memoryUsageLabel);
    usageMemLayout->addStretch();
    memoryLayout->addLayout(usageMemLayout);

    m_memoryGroup->setLayout(memoryLayout);
    mainLayout->addWidget(m_memoryGroup);

    // ===== 刷新按钮 =====
    m_refreshBtn = new QPushButton("刷新系统信息", this);
    mainLayout->addWidget(m_refreshBtn);

    // 底部拉伸（避免控件占满整个窗口）
    mainLayout->addStretch();

    setLayout(mainLayout);
}

// 刷新系统信息
void SystemInfoWidget::refreshSystemInfo() {
    // 从DataManager获取系统信息
    const SystemInfo& sysInfo = DataManager::GetInstance().GetSystemInfo();

    // ===== 更新操作系统信息 =====
    m_osVersionLabel->setText(QString::fromStdWString(sysInfo.osVersion));
    m_hostNameLabel->setText(QString::fromStdWString(sysInfo.hostName));
    m_userNameLabel->setText(QString::fromStdWString(sysInfo.userName));
    m_systemUpTimeLabel->setText(QString::fromStdWString(sysInfo.systemUpTime));

    // ===== 更新硬件信息 =====
    m_cpuInfoLabel->setText(QString::fromStdWString(sysInfo.cpuInfo));
    m_cpuCoresLabel->setText(QString::number(sysInfo.cpuCores));

    // ===== 更新内存信息 =====
    // 总内存（字节→GB）
    m_totalMemoryLabel->setText(bytesToGB(sysInfo.totalPhysicalMemory));
    // 可用内存（字节→GB）
    m_availableMemoryLabel->setText(bytesToGB(sysInfo.availablePhysicalMemory));
    // 内存使用率（计算百分比）
    if (sysInfo.totalPhysicalMemory > 0) {
        double usedPercent = 100.0 - (
            static_cast<double>(sysInfo.availablePhysicalMemory) / sysInfo.totalPhysicalMemory * 100.0
            );
        m_memoryUsageLabel->setText(QString::number(usedPercent, 'f', 1) + " %");
    }
    else {
        m_memoryUsageLabel->setText("未知");
    }
}

// 刷新按钮点击事件
void SystemInfoWidget::onRefreshClicked() {
    // 先刷新DataManager的数据
    DataManager::GetInstance().ManualRefresh();
    // 再更新UI
    refreshSystemInfo();
}