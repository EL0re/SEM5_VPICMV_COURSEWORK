#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>

namespace Ui {
class StudentWindow;
}

class StudentScheduleFilterProxy;

class StudentWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StudentWindow(int studentId, const QString &fullName, QWidget *parent = nullptr);
    ~StudentWindow();

    bool logoutRequested = false;

private slots:
    void onScheduleButtonClicked();
    void onProfileButtonClicked();
    void onLogoutButtonClicked();
    void onGroupComboBoxChanged(int index);
    void onSearchTextChanged();
    void updateAttendanceProgress();

private:
    Ui::StudentWindow *ui;
    int m_studentId;
    QString m_fullName;

    StudentScheduleFilterProxy *m_scheduleProxyModel;

    void setupScheduleTable();
    void loadStudentGroups();
    void loadScheduleData();
    void setupConnections();
};

#endif // STUDENTWINDOW_H
