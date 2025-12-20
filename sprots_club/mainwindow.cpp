#include <QKeyEvent>
#include <QEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMap>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include "attendanceadddialog.h"

MainWindow::MainWindow(const QString &fullName, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tableManager = new TableManager(this);

    ui->FIO_Label->setText(fullName);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->installEventFilter(this);
    connect(ui->searchLineEdit1, &QLineEdit::textChanged, this, &MainWindow::on_anySearchField_changed);
    connect(ui->searchLineEdit2, &QLineEdit::textChanged, this, &MainWindow::on_anySearchField_changed);
    connect(ui->filterLineEdit1, &QLineEdit::textChanged, this, &MainWindow::on_anySearchField_changed);
    connect(ui->filterLineEdit2, &QLineEdit::textChanged, this, &MainWindow::on_anySearchField_changed);
    on_pushButton_2_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::getUserIdByName(const QString &fullName) {
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE full_name = :name");
    query.bindValue(":name", fullName.trimmed());
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

void MainWindow::updateUI(bool s1, bool s2, bool f1, bool f2, const QString &ps1, const QString &ps2, const QString &pf1, const QString &pf2)
{
    ui->searchLineEdit1->setVisible(s1);
    ui->searchLineEdit1->setPlaceholderText(ps1);
    ui->searchLineEdit2->setVisible(s2);
    ui->searchLineEdit2->setPlaceholderText(ps2);
    ui->filterLineEdit1->setVisible(f1);
    ui->filterLineEdit1->setPlaceholderText(pf1);
    ui->filterLineEdit2->setVisible(f2);
    ui->filterLineEdit2->setPlaceholderText(pf2);

    ui->searchLineEdit1->clear();
    ui->searchLineEdit2->clear();
    ui->filterLineEdit1->clear();
    ui->filterLineEdit2->clear();
}

void MainWindow::switchToTable(const QString &tableName, const QString &title) {

    if (ui->tableView->currentIndex().isValid()) {
            ui->tableView->setCurrentIndex(QModelIndex());
        }

    if (tableManager && tableManager->getModel()) {
            // Проверяем, есть ли вообще изменения, чтобы не спамить "No Fields to update"
        if (tableManager->getModel()->isDirty()) {
            if (!tableManager->getModel()->submitAll()) {
                qDebug() << "Не удалось сохранить изменения при переключении:" << tableManager->getModel()->lastError().text();
                tableManager->getModel()->revertAll();
            }
        }
    }
    currentTable = tableName;
    ui->label->setText(title);
    tableManager->setupTable(tableName, ui->tableView);
    auto model = tableManager->getModel();

    // Сбрасываем делегаты для 4-й колонки (чтобы они не накладывались)
    ui->tableView->setItemDelegateForColumn(3, nullptr);
    ui->tableView->setItemDelegateForColumn(4, nullptr);

    if (tableName == "users") {
        model->setHeaderData(1, Qt::Horizontal, "Логин");
        model->setHeaderData(2, Qt::Horizontal, "Пароль");
        model->setHeaderData(3, Qt::Horizontal, "ФИО");
        model->setHeaderData(4, Qt::Horizontal, "Роль");
        ui->labelFilters->hide();
    }
    else if (tableName == "groups") {
        ui->tableView->setItemDelegateForColumn(3, new RelationComboBoxDelegate("users", "full_name", "role='trainer'", this));
        model->setHeaderData(1, Qt::Horizontal, "Название группы");
        model->setHeaderData(2, Qt::Horizontal, "Направление");
        model->setHeaderData(3, Qt::Horizontal, "Тренер");
        model->setHeaderData(4, Qt::Horizontal, "Доб. учеников");
        ui->labelFilters->show();

        ButtonDelegate *btnDel = new ButtonDelegate(this);
        ui->tableView->setItemDelegateForColumn(4, btnDel);

        // Пример обработки нажатия
        connect(btnDel, &ButtonDelegate::buttonClicked, this, [this](const QModelIndex &index){
            int row = index.row();
            qDebug() << "Нажата кнопка в строке:" << row;
        });
    }
    else if (tableName == "schedule") {
        for(int i=1; i<=5; ++i) ui->tableView->setItemDelegateForColumn(i, nullptr);

            // 2. Колонка 1: Группа (используем наш Relation делегат, чтобы был список имен вместо ID)
        ui->tableView->setItemDelegateForColumn(1, new RelationComboBoxDelegate("groups", "name", "", this));

            // 3. Колонка 2: День недели
        QStringList days = {"Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
        ui->tableView->setItemDelegateForColumn(2, new FixedListDelegate(days, this));

            // 4. Колонка 3 и 4: Время начала и окончания
        ui->tableView->setItemDelegateForColumn(3, new TimeEditDelegate(this));
        ui->tableView->setItemDelegateForColumn(4, new TimeEditDelegate(this));

        model->setHeaderData(1, Qt::Horizontal, "Название группы");
        model->setHeaderData(2, Qt::Horizontal, "День недели");
        model->setHeaderData(3, Qt::Horizontal, "Вр. начала");
        model->setHeaderData(4, Qt::Horizontal, "Вр. окончания");
        model->setHeaderData(5, Qt::Horizontal, "Зал");
        ui->labelFilters->hide();
    }
    else if (tableName == "attendance") {
        model->setHeaderData(1, Qt::Horizontal, "ФИО");
        model->setHeaderData(2, Qt::Horizontal, "Название группы");
        model->setHeaderData(3, Qt::Horizontal, "Дата занятия");
        model->setHeaderData(4, Qt::Horizontal, "Статус");
        ui->labelFilters->show();

        ui->tableView->setItemDelegateForColumn(4, new CheckBoxDelegate(this));
    }

    // Скрываем колонку ID (она всегда 0-я во всех таблицах)
    ui->tableView->hideColumn(0);

    reloadview();
}

void MainWindow::on_pushButton_2_clicked() { // ПОЛЬЗОВАТЕЛИ
    switchToTable("users", "Пользователи");
    // Только Поиск ФИО
    updateUI(true, false, false, false, "ФИО", "", "", "");
}

void MainWindow::on_pushButton_clicked() { // ГРУППЫ
    switchToTable("groups", "Группы");
    updateUI(true, false, true, true, "Название группы", "", "Тренер", "Направление");
}

void MainWindow::on_pushButton_3_clicked() { // РАСПИСАНИЕ
    switchToTable("schedule", "Расписание");
    updateUI(true, true, false, false, "Группа", "День недели", "", "");
}

void MainWindow::on_pushButton_4_clicked() { // ПОСЕЩАЕМОСТЬ
    switchToTable("attendance", "Посещаемость");
    updateUI(true, true, true, false, "Ученик", "Дата", "Группа", "");
}

void MainWindow::on_addButton_clicked() {
    if (currentTable == "attendance") {
            // Создаем объект вашего нового класса
        AttendanceAddDialog dialog(this);

            // exec() открывает окно как модальное и блокирует основное окно
        if (dialog.exec() == QDialog::Accepted) {
                // Сюда попадем, если в диалоге нажали "Сохранить" (accept)
            qDebug() << "Записи успешно сформированы (логика будет позже)";
            reloadview();
        }
    }
    else
    {
        auto model = tableManager->getModel();
        if (!model) return;

            // Временно отключаем сортировку, чтобы строка не прыгала
        ui->tableView->setSortingEnabled(false);

        if (model->insertRow(0)) {
                // ПРЕДЗАПОЛНЕНИЕ: Если это таблица пользователей, ставим роль по умолчанию,
                // чтобы избежать ошибки CHECK constraint сразу при создании.
        if (currentTable == "users") {
            model->setData(model->index(0, 4), "student");
        }

                // Вычисляем индекс в прокси-модели
        QModelIndex proxyIndex = tableManager->getProxyModel()->mapFromSource(model->index(0, 1));

        ui->tableView->scrollToTop();
        ui->tableView->setCurrentIndex(proxyIndex);
        ui->tableView->edit(proxyIndex); // Сразу открываем поле для ввода
        }

        ui->tableView->setSortingEnabled(true);
    }

}

void MainWindow::on_logoutButton_clicked()
{
    logoutRequested = true;
    this->close();
}

void MainWindow::ensureTrailingEmptyRow() {
    auto model = tableManager->getModel();
    if (model && (model->rowCount() == 0 || isRowFilled(model->rowCount()-1))) {
        model->insertRow(model->rowCount());
    }
}

bool MainWindow::isRowFilled(int row) const {
    auto model = tableManager->getModel();
    // Список обязательных полей берем из вашей схемы БД
    QStringList fields;
    if (currentTable == "users") fields << "login" << "full_name" << "role";
    else if (currentTable == "groups") fields << "name" << "direction";
    else if (currentTable == "attendance") fields << "lesson_date" << "status";

    for (const QString &f : fields) {
        if (model->data(model->index(row, model->fieldIndex(f))).toString().isEmpty())
            return false;
    }
    return true;
}

void MainWindow::reloadview() {
    ui->tableView->resizeColumnsToContents();
    ensureTrailingEmptyRow();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
    auto model = tableManager->getModel();
    if (!model || !model->isDirty()) return;

    int row = topLeft.row();
    int col = topLeft.column();

    // Блокируем сигналы, чтобы наши манипуляции не вызывали рекурсию
    model->blockSignals(true);

    // Обработка Тренера в Группах
    if (currentTable == "groups" && col == 3) {
        QString val = model->index(row, 3).data(Qt::DisplayRole).toString();
        bool isNumeric;
        val.toInt(&isNumeric);
        if (!isNumeric) { // Если в ячейке текст, а не ID
            QSqlQuery query;
            query.prepare("SELECT id FROM users WHERE full_name = :name AND role = 'trainer'");
            query.bindValue(":name", val);
            if (query.exec() && query.next()) {
                model->setData(model->index(row, 3), query.value(0).toInt(), Qt::EditRole);
            }
        }
    }
    // Обработка Группы в Расписании
    else if (currentTable == "schedule" && col == 1) {
        QString val = model->index(row, 1).data(Qt::DisplayRole).toString();
        bool isNumeric;
        val.toInt(&isNumeric);
        if (!isNumeric) {
            QSqlQuery query;
            query.prepare("SELECT id FROM groups WHERE name = :name");
            query.bindValue(":name", val);
            if (query.exec() && query.next()) {
                model->setData(model->index(row, 1), query.value(0).toInt(), Qt::EditRole);
            }
        }
    }

    model->blockSignals(false);

    // Определение лимита для сохранения
    int lastRequiredCol = 3;
    if (currentTable == "schedule") lastRequiredCol = 5;

    bool allFilled = true;
    for (int i = 1; i <= lastRequiredCol; ++i) {
        if (model->index(row, i).data().toString().trimmed().isEmpty()) {
            allFilled = false;
            break;
        }
    }

    if (allFilled) {
        if (model->submitAll()) {
            // МГНОВЕННОЕ ОБНОВЛЕНИЕ
            model->select();
            // Скрываем ID и возвращаем делегаты
            setupDelegatesForCurrentTable();
        }
    }
}

void MainWindow::setupDelegatesForCurrentTable() {
    ui->tableView->hideColumn(0); // Скрываем ID

    if (currentTable == "groups") {
        ui->tableView->setItemDelegateForColumn(3, new RelationComboBoxDelegate("users", "full_name", "role='trainer'", this));
        ui->tableView->setItemDelegateForColumn(4, new ButtonDelegate(this));
    }
    else if (currentTable == "schedule") {
        ui->tableView->setItemDelegateForColumn(1, new RelationComboBoxDelegate("groups", "name", "", this));
        QStringList days = {"Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
        ui->tableView->setItemDelegateForColumn(2, new FixedListDelegate(days, this));
        ui->tableView->setItemDelegateForColumn(3, new TimeEditDelegate(this));
        ui->tableView->setItemDelegateForColumn(4, new TimeEditDelegate(this));
    }
}

void MainWindow::setupGroupDelegates() {
    // Делегат для выбора ТРЕНЕРОВ
    RelationComboBoxDelegate *coachDel = new RelationComboBoxDelegate("users", "full_name", "role='trainer'", this);
    ui->tableView->setItemDelegateForColumn(3, coachDel);

    // Делегат для кнопки
    ButtonDelegate *btnDel = new ButtonDelegate(this);
    ui->tableView->setItemDelegateForColumn(4, btnDel);
}

void MainWindow::commitLastRow() {
    auto model = tableManager->getModel();
    if (!model || model->rowCount() == 0) return;

    if (model->submitAll()) {
        ensureTrailingEmptyRow();
        ui->tableView->resizeColumnsToContents();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Проверяем, что событие пришло от таблицы и это нажатие клавиши
    if (obj == ui->tableView && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // Если нажат Enter или Return
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {

            // Если мы сейчас редактируем ячейку, закрываем редактор и сохраняем
            QModelIndex cur = ui->tableView->currentIndex();
            if (cur.isValid()) {
                ui->tableView->closePersistentEditor(cur);
                commitLastRow();
                return true; // Событие обработано
            }
        }
    }

    // В остальных случаях отдаем событие стандартному обработчику
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::on_anySearchField_changed() {
    QMap<int, QString> filters;

    // Логируем для отладки, чтобы видеть, какой метод вызван
    qDebug() << "Фильтрация для таблицы:" << currentTable;

    if (currentTable == "users") {
        // Таблица: id(0), login(1), pass(2), full_name(3), role(4)
        QString fio = ui->searchLineEdit1->text().trimmed();
        if (!fio.isEmpty()) {
            filters.insert(3, fio);
        }
    }
    else if (currentTable == "groups") {
        // Таблица: id(0), name(1), specialization(2), trainer_id(3)
        // ВАЖНО: Каждое поле проверяется отдельно (без else)

        // 1. Поиск по Названию группы (Колонка 1)
        QString groupName = ui->searchLineEdit1->text().trimmed();
        if (!groupName.isEmpty()) {
            filters.insert(1, groupName);
        }

        // 2. Поиск по Направлению/Специализации (Колонка 2)
        // Проверьте, что в дизайнере это поле называется searchLineEdit2
        QString direction = ui->searchLineEdit2->text().trimmed();
        if (!direction.isEmpty()) {
            filters.insert(2, direction);
        }

        // 3. Поиск по ФИО Тренера (Колонка 3)
        // Обычно это поле фильтрации в выпадающем списке или отдельный lineEdit
        QString trainer = ui->filterLineEdit1->text().trimmed();
        if (!trainer.isEmpty()) {
            filters.insert(3, trainer);
        }
    }
    else if (currentTable == "schedule") {
        // Таблица: id(0), group_id(1), day_of_week(2), ...
        QString group = ui->searchLineEdit1->text().trimmed();
        if (!group.isEmpty()) {
            filters.insert(1, group);
        }

        QString day = ui->searchLineEdit2->text().trimmed();
        if (!day.isEmpty()) {
            filters.insert(2, day);
        }
    }
    else if (currentTable == "attendance") {
        // Таблица: id(0), student_id(1), group_id(2), date(3), status(4)
        QString student = ui->searchLineEdit1->text().trimmed();
        if (!student.isEmpty()) {
            filters.insert(1, student);
        }

        QString group = ui->searchLineEdit2->text().trimmed();
        if (!group.isEmpty()) {
            filters.insert(2, group);
        }
    }

    // Проверяем в консоли, что мы собрали
    qDebug() << "Итоговая карта фильтров:" << filters;

    // Отправляем карту в TableManager
    if (tableManager) {
        tableManager->applyMultiFilter(filters);
    }
}
