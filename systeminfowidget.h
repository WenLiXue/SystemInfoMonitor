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

    // 刷新系统信息
    void refreshSystemInfo();

private slots:
    // 刷新按钮点击事件
    void onRefreshClicked();

private:
    Ui::SystemInfoWidget* ui;

    // 数据管理器（通过单例访问）
    DataManager& m_dataManager;

    // ===== 操作系统信息组 =====
    QGroupBox* m_osGroup;           // 操作系统信息分组框
    QLabel* m_osVersionLabel;       // 操作系统版本标签（值）
    QLabel* m_hostNameLabel;        // 主机名标签（值）
    QLabel* m_userNameLabel;        // 用户名标签（值）
    QLabel* m_systemUpTimeLabel;    // 系统运行时间标签（值）

    // ===== 硬件信息组 =====
    QGroupBox* m_hardwareGroup;     // 硬件信息分组框
    QLabel* m_cpuInfoLabel;         // CPU信息标签（值）
    QLabel* m_cpuCoresLabel;        // CPU核心数标签（值）

    // ===== 内存信息组 =====
    QGroupBox* m_memoryGroup;       // 内存信息分组框
    QLabel* m_totalMemoryLabel;     // 总物理内存标签（值）
    QLabel* m_availableMemoryLabel; // 可用物理内存标签（值）
    QLabel* m_memoryUsageLabel;     // 内存使用率标签（值）

    // 刷新按钮
    QPushButton* m_refreshBtn;
};

#endif // SYSTEMINFOWIDGET_H