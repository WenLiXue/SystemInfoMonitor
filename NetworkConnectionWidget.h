// networkconnectionwidget.h
#ifndef NETWORKCONNECTIONWIDGET_H
#define NETWORKCONNECTIONWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDateTime>
#include<QComboBox>
#include<QStyledItemDelegate>
#include <QPainter>  // 添加这一行

#include "datamanager.h" // 包含DataManager头文件

// 网络连接Widget
class NetworkConnectionWidget : public QWidget {
    Q_OBJECT
public:
    explicit NetworkConnectionWidget(QWidget* parent = nullptr);
    ~NetworkConnectionWidget();

private slots:
    void onRefreshButtonClicked(); // 刷新按钮点击事件
    void onFilterChanged(int index); // 过滤下拉框变化事件

private:
    void initUI(); // 初始化UI
    void refreshTable();
    QString truncateAddress(const QString& address, int maxLength);
    // 刷新表格数据
    void updateStatus(const QString& text); // 更新状态栏

    QTableView* m_tableView; // 表格视图
    QStandardItemModel* m_model; // 表格模型
    QPushButton* m_refreshBtn; // 刷新按钮
    QLabel* m_statusLabel; // 状态栏
    QComboBox* m_filterCombo; // 过滤下拉框
};

#endif // NETWORKCONNECTIONWIDGET_H