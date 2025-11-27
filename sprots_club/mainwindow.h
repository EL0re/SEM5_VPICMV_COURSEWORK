#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QDebug>
#include <QFileInfo>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QEvent>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QRegularExpression>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

public:
    bool logoutRequested = false;
    explicit MainWindow(const QString &fullName, QWidget *parent = nullptr);
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_searchLineEdit_textChanged(const QString &text);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_logoutButton_clicked();

    void onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    Ui::MainWindow *ui;
    QSqlTableModel *model;
    QSortFilterProxyModel *proxyModel = nullptr;
    void ensureTrailingEmptyRow();
    bool isRowFilled(int row) const;
    void reloadview();
    void commitLastRow();
};

#endif // MAINWINDOW_H
