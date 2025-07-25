#ifndef SYSTEMINFOWIDGET_H
#define SYSTEMINFOWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "datamanager.h"

namespace Ui {
    class SystemInfoWidget;
}

class SystemInfoWidget : public QWidget {
    Q_OBJECT

public:
    explicit SystemInfoWidget(QWidget* parent = nullptr);
    ~SystemInfoWidget();

    void initUI();

    // ˢ��ϵͳ��Ϣ
    void refreshSystemInfo();

private slots:
    // ˢ�°�ť����¼�
    void onRefreshClicked();

private:
    Ui::SystemInfoWidget* ui;

    // ���ݹ�������ͨ���������ʣ�
    DataManager& m_dataManager;

    // ===== ����ϵͳ��Ϣ�� =====
    QGroupBox* m_osGroup;           // ����ϵͳ��Ϣ�����
    QLabel* m_osVersionLabel;       // ����ϵͳ�汾��ǩ��ֵ��
    QLabel* m_hostNameLabel;        // ��������ǩ��ֵ��
    QLabel* m_userNameLabel;        // �û�����ǩ��ֵ��
    QLabel* m_systemUpTimeLabel;    // ϵͳ����ʱ���ǩ��ֵ��

    // ===== Ӳ����Ϣ�� =====
    QGroupBox* m_hardwareGroup;     // Ӳ����Ϣ�����
    QLabel* m_cpuInfoLabel;         // CPU��Ϣ��ǩ��ֵ��
    QLabel* m_cpuCoresLabel;        // CPU��������ǩ��ֵ��

    // ===== �ڴ���Ϣ�� =====
    QGroupBox* m_memoryGroup;       // �ڴ���Ϣ�����
    QLabel* m_totalMemoryLabel;     // �������ڴ��ǩ��ֵ��
    QLabel* m_availableMemoryLabel; // ���������ڴ��ǩ��ֵ��
    QLabel* m_memoryUsageLabel;     // �ڴ�ʹ���ʱ�ǩ��ֵ��

    // ˢ�°�ť
    QPushButton* m_refreshBtn;
};

#endif // SYSTEMINFOWIDGET_H