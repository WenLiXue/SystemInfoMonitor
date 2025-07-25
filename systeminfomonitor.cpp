#include "systeminfomonitor.h"

SystemInfoMonitor::SystemInfoMonitor(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    qDebug()<<ui.tabWidget->size();
	ui.tabWidget->addTab(new ProcessWidget(this), "Processes");
	ui.tabWidget->addTab(new SystemInfoWidget(this), "SystemInfo");


}

SystemInfoMonitor::~SystemInfoMonitor()
{}

