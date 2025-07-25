// sessionwidget.cpp
#include "sessionwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <WtsApi32.h>
#pragma comment(lib, "WtsApi32.lib")

SessionWidget::SessionWidget(QWidget* parent) : QWidget(parent) {
    initUI();
    onRefreshButtonClicked(); // 初始加载
}

SessionWidget::~SessionWidget() {}

void SessionWidget::initUI() {
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 控制栏
    QHBoxLayout* controlLayout = new QHBoxLayout();
    m_refreshBtn = new QPushButton("刷新会话列表", this);
    connect(m_refreshBtn, &QPushButton::clicked, this, &SessionWidget::onRefreshButtonClicked);
    controlLayout->addWidget(m_refreshBtn);
    controlLayout->addStretch();

    // 表格模型
    m_model = new QStandardItemModel(0, 5, this);
    m_model->setHorizontalHeaderLabels({
        "会话ID", "用户名", "所属域", "登录时间", "状态"
        });

    // 表格视图
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setSortingEnabled(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setStretchLastSection(true);

    // 状态栏
    m_statusLabel = new QLabel("就绪", this);

    // 组装布局
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(m_tableView);
    mainLayout->addWidget(m_statusLabel);

    setLayout(mainLayout);
}

QString SessionWidget::sessionStateToString(WTS_CONNECTSTATE_CLASS state) {
    switch (state) {
    case WTSActive:       return "活跃";
    case WTSConnected:    return "已连接";
    case WTSConnectQuery: return "连接查询";
    case WTSShadow:       return "影子模式";
    case WTSDisconnected: return "已断开";
    case WTSIdle:         return "空闲";
    case WTSListen:       return "监听";
    case WTSReset:        return "重置中";
    case WTSDown:         return "已关闭";
    case WTSInit:         return "初始化中";
    default: return QString("未知(%1)").arg(state);
    }
}

void SessionWidget::refreshTable() {
    m_model->removeRows(0, m_model->rowCount());

    // 获取会话数据
    const std::vector<SessionInfo>& sessions = DataManager::GetInstance().GetSessions();
    if (sessions.empty()) {
        updateStatus("未发现登录会话");
        return;
    }

    // 填充表格
    for (const auto& session : sessions) {
        QList<QStandardItem*> items;

        // 1. 会话ID
        items << new QStandardItem(QString::number(session.sessionId));

        // 2. 用户名
        QStandardItem* userItem = new QStandardItem(QString::fromStdWString(session.userName));
        userItem->setToolTip(QString::fromStdWString(session.userName));
        items << userItem;

        // 3. 所属域
        QStandardItem* domainItem = new QStandardItem(QString::fromStdWString(session.domain));
        domainItem->setToolTip(QString::fromStdWString(session.domain));
        items << domainItem;

        // 4. 登录时间
        QStandardItem* timeItem = new QStandardItem(QString::fromStdWString(session.loginTime));
        timeItem->setToolTip(QString::fromStdWString(session.loginTime));
        items << timeItem;

        // 5. 状态（带颜色标记）
        QString stateStr = sessionStateToString(session.state);
        QStandardItem* stateItem = new QStandardItem(stateStr);

        // 根据状态设置颜色
        if (stateStr == "活跃" || stateStr == "已连接") {
            stateItem->setForeground(QColor(0, 177, 89)); // 绿色
        }
        else if (stateStr == "已断开" || stateStr == "已关闭") {
            stateItem->setForeground(QColor(160, 160, 160)); // 灰色
        }
        else if (stateStr.contains("中")) { // 进行中的状态
            stateItem->setForeground(QColor(247, 150, 70)); // 橙色
        }

        items << stateItem;

        // 设置单元格不可编辑
        for (auto* item : items) {
            item->setEditable(false);
        }

        m_model->appendRow(items);
    }

    // 更新状态栏
    updateStatus(QString("共 %1 个登录会话").arg(sessions.size()));
}

void SessionWidget::updateStatus(const QString& text) {
    m_statusLabel->setText(QString("%1，最后更新于 %2")
        .arg(text)
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
}

void SessionWidget::onRefreshButtonClicked() {
    DataManager::GetInstance().ManualRefresh(); // 刷新数据
    refreshTable(); // 刷新表格
}