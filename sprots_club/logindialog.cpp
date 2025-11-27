#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>

logindialog::logindialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::logindialog)
{
    ui->setupUi(this);
    connect(ui->loginBtn, &QPushButton::clicked,
            this, &logindialog::tryLogin);
}

logindialog::~logindialog()
{
    delete ui;
}

void logindialog::tryLogin()
{

    QString login = ui->loginEdit->text().trimmed();
    QString pass  = ui->passEdit->text().trimmed();
    qDebug() << "INPUT LOGIN =" << login;
    qDebug() << "INPUT PASS  =" << pass;

    if (login.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите логин и пароль.");
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT full_name, role FROM users WHERE login = :l AND password = :p");
    q.bindValue(":l", login);
    q.bindValue(":p", pass);

    if (!q.exec()) {
        QMessageBox::critical(this, "Ошибка SQL", q.lastError().text());
        qDebug() << q.lastError();
        return;
    }

    if (q.next()) {
        m_fullName = q.value(0).toString();   // full_name
        m_role     = q.value(1).toString();   // role
        m_username = login;

        accept();   // Вход успешен → закрываем окно
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверный логин или пароль.");
    }
}
