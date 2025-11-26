#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QSqlQuery>
#include <QSqlError>


namespace Ui {
class logindialog;
}

class logindialog : public QDialog
{
    Q_OBJECT

public:
    explicit logindialog(QWidget *parent = nullptr);
    ~logindialog();

    QString role() const { return m_role; }
    QString username() const { return m_username; }

private slots:
    void tryLogin();

private:
    Ui::logindialog *ui;
    QString m_role;
    QString m_username;
};

#endif // LOGINDIALOG_H
