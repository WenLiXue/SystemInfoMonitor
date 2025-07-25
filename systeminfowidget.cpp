#include "systeminfowidget.h"
#include "ui_systeminfowidget.h"
#include <QDateTime>
#include <sstream>

// 辅助函数：字节转GB
QString bytesToGB(ULONGLONG bytes) {
    double gb = static_cast<double>(bytes) / (1024 * 1024 * 1024);
    return QString::number(gb, 'f', 2) + " GB";
}

SystemInfoWidget::SystemInfoWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SystemInfoWidget),
    m_dataManager(DataManager::GetInstance()),
    m_firstLoad(true)
{
    ui->setupUi(this);
    initUI();
	
    // 初始化自动刷新定时器（1秒一次）
    m_autoRefreshTimer = new QTimer(this);
    m_autoRefreshTimer->setInterval(1000); // 1000ms = 1秒
    connect(m_autoRefreshTimer, &QTimer::timeout, this, &SystemInfoWidget::onAutoRefresh);

    // 绑定手动刷新按钮
    connect(m_refreshBtn, &QPushButton::clicked, this, &SystemInfoWidget::onRefreshClicked);


}
// 新增：启动自动刷新（供主窗口调用）
void SystemInfoWidget::startAutoRefresh() {
    if (!m_autoRefreshTimer->isActive()) {
        m_autoRefreshTimer->start();
    }
}

// 新增：停止自动刷新（供主窗口调用）
void SystemInfoWidget::stopAutoRefresh() {
    if (m_autoRefreshTimer->isActive()) {
        m_autoRefreshTimer->stop();
    }
}
SystemInfoWidget::~SystemInfoWidget() {
    delete ui;
}

// 初始化UI组件（保持不变）
void SystemInfoWidget::initUI() {
    // ===== 主布局 =====
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);

    // ===== 1. 操作系统信息组 =====
    m_osGroup = new QGroupBox("操作系统信息", this);
    QVBoxLayout* osLayout = new QVBoxLayout(m_osGroup);
    osLayout->setSpacing(10);

    QHBoxLayout* osVersionLayout = new QHBoxLayout();
    osVersionLayout->addWidget(new QLabel("操作系统版本：", this));
    m_osVersionLabel = new QLabel("-", this);
    osVersionLayout->addWidget(m_osVersionLabel);
    osVersionLayout->addStretch();
    osLayout->addLayout(osVersionLayout);

    QHBoxLayout* hostNameLayout = new QHBoxLayout();
    hostNameLayout->addWidget(new QLabel("主机名：", this));
    m_hostNameLabel = new QLabel("-", this);
    hostNameLayout->addWidget(m_hostNameLabel);
    hostNameLayout->addStretch();
    osLayout->addLayout(hostNameLayout);

    QHBoxLayout* userNameLayout = new QHBoxLayout();
    userNameLayout->addWidget(new QLabel("当前用户：", this));
    m_userNameLabel = new QLabel("-", this);
    userNameLayout->addWidget(m_userNameLabel);
    userNameLayout->addStretch();
    osLayout->addLayout(userNameLayout);

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

    QHBoxLayout* cpuInfoLayout = new QHBoxLayout();
    cpuInfoLayout->addWidget(new QLabel("CPU信息：", this));
    m_cpuInfoLabel = new QLabel("-", this);
    cpuInfoLayout->addWidget(m_cpuInfoLabel);
    cpuInfoLayout->addStretch();
    hardwareLayout->addLayout(cpuInfoLayout);

    QHBoxLayout* cpuCoresLayout = new QHBoxLayout();
    cpuCoresLayout->addWidget(new QLabel("CPU核心数：", this));
    m_cpuCoresLabel = new QLabel("-", this);
    cpuCoresLayout->addWidget(m_cpuCoresLabel);
    cpuCoresLayout->addStretch();
    hardwareLayout->addLayout(cpuCoresLayout);

    QHBoxLayout* cpuUsageLayout = new QHBoxLayout();
    cpuUsageLayout->addWidget(new QLabel("CPU使用率：", this));
    m_cpuUsageLabel = new QLabel("-", this);
    cpuUsageLayout->addWidget(m_cpuUsageLabel);
    m_cpuUsageBar = new QProgressBar(this);
    m_cpuUsageBar->setRange(0, 100);
    m_cpuUsageBar->setTextVisible(false);
    cpuUsageLayout->addWidget(m_cpuUsageBar);
    hardwareLayout->addLayout(cpuUsageLayout);

    m_hardwareGroup->setLayout(hardwareLayout);
    mainLayout->addWidget(m_hardwareGroup);

    // ===== 3. 内存信息组 =====
    m_memoryGroup = new QGroupBox("内存信息", this);
    QVBoxLayout* memoryLayout = new QVBoxLayout(m_memoryGroup);
    memoryLayout->setSpacing(10);

    QHBoxLayout* totalMemLayout = new QHBoxLayout();
    totalMemLayout->addWidget(new QLabel("总物理内存：", this));
    m_totalMemoryLabel = new QLabel("-", this);
    totalMemLayout->addWidget(m_totalMemoryLabel);
    totalMemLayout->addStretch();
    memoryLayout->addLayout(totalMemLayout);

    QHBoxLayout* availableMemLayout = new QHBoxLayout();
    availableMemLayout->addWidget(new QLabel("可用物理内存：", this));
    m_availableMemoryLabel = new QLabel("-", this);
    availableMemLayout->addWidget(m_availableMemoryLabel);
    availableMemLayout->addStretch();
    memoryLayout->addLayout(availableMemLayout);

    QHBoxLayout* usageMemLayout = new QHBoxLayout();
    usageMemLayout->addWidget(new QLabel("内存使用率：", this));
    m_memoryUsageLabel = new QLabel("-", this);
    usageMemLayout->addWidget(m_memoryUsageLabel);
    m_memoryUsageBar = new QProgressBar(this);
    m_memoryUsageBar->setRange(0, 100);
    m_memoryUsageBar->setTextVisible(false);
    usageMemLayout->addWidget(m_memoryUsageBar);
    memoryLayout->addLayout(usageMemLayout);

    m_memoryGroup->setLayout(memoryLayout);
    mainLayout->addWidget(m_memoryGroup);

    // ===== 刷新按钮 =====
    m_refreshBtn = new QPushButton("刷新系统信息", this);
    mainLayout->addWidget(m_refreshBtn);

    mainLayout->addStretch();
    setLayout(mainLayout);
}

// 刷新系统信息（核心数据更新逻辑）
void SystemInfoWidget::refreshSystemInfo() {
    // 强制刷新数据管理器中的最新数据
    m_dataManager.ManualRefresh();
    const SystemInfo& sysInfo = m_dataManager.GetSystemInfo();

    // 更新操作系统信息
    m_osVersionLabel->setText(QString::fromStdWString(sysInfo.osVersion));
    m_hostNameLabel->setText(QString::fromStdWString(sysInfo.hostName));
    m_userNameLabel->setText(QString::fromStdWString(sysInfo.userName));
    m_systemUpTimeLabel->setText(QString::fromStdWString(sysInfo.systemUpTime));

    // 更新CPU信息
    m_cpuInfoLabel->setText(QString::fromStdWString(sysInfo.cpuInfo));
    m_cpuCoresLabel->setText(QString::number(sysInfo.cpuCores));

    // 更新CPU使用率
    double cpuUsage = m_dataManager.GetCpuUsage();
    m_cpuUsageLabel->setText(QString::number(cpuUsage, 'f', 1) + " %");
    m_cpuUsageBar->setValue(static_cast<int>(cpuUsage));
    // 设置CPU进度条颜色
    if (cpuUsage > 80) {
        m_cpuUsageBar->setStyleSheet("QProgressBar {border: 1px solid grey; border-radius: 3px;}"
            "QProgressBar::chunk {background-color: #FF5252;}");
    }
    else if (cpuUsage > 50) {
        m_cpuUsageBar->setStyleSheet("QProgressBar {border: 1px solid grey; border-radius: 3px;}"
            "QProgressBar::chunk {background-color: #FFC107;}");
    }
    else {
        m_cpuUsageBar->setStyleSheet("QProgressBar {border: 1px solid grey; border-radius: 3px;}"
            "QProgressBar::chunk {background-color: #4CAF50;}");
    }

    // 更新内存信息
    m_totalMemoryLabel->setText(bytesToGB(sysInfo.totalPhysicalMemory));
    m_availableMemoryLabel->setText(bytesToGB(sysInfo.availablePhysicalMemory));
    // 计算内存使用率
    if (sysInfo.totalPhysicalMemory > 0) {
        double usedPercent = 100.0 - (
            static_cast<double>(sysInfo.availablePhysicalMemory) / sysInfo.totalPhysicalMemory * 100.0
            );
        m_memoryUsageLabel->setText(QString::number(usedPercent, 'f', 1) + " %");
        m_memoryUsageBar->setValue(static_cast<int>(usedPercent));
        // 设置内存进度条颜色
        if (usedPercent > 80) {
            m_memoryUsageBar->setStyleSheet("QProgressBar {border: 1px solid grey; border-radius: 3px;}"
                "QProgressBar::chunk {background-color: #FF5252;}");
        }
        else if (usedPercent > 50) {
            m_memoryUsageBar->setStyleSheet("QProgressBar {border: 1px solid grey; border-radius: 3px;}"
                "QProgressBar::chunk {background-color: #FFC107;}");
        }
        else {
            m_memoryUsageBar->setStyleSheet("QProgressBar {border: 1px solid grey; border-radius: 3px;}"
                "QProgressBar::chunk {background-color: #4CAF50;}");
        }
    }
    else {
        m_memoryUsageLabel->setText("未知");
        m_memoryUsageBar->setValue(0);
    }

    m_firstLoad = false; // 标记首次加载完成
}

// 手动刷新按钮点击事件
void SystemInfoWidget::onRefreshClicked() {
    refreshSystemInfo();
}

// 定时器自动刷新事件（1秒一次）
void SystemInfoWidget::onAutoRefresh() {
    // 仅在可见时刷新，节省资源
    if (isVisible()) {
        refreshSystemInfo();
    }
}

// 监听自身显示状态变化（标签页切换时触发）
void SystemInfoWidget::onTabVisibleChanged(bool visible) {
    if (visible) {
        // 当标签页切换到当前页时，立即刷新并启动自动刷新
        refreshSystemInfo();
        if (!m_autoRefreshTimer->isActive()) {
            m_autoRefreshTimer->start(); // 启动定时器（1秒一次）
        }
    }
    else {
        // 当标签页切换走时，暂停自动刷新
        if (m_autoRefreshTimer->isActive()) {
            m_autoRefreshTimer->stop();
        }
    }
}

// 外部调用：设置为当前标签页（与主窗口配合）
void SystemInfoWidget::setAsCurrentTab() {
    // 强制刷新并激活自动刷新
    refreshSystemInfo(); // 切换到当前页时立即刷新
    startAutoRefresh();  // 启动自动刷新
}