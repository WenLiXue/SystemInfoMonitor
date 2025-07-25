// sessionwidget.h
#ifndef SESSIONWIDGET_H
#define SESSIONWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include "datamanager.h"

class SessionWidget : public QWidget {
    Q_OBJECT
public:
    explicit SessionWidget(QWidget* parent = nullptr);
    ~SessionWidget() override;

private slots:
    void onRefreshButtonClicked();

private:
    void initUI();
    void refreshTable();
    void updateStatus(const QString& text);

    // 会话状态转换为可读字符串
    QString sessionStateToString(WTS_CONNECTSTATE_CLASS state);

    QTableView* m_tableView;
    QStandardItemModel* m_model;
    QPushButton* m_refreshBtn;
    QLabel* m_statusLabel;
};

#endif // SESSIONWIDGET_H