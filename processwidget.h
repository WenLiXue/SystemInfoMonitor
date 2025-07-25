// processwidget.h
#ifndef PROCESSWIDGET_H
#define PROCESSWIDGET_H

#include <QWidget>
#include<QMessageBox>
#include "datamanager.h"  // 包含DataManager头文件

namespace Ui {
    class ProcessWidget;
}

class ProcessWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProcessWidget(QWidget* parent = nullptr);  // 移除DataManager参数
    ~ProcessWidget();

    // 刷新表格数据
    void refreshTable();

private slots:
    void on_refreshButton_clicked();  // 刷新按钮点击事件

    void on_btnTerminateProcess_clicked();

private:
    Ui::ProcessWidget* ui;
    // 不需要保存DataManager指针，直接通过单例访问
};

#endif // PROCESSWIDGET_H