/********************************************************************************
** Form generated from reading UI file 'logindialog.ui'
**
** Created by: Qt User Interface Compiler version 5.12.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINDIALOG_H
#define UI_LOGINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_logindialog
{
public:
    QLineEdit *passEdit;
    QLineEdit *loginEdit;
    QLabel *pass_label;
    QLabel *login_label;
    QPushButton *loginBtn;

    void setupUi(QWidget *logindialog)
    {
        if (logindialog->objectName().isEmpty())
            logindialog->setObjectName(QString::fromUtf8("logindialog"));
        logindialog->resize(300, 400);
        passEdit = new QLineEdit(logindialog);
        passEdit->setObjectName(QString::fromUtf8("passEdit"));
        passEdit->setGeometry(QRect(50, 225, 200, 50));
        loginEdit = new QLineEdit(logindialog);
        loginEdit->setObjectName(QString::fromUtf8("loginEdit"));
        loginEdit->setGeometry(QRect(50, 75, 200, 50));
        pass_label = new QLabel(logindialog);
        pass_label->setObjectName(QString::fromUtf8("pass_label"));
        pass_label->setGeometry(QRect(0, 150, 300, 50));
        QFont font;
        font.setPointSize(15);
        font.setBold(true);
        font.setWeight(75);
        pass_label->setFont(font);
        pass_label->setAlignment(Qt::AlignCenter);
        login_label = new QLabel(logindialog);
        login_label->setObjectName(QString::fromUtf8("login_label"));
        login_label->setGeometry(QRect(0, 12, 300, 50));
        login_label->setFont(font);
        login_label->setLayoutDirection(Qt::LeftToRight);
        login_label->setAlignment(Qt::AlignCenter);
        loginBtn = new QPushButton(logindialog);
        loginBtn->setObjectName(QString::fromUtf8("loginBtn"));
        loginBtn->setGeometry(QRect(75, 325, 150, 50));

        retranslateUi(logindialog);

        QMetaObject::connectSlotsByName(logindialog);
    } // setupUi

    void retranslateUi(QWidget *logindialog)
    {
        logindialog->setWindowTitle(QApplication::translate("logindialog", "Form", nullptr));
        pass_label->setText(QApplication::translate("logindialog", "\320\237\320\220\320\240\320\236\320\233\320\254", nullptr));
        login_label->setText(QApplication::translate("logindialog", "\320\233\320\236\320\223\320\230\320\235", nullptr));
        loginBtn->setText(QApplication::translate("logindialog", "\320\222\320\236\320\231\320\242\320\230", nullptr));
    } // retranslateUi

};

namespace Ui {
    class logindialog: public Ui_logindialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
