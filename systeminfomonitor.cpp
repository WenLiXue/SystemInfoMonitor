#include "systeminfomonitor.h"


SystemInfoMonitor::SystemInfoMonitor(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    qDebug() << ui.tabWidget->size();

    // ===== 关键：程序启动时初始化DataManager =====
    if (!DataManager::InitGlobalInstance()) {
        qCritical() << "DataManager初始化失败！";
        // 可根据需要弹出错误提示或退出程序
    }

    // 设置tabWidget填满主窗口
    //ui.tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 添加标签页（Widget通过GetInstance()获取已初始化的DataManager）
    ui.tabWidget->addTab(new ProcessWidget(this), "进程信息");
    ui.tabWidget->addTab(new SystemInfoWidget(this), "系统信息");
    ui.tabWidget->addTab(new NetworkConnectionWidget(this), "网络信息");
    ui.tabWidget->addTab(new ServiceWidget(this), "服务信息");
    ui.tabWidget->addTab(new SessionWidget(this), "登录会话"); // 添加会话页


    // 绑定标签页切换事件（控制自动刷新）
    connect(ui.tabWidget, &QTabWidget::currentChanged, this, &SystemInfoMonitor::onTabChanged);
}

SystemInfoMonitor::~SystemInfoMonitor()
{
  
}

// 新增标签页切换事件处理（控制SystemInfoWidget的自动刷新）
void SystemInfoMonitor::onTabChanged(int index) {
    QWidget* currentWidget = ui.tabWidget->widget(index);
    // 判断当前是否为系统信息页（需要先保存其指针）
    if (auto* sysWidget = qobject_cast<SystemInfoWidget*>(currentWidget)) {
        sysWidget->setAsCurrentTab(); // 切换到时启动刷新
    }
    else {
        // 切换到其他页时，停止系统信息页的自动刷新
        if (auto* sysWidget = qobject_cast<SystemInfoWidget*>(ui.tabWidget->widget(1))) {
            sysWidget->stopAutoRefresh();
        }
    }
}