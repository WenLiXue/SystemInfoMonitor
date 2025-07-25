#include "servicewidget.h"
#include <QDateTime>
#include <windows.h>
#include <winsvc.h>

// 链接WTSAPI32库（用于服务状态转换）
#pragma comment(lib, "advapi32.lib")

// 服务状态转换为字符串
QString ServiceWidget::serviceStatusToString(DWORD status) {
    switch (status) {
        case SERVICE_STOPPED:         return "已停止";
        case SERVICE_START_PENDING:   return "启动中";
        case SERVICE_STOP_PENDING:    return "停止中";
        case SERVICE_RUNNING:         return "运行中";
        case SERVICE_CONTINUE_PENDING:return "继续中";
        case SERVICE_PAUSE_PENDING:   return "暂停中";
        case SERVICE_PAUSED:          return "已暂停";
        default: return QString("未知(%1)").arg(status);
    }
}



ServiceWidget::ServiceWidget(QWidget *parent) :
    QWidget(parent),
    m_tableView(nullptr),
    m_model(nullptr),
    m_refreshBtn(nullptr),
    m_statusLabel(nullptr)
{
    initUI();
    onRefreshClicked(); // 初始加载数据
}

ServiceWidget::~ServiceWidget() {
    // Qt自动管理子控件内存
}

void ServiceWidget::initUI() {
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 顶部控制栏
    QHBoxLayout* controlLayout = new QHBoxLayout();
    
    // 刷新按钮
    m_refreshBtn = new QPushButton("刷新服务列表", this);
    connect(m_refreshBtn, &QPushButton::clicked, this, &ServiceWidget::onRefreshClicked);
    controlLayout->addWidget(m_refreshBtn);
    controlLayout->addStretch();

    // 表格模型
    m_model = new QStandardItemModel(0, 5, this);
    m_model->setHorizontalHeaderLabels({
        "服务名称", "显示名称", "状态", "启动类型", "二进制路径"
    });

    // 表格视图
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setSortingEnabled(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 状态栏
    m_statusLabel = new QLabel("就绪", this);

    // 组装布局
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(m_tableView);
    mainLayout->addWidget(m_statusLabel);

    setLayout(mainLayout);
}

void ServiceWidget::refreshTable() {
    // 清空表格
    m_model->removeRows(0, m_model->rowCount());

    // 获取服务数据
    const std::vector<ServiceInfo>& services = DataManager::GetInstance().GetServices();
    if (services.empty()) {
        updateStatus("未发现服务信息");
        return;
    }

    // 填充表格
    for (const auto& service : services) {
        QList<QStandardItem*> items;

        // 1. 服务名称
        items << new QStandardItem(QString::fromStdWString(service.serviceName));

        // 2. 显示名称
        QStandardItem* displayNameItem = new QStandardItem(QString::fromStdWString(service.displayName));
        displayNameItem->setToolTip(QString::fromStdWString(service.displayName)); // 完整名称提示
        items << displayNameItem;

        // 3. 状态（带颜色标记）
        QString statusStr = serviceStatusToString(service.status);
        QStandardItem* statusItem = new QStandardItem(statusStr);
        if (statusStr == "运行中") {
            statusItem->setForeground(QColor(0, 177, 89)); // 绿色
        } else if (statusStr == "已停止") {
            statusItem->setForeground(QColor(160, 160, 160)); // 灰色
        } else if (statusStr.contains("中")) { // 启动中/停止中等过渡状态
            statusItem->setForeground(QColor(247, 150, 70)); // 橙色
        }
        items << statusItem;

        // 4. 启动类型
        items << new QStandardItem(QString::fromStdWString(service.startTypeStr));
        // 5. 二进制路径（长路径截断）
        QString binPath = QString::fromStdWString(service.binaryPath);
        QStandardItem* pathItem = new QStandardItem(binPath.left(50) + "...");
        pathItem->setToolTip(binPath); // 完整路径提示
        items << pathItem;

        // 设置单元格不可编辑
        for (auto* item : items) {
            item->setEditable(false);
        }

        m_model->appendRow(items);
    }

    // 更新状态栏
    updateStatus(QString("共 %1 个服务").arg(services.size()));
}

void ServiceWidget::updateStatus(const QString& text) {
    m_statusLabel->setText(QString("%1，最后更新于 %2")
                          .arg(text)
                          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
}

void ServiceWidget::onRefreshClicked() {
    DataManager::GetInstance().ManualRefresh(); // 刷新数据
    refreshTable(); // 刷新表格
}