/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QTableView *tableView;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QLineEdit *searchLineEdit;
    QPushButton *logoutButton;
    QLabel *imagelabel;
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QLabel *Role_Label;
    QLabel *FIO_Label;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1200, 800);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        tableView = new QTableView(centralWidget);
        tableView->setObjectName(QString::fromUtf8("tableView"));
        tableView->setGeometry(QRect(170, 115, 1031, 680));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(10, 180, 150, 40));
        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(10, 120, 150, 40));
        searchLineEdit = new QLineEdit(centralWidget);
        searchLineEdit->setObjectName(QString::fromUtf8("searchLineEdit"));
        searchLineEdit->setGeometry(QRect(170, 70, 280, 40));
        searchLineEdit->setPlaceholderText(QString::fromUtf8(""));
        logoutButton = new QPushButton(centralWidget);
        logoutButton->setObjectName(QString::fromUtf8("logoutButton"));
        logoutButton->setGeometry(QRect(90, 760, 70, 30));
        imagelabel = new QLabel(centralWidget);
        imagelabel->setObjectName(QString::fromUtf8("imagelabel"));
        imagelabel->setGeometry(QRect(10, 10, 150, 100));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(164, 9, 571, 51));
        QFont font;
        font.setPointSize(25);
        font.setBold(true);
        font.setWeight(75);
        label->setFont(font);
        label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        lineEdit = new QLineEdit(centralWidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(460, 70, 280, 40));
        pushButton_3 = new QPushButton(centralWidget);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));
        pushButton_3->setGeometry(QRect(10, 240, 150, 40));
        pushButton_4 = new QPushButton(centralWidget);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));
        pushButton_4->setGeometry(QRect(10, 300, 150, 40));
        Role_Label = new QLabel(centralWidget);
        Role_Label->setObjectName(QString::fromUtf8("Role_Label"));
        Role_Label->setGeometry(QRect(10, 760, 70, 30));
        QFont font1;
        font1.setPointSize(10);
        font1.setBold(true);
        font1.setWeight(75);
        Role_Label->setFont(font1);
        Role_Label->setAlignment(Qt::AlignCenter);
        FIO_Label = new QLabel(centralWidget);
        FIO_Label->setObjectName(QString::fromUtf8("FIO_Label"));
        FIO_Label->setGeometry(QRect(10, 720, 150, 30));
        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        pushButton->setText(QApplication::translate("MainWindow", "\320\223\320\240\320\243\320\237\320\237\320\253", nullptr));
        pushButton_2->setText(QApplication::translate("MainWindow", "\320\237\320\236\320\233\320\254\320\227\320\236\320\222\320\220\320\242\320\225\320\233\320\230", nullptr));
        logoutButton->setText(QApplication::translate("MainWindow", "\320\222\320\253\320\231\320\242\320\230", nullptr));
        imagelabel->setText(QString());
        label->setText(QApplication::translate("MainWindow", "Admin Panel", nullptr));
        pushButton_3->setText(QApplication::translate("MainWindow", "\320\240\320\220\320\241\320\237\320\230\320\241\320\220\320\235\320\230\320\225", nullptr));
        pushButton_4->setText(QApplication::translate("MainWindow", "\320\237\320\236\320\241\320\225\320\251\320\220\320\225\320\234\320\236\320\241\320\242\320\254", nullptr));
        Role_Label->setText(QApplication::translate("MainWindow", "ADMIN", nullptr));
        FIO_Label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
