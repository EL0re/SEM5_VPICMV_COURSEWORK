#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableView->setSortingEnabled(true);
    QSqlDatabase mydb = QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName("C:/Users/XE4/Desktop/sports_club.db");
    model = new QSqlTableModel(this);
    model->setTable("users");
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    reloadview();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    model = new QSqlTableModel(this, QSqlDatabase::database());
    model->setTable("groups");
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    reloadview();
}

void MainWindow::on_pushButton_2_clicked()
{
    model = new QSqlTableModel(this, QSqlDatabase::database());
    model->setTable("users");
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    reloadview();
}

void MainWindow::ensureTrailingEmptyRow()
{
    if (!model) return;
    int rows = model->rowCount();
    // Если таблица пуста или последняя строка НЕ пустая — добавляем пустую
    if (rows == 0 || isRowFilled(rows - 1)) {
        bool ok = model->insertRow(rows);
        if (!ok) {
            qWarning() << "Не удалось вставить пустую строку";
        }
    }
}

bool MainWindow::isRowFilled(int row) const
{
    if (!model) return false;

    // Перечислим обязательные поля, которые должны быть заполнены:
    // адаптируй имена полей под свою схему: "login","password","full_name","role"
    QStringList required = {"login", "password", "full_name", "role"};

    for (const QString &fieldName : required) {
        int col = model->fieldIndex(fieldName);
        if (col < 0) continue; // если поле не найдено — пропускаем
        QVariant v = model->data(model->index(row, col));
        if (!v.isValid() || v.toString().trimmed().isEmpty()) {
            return false;
        }
    }
    return true;
}

void MainWindow::reloadview(){
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
    ensureTrailingEmptyRow();
}
