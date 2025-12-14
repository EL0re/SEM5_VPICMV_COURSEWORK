#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>
#include <QtSql>
#include <QMessageBox>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QDir::cleanPath(QApplication::applicationDirPath() + "/../sports_club.db");
    qDebug() << "Путь к БД:" << dbPath;
        db.setDatabaseName(dbPath);

        if (!db.open())
        {
            QMessageBox::critical(nullptr, "Ошибка подключения", QString("Не удалось открыть базу данных:\n%1\n\nПуть: %2").arg(db.lastError().text()).arg(dbPath));
            return -1;
        }

    while (true)
    {
        logindialog login;

        if (login.exec() != QDialog::Accepted)
            return 0;

        if (login.role() != "admin")
        {
            continue;
        }

        MainWindow w(login.fullName());
        w.show();
        int result = a.exec();

        if (w.logoutRequested || result != 0)
        {
            return result;
        }
    }

    return 0;
}
