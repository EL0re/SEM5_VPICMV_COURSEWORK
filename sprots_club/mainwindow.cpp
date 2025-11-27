#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(const QString &fullName, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->installEventFilter(this);
    QPixmap pixmap("C:/Users/XE4/Desktop/xui.png");
    ui->imagelabel->setPixmap(pixmap);
    ui->imagelabel->setPixmap(pixmap.scaled(ui->imagelabel->width(),
                                          ui->imagelabel->height(),
                                          Qt::KeepAspectRatio));
    ui->FIO_Label->setText(fullName);
    QSqlDatabase mydb = QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName("C:/Users/XE4/Desktop/sports_club.db");
    model = new QSqlTableModel(this);
    model->setTable("users");
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    connect(model, &QSqlTableModel::dataChanged,
                this, &MainWindow::onModelDataChanged);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    reloadview();
    ui->tableView->setModel(proxyModel);
    ui->tableView->hideColumn(0);
    ui->searchLineEdit->setPlaceholderText("Поиск по ФИО");
    model = new QSqlTableModel(this, QSqlDatabase::database());
    model->setTable("users");
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    connect(model, &QSqlTableModel::dataChanged,
            this, &MainWindow::onModelDataChanged);

    proxyModel->setSourceModel(model);

    int fullNameCol = model->fieldIndex("full_name");
    if (fullNameCol >= 0) {
        proxyModel->setFilterKeyColumn(fullNameCol);
        ui->searchLineEdit->setEnabled(true);
    } else {
        proxyModel->setFilterKeyColumn(-1);
        ui->searchLineEdit->setEnabled(false);
    }

    reloadview();
    ui->tableView->hideColumn(0);
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
    connect(model, &QSqlTableModel::dataChanged,
            this, &MainWindow::onModelDataChanged);
    proxyModel->setSourceModel(model);
    int fullNameCol = model->fieldIndex("full_name");
        if (fullNameCol >= 0) {
            proxyModel->setFilterKeyColumn(fullNameCol);
            ui->searchLineEdit->setEnabled(true);
        } else {
            proxyModel->setFilterKeyColumn(-1); // нет фильтруемой колонки
            ui->searchLineEdit->setEnabled(false);
        }
    reloadview();
}

void MainWindow::on_pushButton_2_clicked()
{
    model = new QSqlTableModel(this, QSqlDatabase::database());
    model->setTable("users");
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    connect(model, &QSqlTableModel::dataChanged,
            this, &MainWindow::onModelDataChanged);

    proxyModel->setSourceModel(model);

    int fullNameCol = model->fieldIndex("full_name");
    if (fullNameCol >= 0) {
        proxyModel->setFilterKeyColumn(fullNameCol);
        ui->searchLineEdit->setEnabled(true);
    } else {
        proxyModel->setFilterKeyColumn(-1);
        ui->searchLineEdit->setEnabled(false);
    }

    reloadview();
    ui->tableView->hideColumn(0);
}

void MainWindow::on_logoutButton_clicked()
{
    logoutRequested = true;
    this->close();
}

void MainWindow::ensureTrailingEmptyRow()
{
    if (!model) return;
    int rows = model->rowCount();
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
    QStringList required = {"login", "password", "full_name", "role"};
    for (const QString &fieldName : required) {
        int col = model->fieldIndex(fieldName);
        if (col < 0) continue;
        QVariant v = model->data(model->index(row, col));
        if (!v.isValid() || v.toString().trimmed().isEmpty()) {
            return false;
        }
    }
    return true;
}

void MainWindow::reloadview(){
    ui->tableView->setModel(proxyModel);
    ui->tableView->resizeColumnsToContents();
    ensureTrailingEmptyRow();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)
    int last = model->rowCount() - 1;
    if (last >= 0 && isRowFilled(last)) {
        commitLastRow();
    }
}

void MainWindow::commitLastRow()
{
    if (!model) return;

    int lastRow = model->rowCount() - 1;
    if (lastRow < 0) return;
    if (!isRowFilled(lastRow)) {
        return;
    }
    if (!model->submitAll()) {
        QString err = model->lastError().text();
        model->revertAll();
        return;
    }
    ensureTrailingEmptyRow();
    ui->tableView->resizeColumnsToContents();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->tableView && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QModelIndex cur = ui->tableView->currentIndex();
            if (!cur.isValid()) return false;
            int curRow = cur.row();
            if (curRow == model->rowCount() - 1) {
                ui->tableView->closePersistentEditor(cur);
                commitLastRow();
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::on_searchLineEdit_textChanged(const QString &text)
{
    if (!proxyModel) return;
    if (text.trimmed().isEmpty()) {
        proxyModel->setFilterRegularExpression(QRegularExpression());
        return;
    }
    QString escaped = QRegularExpression::escape(text);
    QRegularExpression re(escaped, QRegularExpression::CaseInsensitiveOption);
    proxyModel->setFilterRegularExpression(re);
}
