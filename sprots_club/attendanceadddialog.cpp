#include "attendanceadddialog.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

/*тоже косметика для окна(пришлось под календарь цвет чутка подкорректировать) */
AttendanceAddDialog::AttendanceAddDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Добавление записей посещаемости");
    setModal(true);
    setMinimumSize(700, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QString labelStyle = "QLabel { font-size: 16px; font-weight: bold; }";

    QHBoxLayout *topLayout = new QHBoxLayout();

    QVBoxLayout *groupLayout = new QVBoxLayout();
    QLabel *groupLabel = new QLabel("Группа:", this);
    groupLabel->setStyleSheet(labelStyle);

    groupLineEdit = new QLineEdit(this);
    groupLineEdit->setPlaceholderText("Введите название...");
    groupLineEdit->setMinimumHeight(40);

    groupLineEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: #f0f0f0;"
        "    border: 1px solid #d1d1d1;"
        "    border-radius: 12px;"
        "    padding: 6px 12px;"
        "    color: #333333;"
        "    font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "    background-color: #ffffff;"
        "    border: 1px solid #a0a0a0;"
        "}"
    );

    groupLayout->addWidget(groupLabel);
    groupLayout->addWidget(groupLineEdit);
    groupLayout->addStretch();

    QVBoxLayout *dateLayout = new QVBoxLayout();
    QLabel *dateLabel = new QLabel("Дата:", this);
    dateLabel->setStyleSheet(labelStyle);

    calendar = new QCalendarWidget(this);
    calendar->setGridVisible(true);

    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(calendar);

    topLayout->addLayout(groupLayout);
    topLayout->addLayout(dateLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("Отмена", this);
    QPushButton *saveBtn = new QPushButton("Сохранить", this);

    QString baseButtonStyle = "QPushButton { font-size: 14px; font-weight: bold; min-width: 120px; min-height: 40px; color: white; border-radius: 4px; }";

    cancelBtn->setStyleSheet(baseButtonStyle + "QPushButton { background-color: #f44336; } QPushButton:hover { background-color: #d32f2f; }");
    saveBtn->setStyleSheet(baseButtonStyle + "QPushButton { background-color: #2196F3; } QPushButton:hover { background-color: #1976D2; }");

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveBtn);

    mainLayout->addLayout(topLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    connect(cancelBtn, &QPushButton::clicked, this, &AttendanceAddDialog::onCancelClicked);
    connect(saveBtn, &QPushButton::clicked, this, &AttendanceAddDialog::onSaveClicked);
}

/* отмена для окна */
void AttendanceAddDialog::onCancelClicked()
{
    reject();
}

/* сохранение, там валидация на то есть ли группа, сходится ли день недели с датой, есть ли вообще распа и т д
 зато потом находит всех учеников группы и создает записи с Фио, датой, группой, тока чекбокс остается отметить*/
void AttendanceAddDialog::onSaveClicked()
{
    QString groupName = groupLineEdit->text().trimmed();
    QDate selectedDate = calendar->selectedDate();
    QString dateStr = selectedDate.toString("yyyy-MM-dd");

    if (groupName.isEmpty())
    {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, введите название группы.");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT id FROM groups WHERE name = ?");
    query.addBindValue(groupName);

    if (!query.exec() || !query.next())
    {
        QMessageBox::warning(this, "Ошибка", "Группа не найдена.");
        return;
    }
    int groupId = query.value(0).toInt();

    QSqlQuery checkSched;
    checkSched.prepare("SELECT COUNT(*) FROM schedule WHERE group_id = ?");
    checkSched.addBindValue(groupId);
    if (checkSched.exec() && checkSched.next())
    {
        if (checkSched.value(0).toInt() == 0)
        {
            QMessageBox::warning(this, "Ошибка",
                QString("У группы '%1' не заполнено расписание.\nДобавьте занятия в таблицу 'Расписание' перед созданием посещаемости.").arg(groupName));
            return;
        }
    }

    static const QStringList daysRu = {"", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
    QString selectedDayName = daysRu.at(selectedDate.dayOfWeek());

    QSqlQuery checkDay;
    checkDay.prepare("SELECT id FROM schedule WHERE group_id = ? AND day_of_week = ?");
    checkDay.addBindValue(groupId);
    checkDay.addBindValue(selectedDayName);

    if (!checkDay.exec())
    {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось проверить день недели в расписании.");
        return;
    }

    if (!checkDay.next())
    {
        QMessageBox::warning(this, "Неверная дата",
            QString("Выбранная дата (%1) — это %2.\nВ расписании группы '%3' в этот день занятий нет.")
            .arg(selectedDate.toString("dd.MM.yyyy"), selectedDayName, groupName));
        return;
    }

    query.prepare("SELECT student_id FROM group_students WHERE group_id = ?");
    query.addBindValue(groupId);
    if (!query.exec()) return;

    QList<int> studentIds;
    while (query.next())
    {
        studentIds.append(query.value(0).toInt());
    }

    if (studentIds.isEmpty())
    {
        QMessageBox::information(this, "Информация", "В группе нет учеников.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    for (int studentId : studentIds)
    {
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT id FROM attendance WHERE student_id = ? AND group_id = ? AND lesson_date = ?");
        checkQuery.addBindValue(studentId);
        checkQuery.addBindValue(groupId);
        checkQuery.addBindValue(dateStr);

        if (checkQuery.exec() && checkQuery.next()) continue;

        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO attendance (student_id, group_id, lesson_date, status) "
                            "VALUES (?, ?, ?, ?)");
        insertQuery.addBindValue(studentId);
        insertQuery.addBindValue(groupId);
        insertQuery.addBindValue(dateStr);
        insertQuery.addBindValue("absent");

        if (!insertQuery.exec())
        {
            db.rollback();
            QMessageBox::critical(this, "Ошибка", "Ошибка при создании записи: " + insertQuery.lastError().text());
            return;
        }
    }

    if (db.commit())
    {
        QMessageBox::information(this, "Успех", "Список посещаемости сформирован.");
        accept();
    }
}
