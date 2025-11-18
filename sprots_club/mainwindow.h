#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QDebug>
#include <QFileInfo>
#include <QSqlTableModel>
#include <QSqlQuery>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

//protected:
//    bool eventFilter(QObject *obj, QEvent *event) override;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
//     void onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    Ui::MainWindow *ui;
    QSqlTableModel *model;
    void ensureTrailingEmptyRow();
    bool isRowFilled(int row) const;
    void reloadview();
//    void commitLastRow();
};

#endif // MAINWINDOW_H
