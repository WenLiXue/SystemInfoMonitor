// processwidget.cpp
#include "processwidget.h"
#include "ui_processwidget.h"
#include <QHeaderView>
#include <QDateTime>
#include <qstandarditemmodel.h>
#include <warning.h>

// 辅助函数：将FILETIME转换为秒数
double FileTimeToSeconds(const FILETIME& ft) {
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return static_cast<double>(ull.QuadPart) / 10000000.0;
}

ProcessWidget::~ProcessWidget() {
    delete ui;
}
ProcessWidget::ProcessWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ProcessWidget)
{
    ui->setupUi(this);

    // ===== 关键：添加顶层布局 =====
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 去除边缘间距，避免留白
    mainLayout->setSpacing(0); // 控件间无间距

    // 1. 初始化表格模型
    QStandardItemModel* model = new QStandardItemModel(0, 9, this);
    model->setHorizontalHeaderLabels({
        "PID", "PPID", "进程名", "可执行路径", "命令行",
        "创建时间", "内存(KB)", "内核时间(s)", "用户时间(s)"
        });

    // 2. 配置表格视图
    ui->tableView->setModel(model);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    // 设置表格大小策略为“扩展”，确保填满布局空间
    ui->tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 3. 处理“终止进程”相关控件（假设ui中包含按钮和输入框）
    // 创建一个水平布局存放“刷新按钮”“终止按钮”和输入框
    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->addWidget(ui->btnFresh); // 刷新按钮
    controlLayout->addWidget(ui->processInfo); // 进程输入框（QLineEdit）
    controlLayout->addWidget(ui->btnTerminateProcess); // 终止按钮

    // 4. 将所有控件添加到顶层布局
    mainLayout->addLayout(controlLayout); // 添加控制栏（按钮+输入框）
    mainLayout->addWidget(ui->tableView); // 添加表格
    mainLayout->addWidget(ui->bottomState); // 添加状态栏（假设是QLabel）

    // 5. 连接刷新按钮事件
    connect(ui->btnFresh, &QPushButton::clicked, this, &ProcessWidget::on_refreshButton_clicked);

    // 初始加载数据
    refreshTable();
}



// 刷新表格数据（从DataManager单例获取数据）
void ProcessWidget::refreshTable() {
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (!model) return;

    // 清空表格
    model->removeRows(0, model->rowCount());

    // 通过单例获取进程数据
    const std::vector<ProcessInfo>& processes = DataManager::GetInstance().GetProcesses();
    if (processes.empty()) {
        ui->bottomState->setText("无进程数据");
        return;
    }

    // 填充表格（与之前相同）
    for (const auto& proc : processes) {
        QList<QStandardItem*> items;
        items << new QStandardItem(QString::number(proc.pid))
            << new QStandardItem(QString::number(proc.parentPid))
            << new QStandardItem(QString::fromStdWString(proc.processName))
            << new QStandardItem(QString::fromStdWString(proc.executablePath))
            << new QStandardItem(QString::fromStdWString(proc.commandLine).left(100))
            << new QStandardItem(QString::fromStdWString(proc.creationTime))
            << new QStandardItem(QString::number(proc.memoryUsage / 1024.0, 'f', 1))
            << new QStandardItem(QString::number(FileTimeToSeconds(proc.kernelTime), 'f', 2))
            << new QStandardItem(QString::number(FileTimeToSeconds(proc.userTime), 'f', 2));

        for (auto* item : items) {
            item->setEditable(false);
        }
        model->appendRow(items);
    }

    // 更新状态栏
    ui->bottomState->setText(QString("共 %1 个进程，最后更新于 %2")
        .arg(processes.size())
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
}

void ProcessWidget::on_btnTerminateProcess_clicked()
{
    // 确保DataManager已初始化
    DataManager::InitGlobalInstance();

    // 获取用户输入
    QString input = ui->processInfo->text().trimmed();  // 假设processInfo是QLineEdit
    
    // 检查输入是否为空
    if (input.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入进程名或PID");
        return;
    }

    // 判断输入是PID还是进程名
    bool isPidValid;
    DWORD pid = input.toULong(&isPidValid, 10);

    QString processName;
    QString displayText;

    if (isPidValid) {
        // 用户输入的是PID
        processName = "";  // PID方式不需要进程名
        displayText = QString("PID: %1").arg(pid);
    }
    else {
        // 用户输入的是进程名
        processName = input;
        displayText = QString("进程名: %1").arg(processName);
    }

    // 确认对话框
    int ret = QMessageBox::question(this, "确认终止",
        QString("确定要终止以下进程吗？\n%1").arg(displayText),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) {
        return;
    }

    // 执行终止操作
    bool success = false;
    if (isPidValid && pid > 0) {
        success = DataManager::GetInstance().TerminateTargetProcessByPid(pid);
    }
    else {
        success = DataManager::GetInstance().TerminateTargetProcessByName(processName.toStdString());
    }

    // 显示结果
    if (success) {
        QMessageBox::information(this, "成功", "进程已成功终止");
        // 刷新表格
        refreshTable();
    }
    else {
        QMessageBox::critical(this, "失败", "无法终止进程，可能权限不足、进程不存在或名称不正确");
    }
}

// 刷新按钮点击事件处理
void ProcessWidget::on_refreshButton_clicked() {
    // 可以先调用数据更新逻辑（如果需要）
    DataManager::GetInstance().ManualRefresh();  // 假设有刷新数据的方法
    refreshTable();
}