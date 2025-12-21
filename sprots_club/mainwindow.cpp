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
#include "studentsortproxymodel.h"
#include <QSqlRecord>

MainWindow::MainWindow(const QString &fullName, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tableManager = new TableManager(this);

    ui->FIO_Label->setText(fullName);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

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
    // 1. СНАЧАЛА СОХРАНЯЕМ: Если есть изменения в текущей таблице, записываем их перед переходом
    if (tableManager && tableManager->getModel()) {
        auto m = tableManager->getModel();
        if (m->isDirty()) {
            if (!m->submitAll()) {
                qDebug() << "Ошибка сохранения при переходе:" << m->lastError().text();
                m->revertAll();
            }
        }
    }

    // 2. Сбрасываем фокус
    if (ui->tableView->currentIndex().isValid()) {
        ui->tableView->setCurrentIndex(QModelIndex());
    }

    currentTable = tableName;
    ui->label->setText(title);

    // 3. Настраиваем базовую структуру через менеджер
    tableManager->setupTable(tableName, ui->tableView);

    auto model = tableManager->getModel();
    if (!model) return;

    // 4. НАКЛАДЫВАЕМ ДЕЛЕГАТЫ
    if (tableName == "users") {
        model->setHeaderData(1, Qt::Horizontal, "Логин");
        model->setHeaderData(2, Qt::Horizontal, "Пароль");
        model->setHeaderData(3, Qt::Horizontal, "ФИО");
        model->setHeaderData(4, Qt::Horizontal, "Роль");

        QStringList roles = {"admin", "trainer", "student"};
        ui->tableView->setItemDelegateForColumn(4, new FixedListDelegate(roles, this));

        ui->labelFilters->hide();
        ui->slashLabel->hide();
        ui->importButton->hide();
        ui->exportButton->hide();
    }
    else if (tableName == "groups") {
        ui->tableView->setItemDelegateForColumn(3, new RelationComboBoxDelegate("users", "full_name", "role='trainer'", this));
        model->setHeaderData(1, Qt::Horizontal, "Название группы");
        model->setHeaderData(2, Qt::Horizontal, "Направление");
        model->setHeaderData(3, Qt::Horizontal, "Тренер");
        model->setHeaderData(4, Qt::Horizontal, "Состав");

        ui->labelFilters->show();
        ui->slashLabel->hide();
        ui->importButton->hide();
        ui->exportButton->hide();
    }
    else if (tableName == "schedule") {
        ui->tableView->setItemDelegateForColumn(1, new RelationComboBoxDelegate("groups", "name", "", this));
        QStringList days = {"Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
        ui->tableView->setItemDelegateForColumn(2, new FixedListDelegate(days, this));
        ui->tableView->setItemDelegateForColumn(3, new TimeEditDelegate(this));
        ui->tableView->setItemDelegateForColumn(4, new TimeEditDelegate(this));

        model->setHeaderData(1, Qt::Horizontal, "Название группы");
        model->setHeaderData(2, Qt::Horizontal, "День недели");
        model->setHeaderData(3, Qt::Horizontal, "Вр. начала");
        model->setHeaderData(4, Qt::Horizontal, "Вр. окончания");
        model->setHeaderData(5, Qt::Horizontal, "Зал");

        ui->labelFilters->hide();
        ui->slashLabel->show();
        ui->importButton->show();
        ui->exportButton->show();
    }
    else if (tableName == "attendance") {
        model->setHeaderData(1, Qt::Horizontal, "ФИО");
        model->setHeaderData(2, Qt::Horizontal, "Название группы");
        model->setHeaderData(3, Qt::Horizontal, "Дата занятия");
        model->setHeaderData(4, Qt::Horizontal, "Статус");

        ui->tableView->setItemDelegateForColumn(1, new RelationComboBoxDelegate("users", "full_name", "role='student'", this));
        ui->tableView->setItemDelegateForColumn(2, new RelationComboBoxDelegate("groups", "name", "", this));
        ui->tableView->setItemDelegateForColumn(4, new CheckBoxDelegate(this));

        ui->labelFilters->show();
        ui->slashLabel->show();
        ui->importButton->show();
        ui->exportButton->show();
    }

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
                    // ВАЖНО: После того как диалог закрыт с результатом Accepted,
                    // записи уже в базе данных. Нужно обновить модель, чтобы их увидеть.
            if (tableManager && tableManager->getModel()) {
                tableManager->getModel()->select();
            }
            reloadview();
            qDebug() << "Записи посещаемости успешно добавлены и отображены.";
        }
    }
    else
    {
        auto model = tableManager->getModel();
        if (!model) return;

        ui->tableView->setSortingEnabled(false);

        if (model->insertRow(0)) {

            QModelIndex proxyIndex = tableManager->getProxyModel()->mapFromSource(model->index(0, 1));

            ui->tableView->scrollToTop();
            ui->tableView->setCurrentIndex(proxyIndex);
            ui->tableView->edit(proxyIndex); // Сразу открываем поле для ввода
        }

        ui->tableView->setSortingEnabled(true);
    }

}

void MainWindow::on_delButton_clicked() {
    QItemSelectionModel *select = ui->tableView->selectionModel();
    QModelIndexList selectedRows = select->selectedRows();

    if (selectedRows.isEmpty()) {
        QModelIndex current = ui->tableView->currentIndex();
        if (current.isValid()) selectedRows.append(current);
        else return;
    }

    auto model = tableManager->getModel();
    auto proxy = tableManager->getProxyModel();

    // --- ЗАЩИТА АДМИНИСТРАТОРА (ВСТАВЛЕНО СЮДА) ---
    if (currentTable == "users") {
        QString currentUserFio = ui->FIO_Label->text().trimmed();
        for (const QModelIndex &proxyIdx : selectedRows) {
            QModelIndex sourceIdx = proxy->mapToSource(proxyIdx);
            int r = sourceIdx.row();
            // 3 - ФИО, 4 - Роль
            QString targetFio = model->data(model->index(r, 3)).toString().trimmed();
            QString targetRole = model->data(model->index(r, 4)).toString().trimmed();

            if (targetFio == currentUserFio && targetRole == "admin") {
                QMessageBox::warning(this, "Защита", "Вы не можете удалить свою учетную запись администратора!");
                return; // Прерываем выполнение метода полностью
            }
        }
    }

    // Только если защита пройдена, спрашиваем подтверждение
    auto res = QMessageBox::question(this, "Подтверждение",
        "Удалить выбранные записи?\nЭто вызовет каскадное удаление всех связанных данных!",
        QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) return;

    // 1. Собираем ID записей
    QList<int> idsToDelete;
    int idColumn = model->record().indexOf("id");
    if (idColumn == -1) idColumn = 0;

    for (const QModelIndex &proxyIdx : selectedRows) {
        QModelIndex sourceIdx = proxy->mapToSource(proxyIdx);
        int row = sourceIdx.row();
        if (row < model->rowCount()) {
            int id = model->data(model->index(row, idColumn)).toInt();
            if (id > 0) idsToDelete.append(id);
        }
    }

    if (idsToDelete.isEmpty()) return;

    // 2. Выполняем удаление через SQL
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery query;
    query.exec("PRAGMA foreign_keys = ON;");

    bool success = true;
    for (int id : idsToDelete) {
        query.prepare(QString("DELETE FROM %1 WHERE id = ?").arg(currentTable));
        query.addBindValue(id);
        if (!query.exec()) {
            success = false;
            break;
        }
    }

    if (success && db.commit()) {
        model->select();
//        ensureTrailingEmptyRow();
    } else {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось удалить записи: " + query.lastError().text());
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
    if (currentTable == "users") fields << "login" << "password" << "full_name" << "role";
    else if (currentTable == "groups") fields << "name" << "direction";
    else if (currentTable == "attendance") fields << "lesson_date" << "status";

    for (const QString &f : fields) {
        int idx = model->fieldIndex(f);
        if (idx == -1) continue;
        if (model->data(model->index(row, idx)).toString().trimmed().isEmpty())
            return false;
    }
    return true;
}

void MainWindow::reloadview() {
    ui->tableView->resizeColumnsToContents();
//    ensureTrailingEmptyRow();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
    auto model = qobject_cast<QSqlRelationalTableModel*>(tableManager->getModel());
    if (!model || !model->isDirty()) return;

    int row = topLeft.row();
    int col = topLeft.column();

    // --- ПРОВЕРКА ДУБЛИКАТА ФИО ---
    if (currentTable == "users" && col == 3) {
        QString name = model->index(row, 3).data().toString().trimmed();
        int currentId = model->index(row, 0).data().toInt();
        if (!name.isEmpty()) {
            QSqlQuery check;
            check.prepare("SELECT id FROM users WHERE full_name = :n AND id != :id");
            check.bindValue(":n", name);
            check.bindValue(":id", currentId);
            if (check.exec() && check.next()) {
                QMessageBox::warning(this, "Дубликат", "Пользователь с таким ФИО уже есть в базе.");
                model->revertAll();
                model->select();
                return;
            }
        }
    }

    model->blockSignals(true);

    // 1. ПРЕОБРАЗОВАНИЕ ТЕКСТА В ID
    if (currentTable == "groups" && col == 3) {
        QString val = model->index(row, 3).data(Qt::DisplayRole).toString();
        bool isNumeric;
        val.toInt(&isNumeric);
        if (!isNumeric) {
            QSqlQuery query;
            query.prepare("SELECT id FROM users WHERE full_name = :name AND role = 'trainer'");
            query.bindValue(":name", val);
            if (query.exec() && query.next()) {
                model->setData(model->index(row, 3), query.value(0).toInt(), Qt::EditRole);
            }
        }
    }
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

    // 2. ОПРЕДЕЛЕНИЕ ГРАНИЦ ЗАПОЛНЕННОСТИ
    int lastRequiredCol = (currentTable == "users") ? 4 : 3;
    if (currentTable == "schedule") lastRequiredCol = 5;
    else if (currentTable == "attendance") lastRequiredCol = 4;

    bool allFilled = true;
    for (int i = 1; i <= lastRequiredCol; ++i) {
        if (model->index(row, i).data().toString().trimmed().isEmpty()) {
            allFilled = false;
            break;
        }
    }

    // Если строка заполнена, выполняем валидацию
    if (allFilled) {

        // --- ПРОВЕРКА ДЛЯ ТАБЛИЦЫ ГРУПП (Уникальность названия) ---
        if (currentTable == "groups") {
            int currentId = model->index(row, 0).data().toInt();
            QString groupName = model->index(row, 1).data().toString().trimmed();

            QSqlQuery checkName;
            checkName.prepare("SELECT id FROM groups WHERE name = :name AND id != :id");
            checkName.bindValue(":name", groupName);
            checkName.bindValue(":id", currentId);

            if (checkName.exec() && checkName.next()) {
                QMessageBox::warning(this, "Дубликат", QString("Группа с названием '%1' уже существует!").arg(groupName));
                model->revertAll();
                return;
            }
        }

        // --- ПРОВЕРКИ ДЛЯ ТАБЛИЦЫ РАСПИСАНИЯ ---
        if (currentTable == "schedule") {
            int currentId = model->index(row, 0).data().toInt();
            int groupId = model->index(row, 1).data(Qt::EditRole).toInt();
            QString day = model->index(row, 2).data().toString().trimmed();
            QString start = model->index(row, 3).data().toString().trimmed();
            QString end = model->index(row, 4).data().toString().trimmed();
            QString hall = model->index(row, 5).data().toString().trimmed();

            // Проверка логики времени
            if (start >= end) {
                QMessageBox::warning(this, "Ошибка времени", "Время начала занятия должно быть раньше времени окончания!");
                model->revertAll();
                return;
            }

            // ПРОВЕРКА 1: Конфликт по залу
            QSqlQuery qHall;
            qHall.prepare(
                "SELECT g.name FROM schedule s "
                "JOIN groups g ON s.group_id = g.id "
                "WHERE s.day_of_week = :day AND s.hall = :hall AND s.id != :id "
                "AND (time(:start) < time(s.end_time) AND time(:end) > time(s.start_time))"
            );
            qHall.bindValue(":day", day);
            qHall.bindValue(":hall", hall);
            qHall.bindValue(":start", start);
            qHall.bindValue(":end", end);
            qHall.bindValue(":id", currentId);

            if (qHall.exec() && qHall.next()) {
                QMessageBox::critical(this, "Конфликт зала",
                    QString("Зал '%1' уже занят в это время группой: %2").arg(hall, qHall.value(0).toString()));
                model->revertAll();
                return;
            }

            // ПРОВЕРКА 2: Конфликт по тренеру
            QSqlQuery qGetT;
            qGetT.prepare("SELECT trainer_id FROM groups WHERE id = :gid");
            qGetT.bindValue(":gid", groupId);

            if (qGetT.exec() && qGetT.next()) {
                int trainerId = qGetT.value(0).toInt();

                QSqlQuery qTr;
                qTr.prepare(
                    "SELECT g.name FROM schedule s "
                    "JOIN groups g ON s.group_id = g.id "
                    "WHERE s.day_of_week = :day AND g.trainer_id = :tid AND s.id != :id "
                    "AND (time(:start) < time(s.end_time) AND time(:end) > time(s.start_time))"
                );
                qTr.bindValue(":day", day);
                qTr.bindValue(":tid", trainerId);
                qTr.bindValue(":start", start);
                qTr.bindValue(":end", end);
                qTr.bindValue(":id", currentId);

                if (qTr.exec() && qTr.next()) {
                    QMessageBox::critical(this, "Конфликт тренера",
                        QString("Выбранный тренер в это время ведет занятие у группы: %1").arg(qTr.value(0).toString()));
                    model->revertAll();
                    return;
                }
            }
        }

        // Если все проверки пройдены, сохраняем изменения в базу
        if (model->submitAll()) {
            model->select();
            // Скрываем ID и восстанавливаем делегаты
            setupDelegatesForCurrentTable();
        } else {
            qDebug() << "Ошибка сохранения данных:" << model->lastError().text();
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
        if (keyEvent->key() == Qt::Key_Delete) {
            on_delButton_clicked(); // Вызываем тот же метод, что и у кнопки
            return true;
        }
    }

    // В остальных случаях отдаем событие стандартному обработчику
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::on_anySearchField_changed() {
    QMap<int, QString> filters;

    if (currentTable == "users") {
        // Таблица: id(0), login(1), pass(2), full_name(3), role(4)
        QString fio = ui->searchLineEdit1->text().trimmed();
        if (!fio.isEmpty()) filters.insert(3, fio);
    }
    else if (currentTable == "groups") {
        // Таблица: id(0), name(1), direction(2), trainer_id(3)
        QString groupName = ui->searchLineEdit1->text().trimmed();
        if (!groupName.isEmpty()) filters.insert(1, groupName);

        // Направление — это searchLineEdit2 (по вашему updateUI для Групп)
        QString direction = ui->filterLineEdit2->text().trimmed();
        if (!direction.isEmpty()) filters.insert(2, direction);

        // Тренер — это filterLineEdit1
        QString trainer = ui->filterLineEdit1->text().trimmed();
        if (!trainer.isEmpty()) filters.insert(3, trainer);
    }
    else if (currentTable == "schedule") {
        // Таблица: id(0), group_id(1), day_of_week(2)
        QString group = ui->searchLineEdit1->text().trimmed();
        if (!group.isEmpty()) filters.insert(1, group);

        QString day = ui->searchLineEdit2->text().trimmed();
        if (!day.isEmpty()) filters.insert(2, day);
    }
    else if (currentTable == "attendance") {
        // Таблица: id(0), student_id(1), group_id(2), date(3), status(4)

        // 1. Ученик (Колонка 1) - searchLineEdit1
        QString student = ui->searchLineEdit1->text().trimmed();
        if (!student.isEmpty()) filters.insert(1, student);

        // 2. Дата (Колонка 3) - searchLineEdit2
        // ВНИМАНИЕ: Здесь была ошибка в индексе (было 2, стало 3)
        QString date = ui->searchLineEdit2->text().trimmed();
        if (!date.isEmpty()) filters.insert(3, date);

        // 3. Группа (Колонка 2) - filterLineEdit1
        QString group = ui->filterLineEdit1->text().trimmed();
        if (!group.isEmpty()) filters.insert(2, group);
    }

    if (tableManager) {
        tableManager->applyMultiFilter(filters);
    }
}
