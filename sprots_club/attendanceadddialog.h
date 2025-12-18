#ifndef ATTENDANCEADDDIALOG_H
#define ATTENDANCEADDDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCalendarWidget>
#include <QPushButton>
#include <QDate>
#include <QDebug>

class AttendanceAddDialog : public QDialog {
    Q_OBJECT
public:
    explicit AttendanceAddDialog(QWidget *parent = nullptr);

private slots:
    void onCancelClicked();
    void onSaveClicked();

private:
    QLineEdit *groupLineEdit;
    QCalendarWidget *calendar;
};

#endif // ATTENDANCEADDDIALOG_H
