#pragma execution_character_set("utf-8")
#include "systeminfomonitor.h"
#include"processwidget.h"
#include <QtWidgets/QApplication>
#include"DataManager.h"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SystemInfoMonitor window;
    window.show();
    return app.exec();
}
