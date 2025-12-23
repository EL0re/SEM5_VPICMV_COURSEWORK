#include <QKeyEvent>
#include <QEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMap> //ключ-значения хранить
#include <QDebug>
#include <QSqlError> // ошибки бд
#include <QSqlQuery> //запросы бд
#include <QMessageBox>
#include "attendanceadddialog.h"
#include "studentsortproxymodel.h"
#include <QSqlRecord>  //записи - строки и т д
#include <QFileDialog>
#include <QTextStream> //для csv
#include <QStandardPaths>
#include <QDateTime>
#include <QFile>
#include <QDate>
#include <QTime>
#include "utils.h"

/*
создает менеджер таблиц, устанавливает имя пользователя в заголовок, настраивает
режимы выделения и сортировки для таблицы*/
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

/*
 * получениe айди пользователя по его полному имени.*/
int MainWindow::getUserIdByName(const QString &fullName)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE full_name = :name");
    query.bindValue(":name", fullName.trimmed());
    if (query.exec() && query.next())
    {
        return query.value(0).toInt();
    }
    return -1;
}

/*
 * видимость полей фильтрации. */
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

/* отображение таблиц(основной механизм в tablemanager. (на всякий случай сохр.при переходе) */
void MainWindow::switchToTable(const QString &tableName, const QString &title)
{
    if (tableManager && tableManager->getModel())
    {
        auto m = tableManager->getModel();
        if (m->isDirty())
        {
            if (!m->submitAll())
            {
                qDebug() << "Ошибка сохранения при переходе:" << m->lastError().text();
                m->revertAll();
            }
        }
    }

    if (ui->tableView->currentIndex().isValid())
    {
        ui->tableView->setCurrentIndex(QModelIndex());
    }

    currentTable = tableName;
    ui->label->setText(title);

    tableManager->setupTable(tableName, ui->tableView, TableManager::Admin);

    auto model = tableManager->getModel();
    if (!model)
    {
        return;
    }

    if (tableName == "users")
    {
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
    else if (tableName == "groups")
    {
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
    else if (tableName == "schedule")
    {
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
    else if (tableName == "attendance")
    {
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


void MainWindow::on_pushButton_2_clicked()
{
    switchToTable("users", "Пользователи");
    updateUI(true, false, false, false, "ФИО", "", "", "");
}


void MainWindow::on_pushButton_clicked()
{
    switchToTable("groups", "Группы");
    updateUI(true, false, true, true, "Название группы", "", "Тренер", "Направление");
}


void MainWindow::on_pushButton_3_clicked()
{
    switchToTable("schedule", "Расписание");
    updateUI(true, true, false, false, "Группа", "День недели", "", "");
}


void MainWindow::on_pushButton_4_clicked()
{
    switchToTable("attendance", "Посещаемость");
    updateUI(true, true, true, false, "Ученик", "Дата", "Группа", "");
}

/* берет данные из модели(по фильтрам) для экспорта */
void MainWindow::on_exportButton_clicked()
{
    auto proxy = tableManager->getProxyModel();
    if (!proxy || proxy->rowCount() == 0)
    {
        QMessageBox::warning(this, "Экспорт", "Нет данных для экспорта.");
        return;
    }

    QString dateStamp = QDate::currentDate().toString("yyyy-MM-dd");
    QString defaultName = QString("Export_%1.csv").arg(dateStamp);

    QString filePath = QFileDialog::getSaveFileName(this,
        "Сохранить отчет",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + defaultName,
        "CSV файлы (*.csv);;Все файлы (*.*)");

    if (filePath.isEmpty())
    {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл.");
        return;
    }

    QTextStream out(&file);
    out.setGenerateByteOrderMark(true);
    out.setCodec("UTF-8");

    QString sep = ";";

    QStringList headers;
    for (int col = 0; col < proxy->columnCount(); ++col)
    {
        if (!ui->tableView->isColumnHidden(col))
        {
            headers << proxy->headerData(col, Qt::Horizontal).toString();
        }
    }
    out << headers.join(sep) << "\n";

    for (int row = 0; row < proxy->rowCount(); ++row)
    {
        bool hasData = false;
        if (!proxy->data(proxy->index(row, 1)).toString().isEmpty() || !proxy->data(proxy->index(row, 3)).toString().isEmpty())
        {
            hasData = true;
        }
        if (!hasData)
        {
            continue;
        }

        QStringList rowData;
        for (int col = 0; col < proxy->columnCount(); ++col)
        {
            if (ui->tableView->isColumnHidden(col))
            {
                continue;
            }

            QModelIndex idx = proxy->index(row, col);
            QString val;

            if (currentTable == "attendance" && col == 4)
            {
                QString status = proxy->data(idx, Qt::EditRole).toString().toLower();

                if (status == "present")
                {
                    val = "Присутствовал";
                }
                else
                {
                    val = "Отсутствовал";
                }
            }
            else if (currentTable == "attendance" && col == 3)
            {
                val = "'" + proxy->data(idx, Qt::DisplayRole).toString();
            }
            else
            {
                val = proxy->data(idx, Qt::DisplayRole).toString();
            }

            if (val.contains(sep) || val.contains("\"") || val.contains("\n"))
            {
                val = "\"" + val.replace("\"", "\"\"") + "\"";
            }
            rowData << val;
        }
        out << rowData.join(sep) << "\n";
    }

    file.close();
    QMessageBox::information(this, "Завершено", "Данные успешно сохранены в файл:\n" + filePath);
}

/* валидации формата времени для импорта*/
bool MainWindow::validateTime(const QString &time)
{
    return QTime::fromString(time, "HH:mm").isValid();
}

/* валидации формата даты для импорта*/
bool MainWindow::validateDate(const QString &date)
{
    return QDate::fromString(date, "yyyy-MM-dd").isValid();
}

/* импорт таблиц(пока не совсем верно работает, т к во вкладках различия и посещаемость багуется)
 пытаюсь максимально настроить проверки*/
void MainWindow::on_importButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Открыть CSV для импорта", "", "CSV файлы (*.csv)");
    if (filePath.isEmpty())
    {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл.");
        return;
    }

    QTextStream in(&file);
    in.setGenerateByteOrderMark(true);
    in.setCodec("UTF-8");

    QStringList lines;
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty())
        {
            lines.append(line);
        }
    }
    file.close();

    if (lines.size() < 2)
    {
        return;
    }
    lines.takeFirst();

    auto normalizeTime = [](QString t) -> QString
    {
        QTime time = QTime::fromString(t.trimmed(), "H:mm");
        if (!time.isValid())
        {
            time = QTime::fromString(t.trimmed(), "HH:mm");
        }
        return time.isValid() ? time.toString("HH:mm") : t.trimmed();
    };

    QString sep = ";";
    QList<QStringList> rowsToProcess;

    for (const QString &line : lines)
    {
        QStringList cols = line.split(sep);
        for (QString &s : cols)
        {
            s = s.trimmed();
            if (s.startsWith("\"") && s.endsWith("\""))
            {
                s = s.mid(1, s.size()-2).replace("\"\"", "\"");
            }
        }
        rowsToProcess.append(cols);
    }

    QSqlDatabase::database().transaction();
    int added = 0, skipped = 0;

    for (const QStringList &cols : rowsToProcess)
    {
        QSqlQuery ins;
        if (currentTable == "schedule")
        {
            if (cols.size() < 4)
            {
                continue;
            }

            QSqlQuery q;
            q.prepare("SELECT id FROM groups WHERE name = ?");
            q.addBindValue(cols[0]);
            q.exec();
            q.next();
            int gId = q.value(0).toInt();

            QString day   = cols[1];
            QString start = normalizeTime(cols[2]);
            QString end   = normalizeTime(cols[3]);
            QString hall  = cols.value(4);

            QSqlQuery ck;
            ck.prepare("SELECT id FROM schedule WHERE group_id=? AND day_of_week=? AND start_time=? AND hall=?");
            ck.addBindValue(gId);
            ck.addBindValue(day);
            ck.addBindValue(start);
            ck.addBindValue(hall);
            ck.exec();

            if (ck.next())
            {
                skipped++;
                continue;
            }

            ins.prepare("INSERT INTO schedule (group_id, day_of_week, start_time, end_time, hall) VALUES (?, ?, ?, ?, ?)");
            ins.addBindValue(gId);
            ins.addBindValue(day);
            ins.addBindValue(start);
            ins.addBindValue(end);
            ins.addBindValue(hall);
        }
        else if (currentTable == "attendance")
        {
            if (cols.size() < 4)
            {
                continue;
            }

            QSqlQuery q;
            q.prepare("SELECT id FROM users WHERE full_name = ? AND role = 'student'");
            q.addBindValue(cols[0]);
            q.exec();
            q.next();
            int uId = q.value(0).toInt();

            q.prepare("SELECT id FROM groups WHERE name = ?");
            q.addBindValue(cols[1]);
            q.exec();
            q.next();
            int gId = q.value(0).toInt();

            QString date = cols[2];
            if (date.startsWith("'"))
            {
                date.remove(0, 1);
            }

            QSqlQuery ck;
            ck.prepare("SELECT id FROM attendance WHERE student_id=? AND group_id=? AND lesson_date=?");
            ck.addBindValue(uId);
            ck.addBindValue(gId);
            ck.addBindValue(date);
            ck.exec();
            if (ck.next())
            {
                skipped++;
                continue;
            }

            QString st = cols[3].toLower();
            QString dbSt = (st=="present" || st=="1" || st=="+" || st=="присутствовал") ? "present" : "absent";

            ins.prepare("INSERT INTO attendance (student_id, group_id, lesson_date, status) VALUES (?, ?, ?, ?)");
            ins.addBindValue(uId);
            ins.addBindValue(gId);
            ins.addBindValue(date);
            ins.addBindValue(dbSt);
        }
        if (ins.exec())
        {
            added++;
        }
    }

    QSqlDatabase::database().commit();
    tableManager->getModel()->select();
    QMessageBox::information(this, "Импорт", QString("Добавлено: %1, Дубликатов: %2").arg(added).arg(skipped));
}

/* т к иногда кнопка добавления вызывает модальное окно, прокрутка, фильтрация */
void MainWindow::on_addButton_clicked()
{
    if (currentTable == "attendance")
    {
        AttendanceAddDialog dialog(this);

        if (dialog.exec() == QDialog::Accepted)
        {
            if (tableManager && tableManager->getModel())
            {
                tableManager->getModel()->select();
            }
            reloadview();
            qDebug() << "Записи посещаемости успешно добавлены и отображены.";
        }
    }
    else
    {
        auto model = tableManager->getModel();
        if (!model)
        {
            return;
        }

        ui->tableView->setSortingEnabled(false);

        if (model->insertRow(0))
        {
            QModelIndex proxyIndex = tableManager->getProxyModel()->mapFromSource(model->index(0, 1));
            ui->tableView->scrollToTop();
            ui->tableView->setCurrentIndex(proxyIndex);
            ui->tableView->edit(proxyIndex);
        }

        ui->tableView->setSortingEnabled(true);
    }
}

/* удаление с проверками */
void MainWindow::on_delButton_clicked()
{
    QItemSelectionModel *select = ui->tableView->selectionModel();
    QModelIndexList selectedRows = select->selectedRows();

    if (selectedRows.isEmpty())
    {
        QModelIndex current = ui->tableView->currentIndex();
        if (current.isValid())
        {
            selectedRows.append(current);
        }
        else
        {
            return;
        }
    }

    auto model = tableManager->getModel();
    auto proxy = tableManager->getProxyModel();

    if (currentTable == "users")
    {
        QString currentUserFio = ui->FIO_Label->text().trimmed();
        for (const QModelIndex &proxyIdx : selectedRows)
        {
            QModelIndex sourceIdx = proxy->mapToSource(proxyIdx);
            int r = sourceIdx.row();
            QString targetFio = model->data(model->index(r, 3)).toString().trimmed();
            QString targetRole = model->data(model->index(r, 4)).toString().trimmed();

            if (targetFio == currentUserFio && targetRole == "admin")
            {
                QMessageBox::warning(this, "Защита", "Вы не можете удалить свою учетную запись администратора!");
                return;
            }
        }
    }

    auto res = QMessageBox::question(this, "Подтверждение",
                                     "Удалить выбранные записи?\nЭто вызовет каскадное удаление всех связанных данных!",
                                     QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes)
    {
        return;
    }

    QList<int> idsToDelete;
    int idColumn = model->record().indexOf("id");
    if (idColumn == -1)
    {
        idColumn = 0;
    }

    for (const QModelIndex &proxyIdx : selectedRows)
    {
        QModelIndex sourceIdx = proxy->mapToSource(proxyIdx);
        int row = sourceIdx.row();
        if (row >= 0 && row < model->rowCount())
        {
            int id = model->data(model->index(row, idColumn)).toInt();
            if (id > 0)
            {
                idsToDelete.append(id);
            }
        }
    }

    if (idsToDelete.isEmpty())
    {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();

    QSqlQuery pragmaQuery(db);
    pragmaQuery.exec("PRAGMA foreign_keys = ON;");

    if (!db.transaction())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать транзакцию базы данных.");
        return;
    }

    bool success = true;
    QSqlQuery query(db);

    for (int id : idsToDelete)
    {
        query.prepare(QString("DELETE FROM %1 WHERE id = ?").arg(currentTable));
        query.addBindValue(id);
        if (!query.exec())
        {
            success = false;
            break;
        }
    }

    if (success && db.commit())
    {
        model->select();
        while (model->canFetchMore())
        {
            model->fetchMore();
        }
    }
    else
    {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось удалить записи: " + query.lastError().text());
    }
}

/* выход в логинокно. */
void MainWindow::on_logoutButton_clicked()
{
    logoutRequested = true;
    this->close();
}

/* надо будет удалить это потом, т к я решил для эстетики отказаться от быстрой пустой строки. */
void MainWindow::ensureTrailingEmptyRow()
{
    auto model = tableManager->getModel();
    if (model && (model->rowCount() == 0 || isRowFilled(model->rowCount()-1)))
    {
        model->insertRow(model->rowCount());
    }
}

/* проверка заполнености строки(т к у нас дофига нот нулл в бд */
bool MainWindow::isRowFilled(int row) const
{
    auto model = tableManager->getModel();
    QStringList fields;
    if (currentTable == "users")
    {
        fields << "login" << "password" << "full_name" << "role";
    }
    else if (currentTable == "groups")
    {
        fields << "name" << "direction";
    }
    else if (currentTable == "attendance")
    {
        fields << "lesson_date" << "status";
    }

    for (const QString &f : fields)
    {
        int idx = model->fieldIndex(f);
        if (idx == -1)
        {
            continue;
        }
        if (model->data(model->index(row, idx)).toString().trimmed().isEmpty())
        {
            return false;
        }
    }
    return true;
}

/* растягиевает и заполняет колонки под данные*/
void MainWindow::reloadview()
{
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

/*изменение данных в модели, автоматическое хеширование паролей,
   проверку на дубликаты имен пользователей, преобразование текста
   в идентификаторы для связанных таблиц (тренеры, группы) и валидация
   заполненных данных */
void MainWindow::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    auto model = qobject_cast<QSqlRelationalTableModel*>(tableManager->getModel());
    if (!model || !model->isDirty())
    {
        return;
    }

    int row = topLeft.row();
    int col = topLeft.column();

    if (currentTable == "users" && col == 2)
    {
        QString currentPass = model->index(row, 2).data().toString();
        if (!currentPass.isEmpty() && currentPass.length() != 64)
        {
            QString hashedPass = hashPassword(currentPass);
            model->blockSignals(true);
            model->setData(model->index(row, 2), hashedPass);
            model->blockSignals(false);
            qDebug() << "Пароль захеширован автоматически.";
        }
    }

    if (currentTable == "users" && col == 3)
    {
        QString name = model->index(row, 3).data().toString().trimmed();
        int currentId = model->index(row, 0).data().toInt();
        if (!name.isEmpty())
        {
            QSqlQuery check;
            check.prepare("SELECT id FROM users WHERE full_name = :n AND id != :id");
            check.bindValue(":n", name);
            check.bindValue(":id", currentId);
            if (check.exec() && check.next())
            {
                QMessageBox::warning(this, "Дубликат", "Пользователь с таким ФИО уже есть в базе.");
                model->revertAll();
                model->select();
                return;
            }
        }
    }

    model->blockSignals(true);

    if (currentTable == "groups" && col == 3)
    {
        QString val = model->index(row, 3).data(Qt::DisplayRole).toString();
        bool isNumeric;
        val.toInt(&isNumeric);
        if (!isNumeric)
        {
            QSqlQuery query;
            query.prepare("SELECT id FROM users WHERE full_name = :name AND role = 'trainer'");
            query.bindValue(":name", val);
            if (query.exec() && query.next())
            {
                model->setData(model->index(row, 3), query.value(0).toInt(), Qt::EditRole);
            }
        }
    }
    else if (currentTable == "schedule" && col == 1)
    {
        QString val = model->index(row, 1).data(Qt::DisplayRole).toString();
        bool isNumeric;
        val.toInt(&isNumeric);
        if (!isNumeric)
        {
            QSqlQuery query;
            query.prepare("SELECT id FROM groups WHERE name = :name");
            query.bindValue(":name", val);
            if (query.exec() && query.next())
            {
                model->setData(model->index(row, 1), query.value(0).toInt(), Qt::EditRole);
            }
        }
    }

    model->blockSignals(false);

    int lastRequiredCol = (currentTable == "users") ? 4 : 3;
    if (currentTable == "schedule")
    {
        lastRequiredCol = 5;
    }
    else if (currentTable == "attendance")
    {
        lastRequiredCol = 4;
    }

    bool allFilled = true;
    for (int i = 1; i <= lastRequiredCol; ++i)
    {
        if (model->index(row, i).data().toString().trimmed().isEmpty())
        {
            allFilled = false;
            break;
        }
    }

    if (allFilled)
    {
        if (currentTable == "groups")
        {
            int currentId = model->index(row, 0).data().toInt();
            QString groupName = model->index(row, 1).data().toString().trimmed();

            QSqlQuery checkName;
            checkName.prepare("SELECT id FROM groups WHERE name = :name AND id != :id");
            checkName.bindValue(":name", groupName);
            checkName.bindValue(":id", currentId);

            if (checkName.exec() && checkName.next())
            {
                QMessageBox::warning(this, "Дубликат", QString("Группа с названием '%1' уже существует!").arg(groupName));
                model->revertAll();
                return;
            }
        }

        if (currentTable == "schedule")
        {
            int currentId = model->index(row, 0).data().toInt();
            int groupId = model->index(row, 1).data(Qt::EditRole).toInt();
            QString day = model->index(row, 2).data().toString().trimmed();
            QString start = model->index(row, 3).data().toString().trimmed();
            QString end = model->index(row, 4).data().toString().trimmed();
            QString hall = model->index(row, 5).data().toString().trimmed();

            if (start >= end)
            {
                QMessageBox::warning(this, "Ошибка времени", "Время начала занятия должно быть раньше времени окончания!");
                model->revertAll();
                return;
            }

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

            if (qHall.exec() && qHall.next())
            {
                QMessageBox::critical(this, "Конфликт зала",
                    QString("Зал '%1' уже занят в это время группой: %2").arg(hall, qHall.value(0).toString()));
                model->revertAll();
                return;
            }

            QSqlQuery qGetT;
            qGetT.prepare("SELECT trainer_id FROM groups WHERE id = :gid");
            qGetT.bindValue(":gid", groupId);

            if (qGetT.exec() && qGetT.next())
            {
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

                if (qTr.exec() && qTr.next())
                {
                    QMessageBox::critical(this, "Конфликт тренера",
                        QString("Выбранный тренер в это время ведет занятие у группы: %1").arg(qTr.value(0).toString()));
                    model->revertAll();
                    return;
                }
            }
        }

        if (model->submitAll())
        {
            model->select();
            setupDelegatesForCurrentTable();
        }
        else
        {
            qDebug() << "Ошибка сохранения данных:" << model->lastError().text();
        }
    }
}

/* настройка делегатов текущей таблицы, убираем столбец айди
   и назначаем выпадающие списки или кнопки  */
void MainWindow::setupDelegatesForCurrentTable()
{
    ui->tableView->hideColumn(0);

    if (currentTable == "groups")
    {
        ui->tableView->setItemDelegateForColumn(3, new RelationComboBoxDelegate("users", "full_name", "role='trainer'", this));
        ui->tableView->setItemDelegateForColumn(4, new ButtonDelegate(this));
    }
    else if (currentTable == "schedule")
    {
        ui->tableView->setItemDelegateForColumn(1, new RelationComboBoxDelegate("groups", "name", "", this));
        QStringList days = {"Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
        ui->tableView->setItemDelegateForColumn(2, new FixedListDelegate(days, this));
        ui->tableView->setItemDelegateForColumn(3, new TimeEditDelegate(this));
        ui->tableView->setItemDelegateForColumn(4, new TimeEditDelegate(this));
    }
}

/* настройка специализированных делегатов для таблицы групп, выбор тренеров
   через комбобокс и кнопка для отображения состава группы. */
void MainWindow::setupGroupDelegates()
{
    RelationComboBoxDelegate *coachDel = new RelationComboBoxDelegate("users", "full_name", "role='trainer'", this);
    ui->tableView->setItemDelegateForColumn(3, coachDel);

    ButtonDelegate *btnDel = new ButtonDelegate(this);
    ui->tableView->setItemDelegateForColumn(4, btnDel);
}

/* Функция фиксации изменений в последней измененной строке(без этого никуда, т к игорь предложил фигню без спец.форм заполнения) */
void MainWindow::commitLastRow()
{
    auto model = tableManager->getModel();
    if (!model || model->rowCount() == 0)
    {
        return;
    }

    if (model->submitAll())
    {
        ensureTrailingEmptyRow();//
        ui->tableView->resizeColumnsToContents();
    }
}

/*горячие клавиши, а не пирожки( энтер и дел */
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->tableView && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            QModelIndex cur = ui->tableView->currentIndex();
            if (cur.isValid())
            {
                ui->tableView->closePersistentEditor(cur);
                commitLastRow();
                return true;
            }
        }
        if (keyEvent->key() == Qt::Key_Delete)
        {
            on_delButton_clicked();
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

/* Собирает данные
   со всех полей ввода, сопоставляет их с индексами колонок текущей таблицы
   и вызывает мультифильтр(как-то вроде даже работает сложно) */
void MainWindow::on_anySearchField_changed()
{
    QMap<int, QString> filters;

    if (currentTable == "users")
    {
        QString fio = ui->searchLineEdit1->text().trimmed();
        if (!fio.isEmpty())
        {
            filters.insert(3, fio);
        }
    }
    else if (currentTable == "groups")
    {
        QString groupName = ui->searchLineEdit1->text().trimmed();
        if (!groupName.isEmpty())
        {
            filters.insert(1, groupName);
        }

        QString direction = ui->filterLineEdit2->text().trimmed();
        if (!direction.isEmpty())
        {
            filters.insert(2, direction);
        }

        QString trainer = ui->filterLineEdit1->text().trimmed();
        if (!trainer.isEmpty())
        {
            filters.insert(3, trainer);
        }
    }
    else if (currentTable == "schedule")
    {
        QString group = ui->searchLineEdit1->text().trimmed();
        if (!group.isEmpty())
        {
            filters.insert(1, group);
        }

        QString day = ui->searchLineEdit2->text().trimmed();
        if (!day.isEmpty())
        {
            filters.insert(2, day);
        }
    }
    else if (currentTable == "attendance")
    {
        QString student = ui->searchLineEdit1->text().trimmed();
        if (!student.isEmpty())
        {
            filters.insert(1, student);
        }

        QString date = ui->searchLineEdit2->text().trimmed();
        if (!date.isEmpty())
        {
            filters.insert(3, date);
        }

        QString group = ui->filterLineEdit1->text().trimmed();
        if (!group.isEmpty())
        {
            filters.insert(2, group);
        }
    }

    if (tableManager)
    {
        tableManager->applyMultiFilter(filters);
    }
}
