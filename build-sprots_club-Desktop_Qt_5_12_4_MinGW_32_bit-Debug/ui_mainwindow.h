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
    QLineEdit *searchLineEdit1;
    QPushButton *logoutButton;
    QLabel *imagelabel;
    QLabel *label;
    QLineEdit *searchLineEdit2;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QLabel *Role_Label;
    QLabel *FIO_Label;
    QLineEdit *filterLineEdit1;
    QLineEdit *filterLineEdit2;
    QLabel *labelSearch;
    QLabel *labelFilters;
    QPushButton *addButton;
    QPushButton *delButton;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1209, 820);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        tableView = new QTableView(centralWidget);
        tableView->setObjectName(QString::fromUtf8("tableView"));
        tableView->setGeometry(QRect(170, 130, 1031, 681));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(10, 190, 150, 40));
        pushButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: #e0e0e0; \n"
"    color: #444444;\n"
"    border: none;\n"
"    border-radius: 5px;\n"
"    padding: 10px;\n"
"    text-align: center; \n"
"    font-weight: 500;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background-color: #d5d5d5;\n"
"}\n"
"\n"
"QPushButton:checked {\n"
"    background-color: #bdbdbd;\n"
"    font-weight: bold;\n"
"}"));
        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(10, 130, 150, 40));
        pushButton_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: #e0e0e0; \n"
"    color: #444444;\n"
"    border: none;\n"
"    border-radius: 5px;\n"
"    padding: 10px;\n"
"    text-align: center; \n"
"    font-weight: 500;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background-color: #d5d5d5;\n"
"}\n"
"\n"
"QPushButton:checked {\n"
"    background-color: #bdbdbd;\n"
"    font-weight: bold;\n"
"}"));
        searchLineEdit1 = new QLineEdit(centralWidget);
        searchLineEdit1->setObjectName(QString::fromUtf8("searchLineEdit1"));
        searchLineEdit1->setGeometry(QRect(170, 90, 191, 31));
        searchLineEdit1->setAutoFillBackground(false);
        searchLineEdit1->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: #f0f0f0;\n"
"    border: 1px solid #d1d1d1; \n"
"    border-radius: 12px;\n"
"    padding: 6px 12px;\n"
"    color: #333333; \n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    background-color: #ffffff; \n"
"    border: 1px solid #a0a0a0;\n"
"}"));
        searchLineEdit1->setDragEnabled(false);
        searchLineEdit1->setPlaceholderText(QString::fromUtf8(""));
        logoutButton = new QPushButton(centralWidget);
        logoutButton->setObjectName(QString::fromUtf8("logoutButton"));
        logoutButton->setGeometry(QRect(10, 700, 81, 31));
        imagelabel = new QLabel(centralWidget);
        imagelabel->setObjectName(QString::fromUtf8("imagelabel"));
        imagelabel->setGeometry(QRect(10, 10, 150, 100));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(164, 9, 571, 41));
        QFont font;
        font.setPointSize(25);
        font.setBold(true);
        font.setWeight(75);
        label->setFont(font);
        label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        searchLineEdit2 = new QLineEdit(centralWidget);
        searchLineEdit2->setObjectName(QString::fromUtf8("searchLineEdit2"));
        searchLineEdit2->setGeometry(QRect(370, 90, 191, 31));
        searchLineEdit2->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: #f0f0f0; \n"
"    border: 1px solid #d1d1d1; \n"
"    border-radius: 12px;\n"
"    padding: 6px 12px;\n"
"    color: #333333;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    background-color: #ffffff;\n"
"    border: 1px solid #a0a0a0;\n"
"}"));
        pushButton_3 = new QPushButton(centralWidget);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));
        pushButton_3->setGeometry(QRect(10, 250, 150, 40));
        pushButton_3->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: #e0e0e0; \n"
"    color: #444444;\n"
"    border: none;\n"
"    border-radius: 5px;\n"
"    padding: 10px;\n"
"    text-align: center; \n"
"    font-weight: 500;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background-color: #d5d5d5;\n"
"}\n"
"\n"
"QPushButton:checked {\n"
"    background-color: #bdbdbd;\n"
"    font-weight: bold;\n"
"}"));
        pushButton_4 = new QPushButton(centralWidget);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));
        pushButton_4->setGeometry(QRect(10, 310, 150, 40));
        pushButton_4->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: #e0e0e0; \n"
"    color: #444444;\n"
"    border: none;\n"
"    border-radius: 5px;\n"
"    padding: 10px;\n"
"    text-align: center; \n"
"    font-weight: 500;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background-color: #d5d5d5;\n"
"}\n"
"\n"
"QPushButton:checked {\n"
"    background-color: #bdbdbd;\n"
"    font-weight: bold;\n"
"}"));
        Role_Label = new QLabel(centralWidget);
        Role_Label->setObjectName(QString::fromUtf8("Role_Label"));
        Role_Label->setGeometry(QRect(10, 770, 70, 30));
        QFont font1;
        font1.setPointSize(10);
        font1.setBold(true);
        font1.setWeight(75);
        Role_Label->setFont(font1);
        Role_Label->setAlignment(Qt::AlignCenter);
        FIO_Label = new QLabel(centralWidget);
        FIO_Label->setObjectName(QString::fromUtf8("FIO_Label"));
        FIO_Label->setGeometry(QRect(10, 740, 150, 30));
        filterLineEdit1 = new QLineEdit(centralWidget);
        filterLineEdit1->setObjectName(QString::fromUtf8("filterLineEdit1"));
        filterLineEdit1->setGeometry(QRect(670, 90, 191, 31));
        filterLineEdit1->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: #f0f0f0; \n"
"    border: 1px solid #d1d1d1; \n"
"    color: #333333;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    background-color: #ffffff;\n"
"    border: 1px solid #a0a0a0;\n"
"}"));
        filterLineEdit2 = new QLineEdit(centralWidget);
        filterLineEdit2->setObjectName(QString::fromUtf8("filterLineEdit2"));
        filterLineEdit2->setGeometry(QRect(870, 90, 191, 31));
        filterLineEdit2->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: #f0f0f0; \n"
"    border: 1px solid #d1d1d1; \n"
"    color: #333333;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    background-color: #ffffff;\n"
"    border: 1px solid #a0a0a0;\n"
"}"));
        labelSearch = new QLabel(centralWidget);
        labelSearch->setObjectName(QString::fromUtf8("labelSearch"));
        labelSearch->setGeometry(QRect(170, 50, 81, 41));
        labelSearch->setFont(font1);
        labelFilters = new QLabel(centralWidget);
        labelFilters->setObjectName(QString::fromUtf8("labelFilters"));
        labelFilters->setGeometry(QRect(670, 50, 101, 41));
        labelFilters->setFont(font1);
        addButton = new QPushButton(centralWidget);
        addButton->setObjectName(QString::fromUtf8("addButton"));
        addButton->setGeometry(QRect(1110, 90, 31, 31));
        addButton->setFont(font1);
        addButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: #4CAF50;\n"
"    color: white;\n"
"    border-radius: 8px;\n"
"    font-weight: bold;\n"
"    padding: 5px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #45a049;\n"
"}"));
        delButton = new QPushButton(centralWidget);
        delButton->setObjectName(QString::fromUtf8("delButton"));
        delButton->setGeometry(QRect(1150, 90, 51, 31));
        QFont font2;
        font2.setPointSize(9);
        font2.setBold(true);
        font2.setWeight(75);
        delButton->setFont(font2);
        delButton->setAutoFillBackground(false);
        delButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: #f44336;\n"
"    color: white;\n"
"    border-radius: 8px;\n"
"    font-weight: bold;\n"
"    padding: 5px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #d32f2f;\n"
"}"));
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
        labelSearch->setText(QApplication::translate("MainWindow", "\320\237\320\276\320\270\321\201\320\272:", nullptr));
        labelFilters->setText(QApplication::translate("MainWindow", "\320\244\320\270\320\273\321\214\321\202\321\200\321\213:", nullptr));
        addButton->setText(QApplication::translate("MainWindow", "+", nullptr));
        delButton->setText(QApplication::translate("MainWindow", "del", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
