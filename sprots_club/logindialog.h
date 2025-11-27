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

    QString fullName() const { return m_fullName; }
    QString role() const { return m_role; }
    QString username() const { return m_username; }

private slots:
    void tryLogin();

private:
    Ui::logindialog *ui;
    QString m_role;
    QString m_username;
    QString m_fullName;

};

#endif // LOGINDIALOG_H
