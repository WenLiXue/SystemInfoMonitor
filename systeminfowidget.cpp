#include "systeminfowidget.h"
#include "ui_systeminfowidget.h"
#include <QDateTime>
#include <sstream>

// �������������ֽ�ת��ΪGB������2λС����
QString bytesToGB(ULONGLONG bytes) {
    double gb = static_cast<double>(bytes) / (1024 * 1024 * 1024);
    return QString::number(gb, 'f', 2) + " GB";
}

SystemInfoWidget::SystemInfoWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SystemInfoWidget),
    m_dataManager(DataManager::GetInstance()) // ��ȡDataManager����
{
    ui->setupUi(this);
    SystemInfoWidget::initUI();       // ��ʼ��UI���
    refreshSystemInfo(); // ��ʼ��������

    // ��ˢ�°�ť�¼�
    connect(m_refreshBtn, &QPushButton::clicked, this, &SystemInfoWidget::onRefreshClicked);
}

SystemInfoWidget::~SystemInfoWidget() {
    delete ui;
}

// ��ʼ��UI���
void SystemInfoWidget::initUI() {
    // ===== ������ =====
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20); // �����

    // ===== 1. ����ϵͳ��Ϣ�� =====
    m_osGroup = new QGroupBox("����ϵͳ��Ϣ", this);
    QVBoxLayout* osLayout = new QVBoxLayout(m_osGroup);
    osLayout->setSpacing(10); // ����

    // ����ϵͳ�汾
    QHBoxLayout* osVersionLayout = new QHBoxLayout();
    osVersionLayout->addWidget(new QLabel("����ϵͳ�汾��", this));
    m_osVersionLabel = new QLabel("-", this); // ��ʼռλ��
    osVersionLayout->addWidget(m_osVersionLabel);
    osVersionLayout->addStretch(); // �Ҷ�������
    osLayout->addLayout(osVersionLayout);

    // ������
    QHBoxLayout* hostNameLayout = new QHBoxLayout();
    hostNameLayout->addWidget(new QLabel("��������", this));
    m_hostNameLabel = new QLabel("-", this);
    hostNameLayout->addWidget(m_hostNameLabel);
    hostNameLayout->addStretch();
    osLayout->addLayout(hostNameLayout);

    // �û���
    QHBoxLayout* userNameLayout = new QHBoxLayout();
    userNameLayout->addWidget(new QLabel("��ǰ�û���", this));
    m_userNameLabel = new QLabel("-", this);
    userNameLayout->addWidget(m_userNameLabel);
    userNameLayout->addStretch();
    osLayout->addLayout(userNameLayout);

    // ϵͳ����ʱ��
    QHBoxLayout* upTimeLayout = new QHBoxLayout();
    upTimeLayout->addWidget(new QLabel("ϵͳ����ʱ�䣺", this));
    m_systemUpTimeLabel = new QLabel("-", this);
    upTimeLayout->addWidget(m_systemUpTimeLabel);
    upTimeLayout->addStretch();
    osLayout->addLayout(upTimeLayout);

    m_osGroup->setLayout(osLayout);
    mainLayout->addWidget(m_osGroup);

    // ===== 2. Ӳ����Ϣ�� =====
    m_hardwareGroup = new QGroupBox("Ӳ����Ϣ", this);
    QVBoxLayout* hardwareLayout = new QVBoxLayout(m_hardwareGroup);
    hardwareLayout->setSpacing(10);

    // CPU��Ϣ
    QHBoxLayout* cpuInfoLayout = new QHBoxLayout();
    cpuInfoLayout->addWidget(new QLabel("CPU��Ϣ��", this));
    m_cpuInfoLabel = new QLabel("-", this);
    cpuInfoLayout->addWidget(m_cpuInfoLabel);
    cpuInfoLayout->addStretch();
    hardwareLayout->addLayout(cpuInfoLayout);

    // CPU������
    QHBoxLayout* cpuCoresLayout = new QHBoxLayout();
    cpuCoresLayout->addWidget(new QLabel("CPU��������", this));
    m_cpuCoresLabel = new QLabel("-", this);
    cpuCoresLayout->addWidget(m_cpuCoresLabel);
    cpuCoresLayout->addStretch();
    hardwareLayout->addLayout(cpuCoresLayout);

    m_hardwareGroup->setLayout(hardwareLayout);
    mainLayout->addWidget(m_hardwareGroup);

    // ===== 3. �ڴ���Ϣ�� =====
    m_memoryGroup = new QGroupBox("�ڴ���Ϣ", this);
    QVBoxLayout* memoryLayout = new QVBoxLayout(m_memoryGroup);
    memoryLayout->setSpacing(10);

    // �������ڴ�
    QHBoxLayout* totalMemLayout = new QHBoxLayout();
    totalMemLayout->addWidget(new QLabel("�������ڴ棺", this));
    m_totalMemoryLabel = new QLabel("-", this);
    totalMemLayout->addWidget(m_totalMemoryLabel);
    totalMemLayout->addStretch();
    memoryLayout->addLayout(totalMemLayout);

    // ���������ڴ�
    QHBoxLayout* availableMemLayout = new QHBoxLayout();
    availableMemLayout->addWidget(new QLabel("���������ڴ棺", this));
    m_availableMemoryLabel = new QLabel("-", this);
    availableMemLayout->addWidget(m_availableMemoryLabel);
    availableMemLayout->addStretch();
    memoryLayout->addLayout(availableMemLayout);

    // �ڴ�ʹ����
    QHBoxLayout* usageMemLayout = new QHBoxLayout();
    usageMemLayout->addWidget(new QLabel("�ڴ�ʹ���ʣ�", this));
    m_memoryUsageLabel = new QLabel("-", this);
    usageMemLayout->addWidget(m_memoryUsageLabel);
    usageMemLayout->addStretch();
    memoryLayout->addLayout(usageMemLayout);

    m_memoryGroup->setLayout(memoryLayout);
    mainLayout->addWidget(m_memoryGroup);

    // ===== ˢ�°�ť =====
    m_refreshBtn = new QPushButton("ˢ��ϵͳ��Ϣ", this);
    mainLayout->addWidget(m_refreshBtn);

    // �ײ����죨����ؼ�ռ���������ڣ�
    mainLayout->addStretch();

    setLayout(mainLayout);
}

// ˢ��ϵͳ��Ϣ
void SystemInfoWidget::refreshSystemInfo() {
    // ��DataManager��ȡϵͳ��Ϣ
    const SystemInfo& sysInfo = DataManager::GetInstance().GetSystemInfo();

    // ===== ���²���ϵͳ��Ϣ =====
    m_osVersionLabel->setText(QString::fromStdWString(sysInfo.osVersion));
    m_hostNameLabel->setText(QString::fromStdWString(sysInfo.hostName));
    m_userNameLabel->setText(QString::fromStdWString(sysInfo.userName));
    m_systemUpTimeLabel->setText(QString::fromStdWString(sysInfo.systemUpTime));

    // ===== ����Ӳ����Ϣ =====
    m_cpuInfoLabel->setText(QString::fromStdWString(sysInfo.cpuInfo));
    m_cpuCoresLabel->setText(QString::number(sysInfo.cpuCores));

    // ===== �����ڴ���Ϣ =====
    // ���ڴ棨�ֽڡ�GB��
    m_totalMemoryLabel->setText(bytesToGB(sysInfo.totalPhysicalMemory));
    // �����ڴ棨�ֽڡ�GB��
    m_availableMemoryLabel->setText(bytesToGB(sysInfo.availablePhysicalMemory));
    // �ڴ�ʹ���ʣ�����ٷֱȣ�
    if (sysInfo.totalPhysicalMemory > 0) {
        double usedPercent = 100.0 - (
            static_cast<double>(sysInfo.availablePhysicalMemory) / sysInfo.totalPhysicalMemory * 100.0
            );
        m_memoryUsageLabel->setText(QString::number(usedPercent, 'f', 1) + " %");
    }
    else {
        m_memoryUsageLabel->setText("δ֪");
    }
}

// ˢ�°�ť����¼�
void SystemInfoWidget::onRefreshClicked() {
    // ��ˢ��DataManager������
    DataManager::GetInstance().ManualRefresh();
    // �ٸ���UI
    refreshSystemInfo();
}