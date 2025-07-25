// networkconnectionwidget.cpp
#include "networkconnectionwidget.h"

// 协议类型转换（数字转文本）
QString protocolToString(int protocol) {
    switch (protocol) {
    case IPPROTO_TCP: return "TCP";
    case IPPROTO_UDP: return "UDP";
    default: return QString("未知(%1)").arg(protocol);
    }
}

NetworkConnectionWidget::NetworkConnectionWidget(QWidget* parent) :
    QWidget(parent),
    m_tableView(nullptr),
    m_model(nullptr),
    m_refreshBtn(nullptr),
    m_statusLabel(nullptr),
    m_filterCombo(nullptr)
{
    initUI();
    onRefreshButtonClicked(); // 初始化时加载数据
}

NetworkConnectionWidget::~NetworkConnectionWidget() {
    // Qt自动管理子控件内存
}
// 添加 NetworkConnectionDelegate 类（用于自定义渲染）
class NetworkConnectionDelegate : public QStyledItemDelegate {
public:
    explicit NetworkConnectionDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        // 获取协议类型（第一列数据）
        if (index.column() == 0) {
            QString protocol = index.model()->data(index, Qt::DisplayRole).toString();
            QStyleOptionViewItem opt = option;

            // 根据协议设置行背景色
            if (protocol == "TCP") {
                opt.backgroundBrush = QBrush(QColor(220, 230, 241)); // 浅蓝色
            }
            else if (protocol == "UDP") {
                opt.backgroundBrush = QBrush(QColor(232, 245, 233)); // 浅绿色
            }

            // 绘制行背景
            painter->fillRect(opt.rect, opt.backgroundBrush);
        }

        // 调用基类绘制其他部分
        QStyledItemDelegate::paint(painter, option, index);
    }
};
void NetworkConnectionWidget::initUI() {
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 顶部控制栏
    QHBoxLayout* controlLayout = new QHBoxLayout();

    // 刷新按钮
    m_refreshBtn = new QPushButton("刷新网络连接", this);
    connect(m_refreshBtn, &QPushButton::clicked, this, &NetworkConnectionWidget::onRefreshButtonClicked);
    controlLayout->addWidget(m_refreshBtn);

    // 过滤下拉框
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItems({ "全部连接", "TCP连接", "UDP连接" });
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &NetworkConnectionWidget::onFilterChanged);
    controlLayout->addWidget(m_filterCombo);

    controlLayout->addStretch(); // 填充剩余空间

    // 初始化表格模型
    m_model = new QStandardItemModel(0, 6, this);
    m_model->setHorizontalHeaderLabels({
        "协议", "本地地址", "远程地址", "状态", "PID", "进程名称"
        });

    // 初始化表格视图
    m_tableView = new QTableView(this);
    m_tableView->setItemDelegate(new NetworkConnectionDelegate(this));

    m_tableView->setModel(m_model);
    m_tableView->setSortingEnabled(true); // 允许排序
    m_tableView->setAlternatingRowColors(true); // 隔行变色

    // 设置列宽策略
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setStretchLastSection(true);

    // 设置表格大小策略
    m_tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 设置垂直滚动条策略
    m_tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 状态栏
    m_statusLabel = new QLabel("就绪", this);

    // 组装布局
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(m_tableView);
    mainLayout->addWidget(m_statusLabel);

    setLayout(mainLayout);
}

void NetworkConnectionWidget::refreshTable() {
    // 清空表格
    m_model->removeRows(0, m_model->rowCount());

    // 获取网络连接数据
    const std::vector<ConnectionInfo>& connections = DataManager::GetInstance().GetConnections();
    if (connections.empty()) {
        updateStatus("未发现网络连接");
        return;
    }

    // 获取进程列表（用于关联PID到进程名）
    const std::vector<ProcessInfo>& processes = DataManager::GetInstance().GetProcesses();
    std::map<DWORD, std::wstring> pidToName;
    for (const auto& proc : processes) {
        pidToName[proc.pid] = proc.processName;
    }

    // 过滤选项
    int filterIndex = m_filterCombo->currentIndex();

    // 填充表格
    int displayedCount = 0;
    for (const auto& conn : connections) {
        // 应用过滤
        if (filterIndex == 1 && conn.protocol != IPPROTO_TCP) continue; // 只显示TCP
        if (filterIndex == 2 && conn.protocol != IPPROTO_UDP) continue; // 只显示UDP

        // 获取进程名
        QString processName = "[未知]";
        auto it = pidToName.find(conn.pid);
        if (it != pidToName.end()) {
            processName = QString::fromStdWString(it->second);
        }

        // 创建表格行（带格式设置）
        QList<QStandardItem*> items;

        // 1. 协议列（行颜色区分）
        QStandardItem* protoItem = new QStandardItem(protocolToString(conn.protocol));
        if (conn.protocol == IPPROTO_TCP) {
            protoItem->setBackground(QColor(220, 230, 241)); // 浅蓝色
        }
        else if (conn.protocol == IPPROTO_UDP) {
            protoItem->setBackground(QColor(232, 245, 233)); // 浅绿色
        }
        items << protoItem;

        // 2. 本地地址列（长地址截断+提示）
        QString localAddr = QString::fromStdWString(conn.localAddress);
        QStandardItem* localAddrItem = new QStandardItem(truncateAddress(localAddr,30));
        localAddrItem->setToolTip(localAddr); // 完整地址提示
        items << localAddrItem;

        // 3. 远程地址列（长地址截断+提示）
        QString remoteAddr = QString::fromStdWString(conn.remoteAddress);
        QStandardItem* remoteAddrItem = new QStandardItem(truncateAddress(remoteAddr,30));
        remoteAddrItem->setToolTip(remoteAddr); // 完整地址提示
        items << remoteAddrItem;

        // 4. 状态列（颜色标记）
        QStandardItem* stateItem = new QStandardItem(QString::fromStdWString(conn.state));
        QString state = QString::fromStdWString(conn.state);
        if (state == "ESTABLISHED") {
            stateItem->setForeground(QColor(0, 120, 215)); // 蓝色
        }
        else if (state == "LISTENING") {
            stateItem->setForeground(QColor(0, 177, 89)); // 绿色
        }
        else if (state == "CLOSE_WAIT") {
            stateItem->setForeground(QColor(247, 150, 70)); // 橙色
        }
        else if (state == "TIME_WAIT") {
            stateItem->setForeground(QColor(160, 80, 0)); // 棕色
        }
        else if (state == "CLOSED") {
            stateItem->setForeground(QColor(160, 160, 160)); // 灰色
        }
        else if (state == "SYN_SENT" || state == "SYN_RECV") {
            stateItem->setForeground(QColor(255, 59, 48)); // 红色
        }
        items << stateItem;

        // 5. PID列
        items << new QStandardItem(QString::number(conn.pid));

        // 6. 进程名列
        items << new QStandardItem(processName);

        // 设置所有单元格不可编辑
        for (auto* item : items) {
            item->setEditable(false);
        }

        // 添加到表格
        m_model->appendRow(items);
        displayedCount++;
    }

    // 更新状态栏
    updateStatus(QString("共 %1 个网络连接，显示 %2 个")
        .arg(connections.size())
        .arg(displayedCount));
}

// 辅助方法：截断过长的地址
QString NetworkConnectionWidget::truncateAddress(const QString& address, int maxLength=30) {
    if (address.length() <= maxLength) {
        return address;
    }

    // 尝试在端口号前截断（格式为 IP:PORT）
    int colonPos = address.lastIndexOf(':');
    if (colonPos > 0 && colonPos < address.length() - 5) {
        QString ipPart = address.left(colonPos);
        QString portPart = address.mid(colonPos);

        // 截断IP部分，保留端口
        if (ipPart.length() > maxLength - 5) {
            return ipPart.left(maxLength - 5) + "..." + portPart;
        }
    }

    // 默认截断方式
    return address.left(maxLength - 3) + "...";
}

void NetworkConnectionWidget::updateStatus(const QString& text) {
    m_statusLabel->setText(QString("%1，最后更新于 %2")
        .arg(text)
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
}

void NetworkConnectionWidget::onRefreshButtonClicked() {
    // 刷新数据前先停止自动刷新（如果有）
    // ...

    // 强制刷新数据管理器
    DataManager::GetInstance().ManualRefresh();

    // 刷新表格
    refreshTable();
}

void NetworkConnectionWidget::onFilterChanged(int index) {
    // 过滤条件变化时刷新表格
    refreshTable();
}