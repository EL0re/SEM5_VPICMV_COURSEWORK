#include <QKeyEvent>
#include <QEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMap>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>

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
        connect(btnDel, &ButtonDelegate::buttonClicked, this, [this](int row){
            qDebug() << "Нажата кнопка в строке:" << row;
        });
    }
    else if (tableName == "schedule") {
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
        qDebug() << "Открытие окна посещаемости...";
        return;
    }

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

    // Проверка заполненности (Название, Направление, Тренер)
    bool allFilled = true;
    for (int col = 1; col <= 3; ++col) {
        // Проверяем значение в модели
        QVariant val = model->index(row, col).data();
        if (val.toString().trimmed().isEmpty() || val.isNull()) {
            allFilled = false;
            break;
        }
    }

    if (allFilled) {
        // Пытаемся сохранить
        if (model->submitAll()) {
            qDebug() << "Успешное сохранение в базу данных.";
            model->select(); // Обновляем, чтобы увидеть ФИО вместо ID

            if (currentTable == "groups") {
                model->insertColumn(4);
                model->setHeaderData(4, Qt::Horizontal, "Действие");

                // Переназначаем делегаты, так как select() их сбрасывает
                ui->tableView->setItemDelegateForColumn(3,
                    new RelationComboBoxDelegate("users", "full_name", "role='trainer'", this));
                ui->tableView->setItemDelegateForColumn(4, new ButtonDelegate(this));
                ui->tableView->hideColumn(0);
            }
        } else {
            qDebug() << "Ошибка сохранения:" << model->lastError().text();
            // Если ошибка NOT NULL, значит ID всё равно не дошел.
            // Но с новым делегатом этой ошибки быть не должно.
        }
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

    if (currentTable == "users") {
        // Таблица: id(0), login(1), pass(2), full_name(3)
        filters[3] = ui->searchLineEdit1->text();
    }
    else if (currentTable == "groups") {
        // id(0), name(1), dir(2), trainer(3) - тут trainer подменяется на ФИО
        if (!ui->searchLineEdit1->text().isEmpty())
            filters[1] = ui->searchLineEdit1->text(); // Поиск по названию
        else
            filters[3] = ui->filterLineEdit1->text(); // Фильтр по имени тренера
    }
    else if (currentTable == "schedule") {
        // id(0), group(1) - подменяется на имя, day(2)
        if (!ui->searchLineEdit1->text().isEmpty())
            filters[1] = ui->searchLineEdit1->text(); // Имя группы
        else
            filters[2] = ui->searchLineEdit2->text(); // День недели
    }
    else if (currentTable == "attendance") {
        // id(0), student(1) - подменяется на ФИО, group(2) - на имя
        if (!ui->searchLineEdit1->text().isEmpty())
            filters[1] = ui->searchLineEdit1->text(); // ФИО студента
        else
            filters[2] = ui->searchLineEdit2->text(); // Имя группы
    }

    tableManager->applyMultiFilter(filters);
}
