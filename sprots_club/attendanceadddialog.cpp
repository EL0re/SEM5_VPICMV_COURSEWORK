#include "attendanceadddialog.h"
#include <QDebug>

AttendanceAddDialog::AttendanceAddDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Добавление записей посещаемости");
    setModal(true);
    setMinimumSize(700, 500); // Увеличенный размер окна

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
    groupLineEdit->setMinimumHeight(35);
    groupLineEdit->setStyleSheet("QLineEdit { font-size: 14px; }");

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

    if (groupName.isEmpty()) {
        qDebug() << "Ошибка: Группа не введена";
        return;
    }

    qDebug() << "Сохранение: Группа =" << groupName << "Дата =" << selectedDate.toString("dd.MM.yyyy");

    accept();
}
