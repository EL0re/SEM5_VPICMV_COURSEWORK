#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <QCryptographicHash>
#include "utils.h"

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

void logindialog::tryLogin() {
    QString login = ui->loginEdit->text().trimmed();
    QString pass  = ui->passEdit->text().trimmed();

    if (login.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите логин и пароль.");
        return;
    }

    QString hashedInput = hashPassword(pass);


    QSqlQuery q;
    q.prepare("SELECT full_name, role FROM users WHERE login = :l AND password = :p");
    q.bindValue(":l", login);
    q.bindValue(":p", hashedInput);
//    q.bindValue(":p", pass);

    if (!q.exec()) {
        QMessageBox::critical(this, "Ошибка SQL", q.lastError().text());
        return;
    }

    if (q.next()) {
        m_fullName = q.value(0).toString();
        m_role     = q.value(1).toString();
        m_username = login;
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверный логин или пароль.");
    }
}

