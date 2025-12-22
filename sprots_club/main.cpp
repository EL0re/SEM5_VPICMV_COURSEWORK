#include "mainwindow.h"
#include "studentwindow.h"
#include "logindialog.h"
#include <QApplication>
#include <QtSql>
#include <QMessageBox>
#include <QDir>
#include <QCryptographicHash>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QDir::cleanPath(QApplication::applicationDirPath() + "/../sports_club.db");
    //пока крч так: копируйте бд в папку билда, иначе он будет создавать новую. С добавлением пути в исходную папку проекта потом разберемся
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

        QString role = login.role();
        QString fullName = login.fullName();

        int userId = -1;
        QSqlQuery query;
        query.prepare("SELECT id FROM users WHERE full_name = ?");
        query.addBindValue(fullName);
        if (query.exec() && query.next()) {
            userId = query.value(0).toInt();
        }

        if (role == "admin")
        {
            MainWindow w(fullName);
            w.show();
            int result = a.exec();

            if (!w.logoutRequested)
            {
                return result;
            }
        }
        else if (role == "student")
        {
            StudentWindow w(userId, fullName);
            w.show();
            int result = a.exec();

            if (!w.logoutRequested)
            {
                return result;
            }
        }
    }

    return 0;
}
