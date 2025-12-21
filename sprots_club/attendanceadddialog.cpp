#include "attendanceadddialog.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

AttendanceAddDialog::AttendanceAddDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Добавление записей посещаемости");
    setModal(true);
    setMinimumSize(700, 350); // Увеличенный размер окна

    // Основной вертикальный слой
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Стили для меток (Labels)
    QString labelStyle = "QLabel { font-size: 16px; font-weight: bold; }";

    // Верхняя часть (Группа и Календарь)
    QHBoxLayout *topLayout = new QHBoxLayout();

    // Левая колонка: Группа
    QVBoxLayout *groupLayout = new QVBoxLayout();
    QLabel *groupLabel = new QLabel("Группа:", this);
    groupLabel->setStyleSheet(labelStyle);

    groupLineEdit = new QLineEdit(this);
    groupLineEdit->setPlaceholderText("Введите название...");
    groupLineEdit->setMinimumHeight(40); // Немного увеличим высоту для удобства

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

    // Правая колонка: Дата
    QVBoxLayout *dateLayout = new QVBoxLayout();
    QLabel *dateLabel = new QLabel("Дата:", this);
    dateLabel->setStyleSheet(labelStyle);

    calendar = new QCalendarWidget(this);
    calendar->setGridVisible(true);

    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(calendar);

    topLayout->addLayout(groupLayout);
    topLayout->addLayout(dateLayout);

    // Нижняя часть (Кнопки)
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("Отмена", this);
    QPushButton *saveBtn = new QPushButton("Сохранить", this);

    // Общий стиль для кнопок (размер и шрифт)
    QString baseButtonStyle = "QPushButton { font-size: 14px; font-weight: bold; min-width: 120px; min-height: 40px; color: white; border-radius: 4px; }";

    // Стилизация: Отмена (Красная)
    cancelBtn->setStyleSheet(baseButtonStyle + "QPushButton { background-color: #f44336; } QPushButton:hover { background-color: #d32f2f; }");

    // Стилизация: Сохранить (Синяя)
    saveBtn->setStyleSheet(baseButtonStyle + "QPushButton { background-color: #2196F3; } QPushButton:hover { background-color: #1976D2; }");

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveBtn);

    mainLayout->addLayout(topLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // Соединение сигналов
    connect(cancelBtn, &QPushButton::clicked, this, &AttendanceAddDialog::onCancelClicked);
    connect(saveBtn, &QPushButton::clicked, this, &AttendanceAddDialog::onSaveClicked);
}

void AttendanceAddDialog::onCancelClicked() {
    reject();
}

void AttendanceAddDialog::onSaveClicked() {
    QString groupName = groupLineEdit->text().trimmed();
    QDate selectedDate = calendar->selectedDate();
    QString dateStr = selectedDate.toString("yyyy-MM-dd");

    if (groupName.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, введите название группы.");
        return;
    }

    QSqlQuery query;
    // 1. Поиск ID группы
    query.prepare("SELECT id FROM groups WHERE name = ?");
    query.addBindValue(groupName);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Ошибка", "Группа не найдена.");
        return;
    }
    int groupId = query.value(0).toInt();

    // --- НОВАЯ ЛОГИКА: ПРОВЕРКА РАСПИСАНИЯ ---

    // 1. Проверяем, есть ли у группы расписание вообще
    QSqlQuery checkSched;
    checkSched.prepare("SELECT COUNT(*) FROM schedule WHERE group_id = ?");
    checkSched.addBindValue(groupId);
    if (checkSched.exec() && checkSched.next()) {
        if (checkSched.value(0).toInt() == 0) {
            QMessageBox::warning(this, "Ошибка",
                QString("У группы '%1' не заполнено расписание.\nДобавьте занятия в таблицу 'Расписание' перед созданием посещаемости.").arg(groupName));
            return;
        }
    }

    // 2. Получаем день недели из календаря и сопоставляем с БД
    // QDate::dayOfWeek(): 1 = Понедельник, ..., 7 = Воскресенье
    static const QStringList daysRu = {"", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
    QString selectedDayName = daysRu.at(selectedDate.dayOfWeek());

    QSqlQuery checkDay;
    checkDay.prepare("SELECT id FROM schedule WHERE group_id = ? AND day_of_week = ?");
    checkDay.addBindValue(groupId);
    checkDay.addBindValue(selectedDayName);

    if (!checkDay.exec()) {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось проверить день недели в расписании.");
        return;
    }

    if (!checkDay.next()) {
        QMessageBox::warning(this, "Неверная дата",
            QString("Выбранная дата (%1) — это %2.\nВ расписании группы '%3' в этот день занятий нет.")
            .arg(selectedDate.toString("dd.MM.yyyy"), selectedDayName, groupName));
        return;
    }
    // --- КОНЕЦ НОВОЙ ЛОГИКИ ---

    // 2. Получение списка учеников
    query.prepare("SELECT student_id FROM group_students WHERE group_id = ?");
    query.addBindValue(groupId);
    if (!query.exec()) return;

    QList<int> studentIds;
    while (query.next()) studentIds.append(query.value(0).toInt());

    if (studentIds.isEmpty()) {
        QMessageBox::information(this, "Информация", "В группе нет учеников.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    for (int studentId : studentIds) {
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

        if (!insertQuery.exec()) {
            db.rollback();
            qDebug() << "SQL Error:" << insertQuery.lastError().text();
            QMessageBox::critical(this, "Ошибка", "Ошибка при создании записи: " + insertQuery.lastError().text());
            return;
        }
    }

    if (db.commit()) {
        QMessageBox::information(this, "Успех", "Список посещаемости сформирован.");
        accept();
    }
}
