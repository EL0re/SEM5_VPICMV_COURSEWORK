#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>
#include <QtSql>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/Users/XE4/Desktop/sports_club.db");
    if (!db.open()) return -1;
    while (true)
    {
        logindialog login;
        if (login.exec() != QDialog::Accepted)
            return 0;
        if (login.role() != "admin") {
            continue;
        }
        MainWindow w(login.fullName());
        w.show();
        a.exec();
        if (!w.logoutRequested)
                    return 0;
    }
    return 0;
}
