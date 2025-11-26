#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>
#include <QtSql>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/Users/XE4/Desktop/sports_club.db");
    db.open();
    QSqlQuery test("SELECT * FROM users");
    while (test.next()) {
        qDebug() << "USER:" << test.value("login").toString()
                 << test.value("password").toString()
                 << test.value("role").toString();
    }
    if (!db.open()) {
        qDebug() << "DB ERROR:" << db.lastError().text();
        return -1;
    }

    // Показываем окно логина
    logindialog login;
    if (login.exec() != QDialog::Accepted) {
        // если пользователь закрыл окно или нажал отмену — выходим
        return 0;
    }

    // Проверяем роль
    if (login.role() != "admin") {
        return 0;
    }

    // Успешная авторизация → запускаем основное окно
    MainWindow w;
    w.show();

    return a.exec();
}
