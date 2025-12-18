#include <QKeyEvent>
#include <QEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMap>

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

void MainWindow::updateUI(bool s1, bool s2, bool f1, bool f2, const QString &ps1, const QString &ps2, const QString &pf1, const QString &pf2)
{
    ui->searchLineEdit1->setVisible(s1); ui->searchLineEdit1->setPlaceholderText(ps1);
    ui->searchLineEdit2->setVisible(s2); ui->searchLineEdit2->setPlaceholderText(ps2);
    ui->filterLineEdit1->setVisible(f1); ui->filterLineEdit1->setPlaceholderText(pf1);
    ui->filterLineEdit2->setVisible(f2); ui->filterLineEdit2->setPlaceholderText(pf2);

    ui->searchLineEdit1->clear(); ui->searchLineEdit2->clear();
    ui->filterLineEdit1->clear(); ui->filterLineEdit2->clear();
}

void MainWindow::switchToTable(const QString &tableName, const QString &title) {
    currentTable = tableName;
    // Удаляем упоминание currentSearchField и ui->searchLineEdit,
    // так как теперь у нас searchLineEdit1 и 2

    ui->label->setText(title);
    tableManager->setupTable(tableName, ui->tableView);

    // Переподключаем сигнал изменений для новой модели
    connect(tableManager->getModel(), &QSqlTableModel::dataChanged,
            this, &MainWindow::onModelDataChanged);

    reloadview();
    ui->tableView->hideColumn(0); // Скрываем ID
}

void MainWindow::on_pushButton_2_clicked() { // ПОЛЬЗОВАТЕЛИ
    switchToTable("users", "Пользователи");
    // Только Поиск ФИО
    updateUI(true, false, false, false, "ФИО", "", "", "");
}

void MainWindow::on_pushButton_clicked() { // ГРУППЫ
    switchToTable("groups", "Группы");
    // Поиск: Название. Фильтр: Тренер (ID), Направление
    updateUI(true, false, true, true, "Название группы", "", "ID Тренера", "Направление");
}

void MainWindow::on_pushButton_3_clicked() { // РАСПИСАНИЕ
    switchToTable("schedule", "Расписание");
    // Поиск: Группа (ID), День недели. Фильтров нет.
    updateUI(true, true, false, false, "ID Группы", "День недели", "", "");
}

void MainWindow::on_pushButton_4_clicked() { // ПОСЕЩАЕМОСТЬ
    switchToTable("attendance", "Посещаемость");
    // Поиск: ФИО (ID студента), Дата. Фильтр: Тренер (ID группы)
    updateUI(true, true, true, false, "ID Студента", "Дата", "ID Группы", "");
}

void MainWindow::on_addButton_clicked()
{
    if(ui->label->text() == "attendance")
    {
        //проверка, т к будет всплывающее окно
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
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)

    auto model = tableManager->getModel();
    if (!model) return;

    int last = model->rowCount() - 1;
    if (last >= 0 && isRowFilled(last)) {
        commitLastRow();
    }
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
    QMap<QString, QString> filters;

    if (currentTable == "users") {
        filters["full_name"] = ui->searchLineEdit1->text();
    }
    else if (currentTable == "groups") {
        filters["name"] = ui->searchLineEdit1->text();
        filters["trainer_id"] = ui->filterLineEdit1->text();
        filters["direction"] = ui->filterLineEdit2->text();
    }
    else if (currentTable == "schedule") {
        filters["group_id"] = ui->searchLineEdit1->text();
        filters["day_of_week"] = ui->searchLineEdit2->text();
    }
    else if (currentTable == "attendance") {
        filters["student_id"] = ui->searchLineEdit1->text();
        filters["lesson_date"] = ui->searchLineEdit2->text();
        filters["group_id"] = ui->filterLineEdit1->text();
    }

    tableManager->applyMultiFilter(filters);
}
