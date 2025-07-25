#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_systeminfomonitor.h"
#include"processwidget.h"
#include"systeminfowidget.h"
class SystemInfoMonitor : public QMainWindow
{
    Q_OBJECT

public:
    SystemInfoMonitor(QWidget *parent = nullptr);
    ~SystemInfoMonitor();

private:
    Ui::SystemInfoMonitorClass ui;
};

