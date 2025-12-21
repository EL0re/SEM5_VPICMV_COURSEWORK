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
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_logindialog
{
public:
    QPushButton *loginBtn;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_3;
    QLabel *login_label;
    QLineEdit *loginEdit;
    QLabel *pass_label;
    QLineEdit *passEdit;

    void setupUi(QWidget *logindialog)
    {
        if (logindialog->objectName().isEmpty())
            logindialog->setObjectName(QString::fromUtf8("logindialog"));
        logindialog->resize(382, 490);
        loginBtn = new QPushButton(logindialog);
        loginBtn->setObjectName(QString::fromUtf8("loginBtn"));
        loginBtn->setGeometry(QRect(90, 440, 200, 40));
        loginBtn->setMinimumSize(QSize(100, 30));
        loginBtn->setMaximumSize(QSize(200, 40));
        layoutWidget = new QWidget(logindialog);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 12, 361, 391));
        verticalLayout_3 = new QVBoxLayout(layoutWidget);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        login_label = new QLabel(layoutWidget);
        login_label->setObjectName(QString::fromUtf8("login_label"));
        login_label->setMaximumSize(QSize(359, 100));
        QFont font;
        font.setPointSize(15);
        font.setBold(true);
        font.setWeight(75);
        login_label->setFont(font);
        login_label->setLayoutDirection(Qt::LeftToRight);
        login_label->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(login_label);

        loginEdit = new QLineEdit(layoutWidget);
        loginEdit->setObjectName(QString::fromUtf8("loginEdit"));
        loginEdit->setMinimumSize(QSize(230, 35));

        verticalLayout_3->addWidget(loginEdit, 0, Qt::AlignHCenter);

        pass_label = new QLabel(layoutWidget);
        pass_label->setObjectName(QString::fromUtf8("pass_label"));
        pass_label->setMinimumSize(QSize(100, 100));
        pass_label->setMaximumSize(QSize(359, 100));
        pass_label->setFont(font);
        pass_label->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(pass_label);

        passEdit = new QLineEdit(layoutWidget);
        passEdit->setObjectName(QString::fromUtf8("passEdit"));
        passEdit->setMinimumSize(QSize(230, 35));
        passEdit->setEchoMode(QLineEdit::Password);

        verticalLayout_3->addWidget(passEdit, 0, Qt::AlignHCenter);

        pass_label->raise();
        passEdit->raise();
        loginEdit->raise();
        login_label->raise();

        retranslateUi(logindialog);

        QMetaObject::connectSlotsByName(logindialog);
    } // setupUi

    void retranslateUi(QWidget *logindialog)
    {
        logindialog->setWindowTitle(QApplication::translate("logindialog", "\320\222\321\205\320\276\320\264 \320\262 \321\201\320\270\321\201\321\202\320\265\320\274\321\203", nullptr));
        loginBtn->setText(QApplication::translate("logindialog", "\320\222\320\236\320\231\320\242\320\230", nullptr));
        login_label->setText(QApplication::translate("logindialog", "\320\233\320\236\320\223\320\230\320\235", nullptr));
        pass_label->setText(QApplication::translate("logindialog", "\320\237\320\220\320\240\320\236\320\233\320\254", nullptr));
    } // retranslateUi

};

namespace Ui {
    class logindialog: public Ui_logindialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
