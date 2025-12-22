#include "studentwindow.h"
#include "ui_studentwindow.h"
#include "studentschedulefilterproxy.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDate>
#include <QMessageBox>
#include <QSortFilterProxyModel>

StudentWindow::StudentWindow(int studentId, const QString &fullName, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StudentWindow),
    m_studentId(studentId),
    m_fullName(fullName),
    m_scheduleProxyModel(nullptr)
{
    ui->setupUi(this);

    ui->userInfoLabel->setText(fullName);

    setupConnections();
    setupScheduleTable();
    loadStudentGroups();

    onScheduleButtonClicked();

    ui->attendanceProgressBar->setStyleSheet(
        "QProgressBar {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 5px;"
        "    text-align: center;"
        "    background-color: #f0f0f0;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #4CAF50;"
        "    border-radius: 4px;"
        "}"
    );
}

StudentWindow::~StudentWindow()
{
    delete ui;
}

void StudentWindow::setupScheduleTable()
{
    m_scheduleProxyModel = new StudentScheduleFilterProxy(m_studentId, this);
    ui->scheduleTableView->setModel(m_scheduleProxyModel);

    ui->scheduleTableView->hideColumn(0);
    ui->scheduleTableView->hideColumn(1);

    ui->scheduleTableView->horizontalHeader()->setStretchLastSection(true);
    ui->scheduleTableView->setAlternatingRowColors(true);
    ui->scheduleTableView->setSortingEnabled(true);

    loadScheduleData();
}

void StudentWindow::loadScheduleData()
{
    QString queryStr =
        "SELECT s.id, s.group_id, g.name as group_name, s.day_of_week, "
        "s.start_time, s.end_time, s.hall, u.full_name as trainer_name "
        "FROM schedule s "
        "JOIN groups g ON s.group_id = g.id "
        "JOIN users u ON g.trainer_id = u.id "
        "ORDER BY "
        "CASE s.day_of_week "
        "  WHEN 'Понедельник' THEN 1 "
        "  WHEN 'Вторник' THEN 2 "
        "  WHEN 'Среда' THEN 3 "
        "  WHEN 'Четверг' THEN 4 "
        "  WHEN 'Пятница' THEN 5 "
        "  WHEN 'Суббота' THEN 6 "
        "  WHEN 'Воскресенье' THEN 7 "
        "END, s.start_time";

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        qDebug() << "Ошибка загрузки расписания:" << query.lastError().text();
        return;
    }

    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery(query);

    queryModel->setHeaderData(0, Qt::Horizontal, "ID");
    queryModel->setHeaderData(1, Qt::Horizontal, "Group ID");
    queryModel->setHeaderData(2, Qt::Horizontal, "Группа");
    queryModel->setHeaderData(3, Qt::Horizontal, "День недели");
    queryModel->setHeaderData(4, Qt::Horizontal, "Начало");
    queryModel->setHeaderData(5, Qt::Horizontal, "Конец");
    queryModel->setHeaderData(6, Qt::Horizontal, "Зал");
    queryModel->setHeaderData(7, Qt::Horizontal, "Тренер");

    if (m_scheduleProxyModel) {
        m_scheduleProxyModel->setSourceModel(queryModel);
        ui->scheduleTableView->hideColumn(0); // schedule.id
        ui->scheduleTableView->hideColumn(1); // group_id
    }
}

void StudentWindow::loadStudentGroups()
{
    QSqlQuery query;
    query.prepare("SELECT g.id, g.name FROM groups g "
                  "JOIN group_students gs ON g.id = gs.group_id "
                  "WHERE gs.student_id = ? ORDER BY g.name");
    query.addBindValue(m_studentId);

    if (query.exec()) {
        ui->groupComboBox->clear();
        ui->groupComboBox->setEnabled(true);

        while (query.next()) {
            int groupId = query.value(0).toInt();
            QString groupName = query.value(1).toString();
            ui->groupComboBox->addItem(groupName, groupId);
        }

        if (ui->groupComboBox->count() > 0) {
            ui->groupComboBox->setCurrentIndex(0);
            updateAttendanceProgress();
        } else {
            ui->groupComboBox->setEnabled(false);
        }
    }
}

void StudentWindow::setupConnections()
{
    connect(ui->scheduleButton, &QPushButton::clicked, this, &StudentWindow::onScheduleButtonClicked);
    connect(ui->profileButton, &QPushButton::clicked, this, &StudentWindow::onProfileButtonClicked);

    connect(ui->logoutButton, &QPushButton::clicked, this, &StudentWindow::onLogoutButtonClicked);

    connect(ui->groupComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StudentWindow::onGroupComboBoxChanged);

    connect(ui->groupSearchEdit, &QLineEdit::textChanged, this, &StudentWindow::onSearchTextChanged);
    connect(ui->daySearchEdit, &QLineEdit::textChanged, this, &StudentWindow::onSearchTextChanged);
}

void StudentWindow::onScheduleButtonClicked()
{
    ui->groupSearchEdit->show();
    ui->daySearchEdit->show();
    ui->scheduleTableView->show();
    ui->labelSearch->show();

    ui->profileContainer->hide();

    ui->titleLabel->setText("Расписание");

    ui->scheduleButton->setChecked(true);
    ui->profileButton->setChecked(false);
}

void StudentWindow::onProfileButtonClicked()
{
    ui->groupSearchEdit->hide();
    ui->daySearchEdit->hide();
    ui->scheduleTableView->hide();
    ui->labelSearch->hide();

    ui->profileContainer->show();

    ui->titleLabel->setText("Личный кабинет");

    ui->profileButton->setChecked(true);
    ui->scheduleButton->setChecked(false);

    updateAttendanceProgress();
}

void StudentWindow::onLogoutButtonClicked()
{
    logoutRequested = true;
    this->close();
}

void StudentWindow::onGroupComboBoxChanged(int index)
{
    if (index >= 0) {
        updateAttendanceProgress();
    }
}

void StudentWindow::onSearchTextChanged()
{
    QString groupFilter = ui->groupSearchEdit->text();
    QString dayFilter = ui->daySearchEdit->text();

    if (!groupFilter.isEmpty()) {
        ui->daySearchEdit->setEnabled(false);
        ui->daySearchEdit->setPlaceholderText("Очистите поле 'Группа'");
    }
    else if (!dayFilter.isEmpty()) {
        ui->groupSearchEdit->setEnabled(false);
        ui->groupSearchEdit->setPlaceholderText("Очистите поле 'День недели'");
    }
    else {
        ui->groupSearchEdit->setEnabled(true);
        ui->daySearchEdit->setEnabled(true);
        ui->groupSearchEdit->setPlaceholderText("Группа...");
        ui->daySearchEdit->setPlaceholderText("День недели...");
    }

    if (!m_scheduleProxyModel) {
        return;
    }

    if (!groupFilter.isEmpty()) {
        m_scheduleProxyModel->setFilterFixedString(groupFilter);
        m_scheduleProxyModel->setFilterKeyColumn(2);
    }
    else if (!dayFilter.isEmpty()) {
        m_scheduleProxyModel->setFilterFixedString(dayFilter);
        m_scheduleProxyModel->setFilterKeyColumn(3);
    }
    else {
        m_scheduleProxyModel->setFilterFixedString("");
    }

    m_scheduleProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_scheduleProxyModel->invalidate();
}

void StudentWindow::updateAttendanceProgress()
{
    int groupId = ui->groupComboBox->currentData().toInt();
    if (groupId <= 0) return;

    QSqlQuery query;

    query.prepare("SELECT COUNT(*) FROM schedule WHERE group_id = ?");
    query.addBindValue(groupId);

    int totalLessons = 0;
    if (query.exec() && query.next()) {
        totalLessons = query.value(0).toInt();
    }

    query.prepare(
        "SELECT COUNT(*) FROM attendance a "
        "WHERE a.student_id = ? AND a.group_id = ? "
        "AND a.status = 'present'"
    );
    query.addBindValue(m_studentId);
    query.addBindValue(groupId);

    int attendedLessons = 0;
    if (query.exec() && query.next()) {
        attendedLessons = query.value(0).toInt();
    }

    if (totalLessons > 0) {
        int percentage = (attendedLessons * 100) / totalLessons;
        ui->attendanceProgressBar->setValue(percentage);
        ui->attendanceTextLabel->setText(
            QString("%1 из %2 занятий (%3%)").arg(attendedLessons).arg(totalLessons).arg(percentage)
        );
    } else {
        ui->attendanceProgressBar->setValue(0);
        ui->attendanceTextLabel->setText("Нет занятий в расписании");
    }
}
