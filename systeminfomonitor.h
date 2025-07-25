#ifndef SYSTEMINFOMONITOR_H
#define SYSTEMINFOMONITOR_H

#include <QMainWindow>
#include "ui_systeminfomonitor.h" // 包含UI头文件
#include "datamanager.h" // 包含DataManager头文件
#include "processwidget.h"
#include "systeminfowidget.h"
#include"NetworkConnectionWidget.h"
#include "servicewidget.h"
#include "sessionwidget.h" // 包含会话Widget头文件
class SystemInfoMonitor : public QMainWindow {
    Q_OBJECT

public:
    SystemInfoMonitor(QWidget *parent = nullptr);
    ~SystemInfoMonitor();

private slots:
    void onTabChanged(int index); // 标签页切换事件处理

private:
    Ui::SystemInfoMonitorClass ui;

};

#endif // SYSTEMINFOMONITOR_H