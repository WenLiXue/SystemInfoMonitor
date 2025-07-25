#ifndef SERVICEWIDGET_H
#define SERVICEWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include "datamanager.h"

// 服务窗口类
class ServiceWidget : public QWidget {
    Q_OBJECT
public:
    explicit ServiceWidget(QWidget* parent = nullptr);
    ~ServiceWidget() override;

private slots:
    void onRefreshClicked(); // 刷新按钮点击事件

private:
    void initUI(); // 初始化UI
    void refreshTable(); // 刷新服务表格
    void updateStatus(const QString& text); // 更新状态栏

    // 状态转换辅助函数
    QString serviceStatusToString(DWORD status);

    // UI组件
    QTableView* m_tableView;
    QStandardItemModel* m_model;
    QPushButton* m_refreshBtn;
    QLabel* m_statusLabel;
};

#endif // SERVICEWIDGET_H