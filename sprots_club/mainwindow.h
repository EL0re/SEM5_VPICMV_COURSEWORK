#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QModelIndex>
#include "tablemanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

public:
    explicit MainWindow(const QString &fullName, QWidget *parent = nullptr);
    ~MainWindow();

    bool logoutRequested = false;

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();

    void on_logoutButton_clicked();
    void on_addButton_clicked();
    void on_anySearchField_changed();
    void onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    Ui::MainWindow *ui;

    TableManager *tableManager;
    QString currentTable;
    QString currentSearchField;

    void setupGroupDelegates();
    int getUserIdByName(const QString &fullName);
    void switchToTable(const QString &tableName, const QString &title);
    void ensureTrailingEmptyRow();
    bool isRowFilled(int row) const;
    void reloadview();
    void commitLastRow();
    void updateUI(bool s1, bool s2, bool f1, bool f2, const QString &ps1, const QString &ps2, const QString &pf1, const QString &pf2);
};

#endif // MAINWINDOW_H
