#ifndef TRAINERWINDOW_H
#define TRAINERWINDOW_H

#include <QWidget>
#include <QString>
#include <QModelIndex>
#include "tablemanager.h"
#include <QDate>

namespace Ui {
class trainerwindow;
}

class trainerwindow : public QWidget
{
    Q_OBJECT

public:
    void onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    explicit trainerwindow(int userId, const QString &fullName, QWidget *parent = nullptr);
    ~trainerwindow();

    bool logoutRequested = false;

private slots:
    void on_pushButton_3_clicked();
    void on_logoutButton_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_clicked();
    void on_addButton_clicked();
    void on_anySearchField_changed();

private:
    Ui::trainerwindow *ui;
    QString currentTable;
    TableManager *tableManager;
    int currentUserId;
    void setupDelegatesForCurrentTable();
    void switchToTable(const QString &tableName, const QString &title);
    void reloadview();
    void updateUI(bool s1, bool s2, bool f1, bool f2, const QString &ps1, const QString &ps2, const QString &pf1, const QString &pf2);
};

#endif // TRAINERWINDOW_H
