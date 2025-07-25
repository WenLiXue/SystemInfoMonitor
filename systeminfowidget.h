#ifndef SYSTEMINFOWIDGET_H
#define SYSTEMINFOWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
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
    void startAutoRefresh();
    void stopAutoRefresh();
    ~SystemInfoWidget() override;

    void initUI();

    void refreshSystemInfo();
    void setAsCurrentTab(); // 标签页切换到当前页时调用

private slots:
    void onRefreshClicked();  // 手动刷新按钮
    void onAutoRefresh();     // 定时器自动刷新
    void onTabVisibleChanged(bool visible); // 标签页显示/隐藏状态变化

private:
    Ui::SystemInfoWidget* ui;
    DataManager& m_dataManager;

    // UI组件
    QGroupBox* m_osGroup;
    QLabel* m_osVersionLabel;
    QLabel* m_hostNameLabel;
    QLabel* m_userNameLabel;
    QLabel* m_systemUpTimeLabel;

    QGroupBox* m_hardwareGroup;
    QLabel* m_cpuInfoLabel;
    QLabel* m_cpuCoresLabel;
    QLabel* m_cpuUsageLabel;
    QProgressBar* m_cpuUsageBar;

    QGroupBox* m_memoryGroup;
    QLabel* m_totalMemoryLabel;
    QLabel* m_availableMemoryLabel;
    QLabel* m_memoryUsageLabel;
    QProgressBar* m_memoryUsageBar;

    QPushButton* m_refreshBtn;
    QTimer* m_autoRefreshTimer; // 自动刷新定时器
    bool m_firstLoad;           // 是否首次加载
};

#endif // SYSTEMINFOWIDGET_H