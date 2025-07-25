// processwidget.h
#ifndef PROCESSWIDGET_H
#define PROCESSWIDGET_H

#include <QWidget>
#include<QMessageBox>
#include "datamanager.h"  // ����DataManagerͷ�ļ�

namespace Ui {
    class ProcessWidget;
}

class ProcessWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProcessWidget(QWidget* parent = nullptr);  // �Ƴ�DataManager����
    ~ProcessWidget();

    // ˢ�±������
    void refreshTable();

private slots:
    void on_refreshButton_clicked();  // ˢ�°�ť����¼�

    void on_btnTerminateProcess_clicked();

private:
    Ui::ProcessWidget* ui;
    // ����Ҫ����DataManagerָ�룬ֱ��ͨ����������
};

#endif // PROCESSWIDGET_H