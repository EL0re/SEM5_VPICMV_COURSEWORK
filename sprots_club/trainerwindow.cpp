#include "trainerwindow.h"
#include "ui_trainerwindow.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include "attendanceadddialog.h"
#include "studentsortproxymodel.h"
#include <QSqlRecord>
#include "utils.h"

trainerwindow::trainerwindow(int userId, const QString &fullName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::trainerwindow),
    currentUserId(userId)
{
    ui->setupUi(this);
    ui->FIO_Label->setText(fullName);
    tableManager = new TableManager(this);
    connect(ui->searchLineEdit1, &QLineEdit::textChanged, this, &trainerwindow::on_anySearchField_changed);
    connect(ui->searchLineEdit2, &QLineEdit::textChanged, this, &trainerwindow::on_anySearchField_changed);
    connect(ui->filterLineEdit1, &QLineEdit::textChanged, this, &trainerwindow::on_anySearchField_changed);
    connect(ui->filterLineEdit2, &QLineEdit::textChanged, this, &trainerwindow::on_anySearchField_changed);
    on_pushButton_3_clicked();
}

trainerwindow::~trainerwindow()
{
    delete ui;
}

void trainerwindow::switchToTable(const QString &tableName, const QString &title)
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

    tableManager->setupTable(tableName, ui->tableView,TableManager::Trainer);

    auto model = tableManager->getModel();
    if (!model)
    {
        return;
    }

    if (tableName == "groups")
    {
        ui->addButton->hide();
        ui->filterLineEdit2->hide();
        ui->labelFilters->show();
        QString filter = QString("trainer_id = %1").arg(currentUserId);
        model->setFilter(filter);
        model->select();
        ui->tableView->setItemDelegateForColumn(3, new RelationComboBoxDelegate("users", "full_name", "role='trainer'", this));
        int colCount = model->columnCount();
        bool hasButtonColumn = false;
        for (int i = 0; i < colCount; ++i) {
            QString header = model->headerData(i, Qt::Horizontal).toString();
            if (header.contains("Действия", Qt::CaseInsensitive) ||
                header.contains("Состав", Qt::CaseInsensitive)) {
                hasButtonColumn = true;
                break;
            }
        }
        if (!hasButtonColumn) {
            model->insertColumn(colCount);
            colCount = model->columnCount();
        }
        model->setHeaderData(1, Qt::Horizontal, "Название группы");
        model->setHeaderData(2, Qt::Horizontal, "Направление");
        model->setHeaderData(3, Qt::Horizontal, "Тренер");
        model->setHeaderData(colCount-1, Qt::Horizontal, "Состав");
        int buttonColumnIndex = colCount - 1;
        ButtonDelegate *buttonDelegate = new ButtonDelegate(this);
        ui->tableView->setItemDelegateForColumn(buttonColumnIndex, buttonDelegate);
        ui->tableView->setColumnWidth(buttonColumnIndex, 100);
        connect(buttonDelegate, &ButtonDelegate::buttonClicked, this, [this, model](const QModelIndex &proxyIndex) {
            QModelIndex sourceIndex = tableManager->getProxyModel()->mapToSource(proxyIndex);
            int row = sourceIndex.row();
            int groupId = model->index(row, 0).data().toInt();
            QString groupName = model->index(row, 1).data().toString();
            qDebug() << "Просмотр состава группы:" << groupId << groupName;
            QSqlQuery query;
            query.prepare(
                "SELECT u.full_name FROM users u "
                "JOIN group_students gs ON u.id = gs.student_id "
                "WHERE gs.group_id = ? AND u.role = 'student' "
                "ORDER BY u.full_name"
            );
            query.addBindValue(groupId);
            QStringList students;
            if (query.exec()) {
                while (query.next()) {
                    students << query.value(0).toString();
                }
            }
            QString message;
            if (students.isEmpty()) {
                message = QString("В группе '%1' пока нет учеников.").arg(groupName);
            } else {
                message = QString("Список учеников в группе '%1':\n\n").arg(groupName);
                for (int i = 0; i < students.size(); ++i) {
                    message += QString("%1. %2\n").arg(i + 1).arg(students[i]);
                }
                message += QString("\nВсего: %1 ученик(ов)").arg(students.size());
            }
            QMessageBox::information(this, "Состав группы", message);
        });
    }
    else if (tableName == "schedule")
    {
        QString filter = QString("group_id IN (SELECT id FROM groups WHERE trainer_id = %1)").arg(currentUserId);
        model->setFilter(filter);
        model->select();
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
        ui->addButton->hide();
    }
    else if (tableName == "attendance")
    {
        QString filter = QString("group_id IN (SELECT id FROM groups WHERE trainer_id = %1)").arg(currentUserId);
        model->setFilter(filter);
        model->setSort(3, Qt::DescendingOrder);
        model->select();
        model->setHeaderData(1, Qt::Horizontal, "ФИО");
        model->setHeaderData(2, Qt::Horizontal, "Название группы");
        model->setHeaderData(3, Qt::Horizontal, "Дата занятия");
        model->setHeaderData(4, Qt::Horizontal, "Статус");
        ui->tableView->setItemDelegateForColumn(1, new RelationComboBoxDelegate("users", "full_name", "role='student'", this));
        ui->tableView->setItemDelegateForColumn(2, new RelationComboBoxDelegate("groups", "name", "", this));
        ui->tableView->setItemDelegateForColumn(4, new CheckBoxDelegate(this));
        ui->labelFilters->show();
        ui->addButton->show();
    }

    ui->tableView->hideColumn(0);
    reloadview();
}

void trainerwindow::reloadview()
{
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void trainerwindow::on_logoutButton_clicked()
{
    logoutRequested = true;
    this->close();
}

void trainerwindow::on_pushButton_3_clicked()
{
    switchToTable("schedule", "Расписание");
    updateUI(true, true, false, false, "Группа", "День недели", "", "");
}

void trainerwindow::on_pushButton_clicked()
{
    switchToTable("groups", "Группы");
    updateUI(true, false, true, false, "Название группы", "", "Направление", "");
}

void trainerwindow::on_pushButton_4_clicked()
{
    switchToTable("attendance", "Посещаемость");
    updateUI(true, true, true, false, "Ученик", "Дата", "Группа", "");
}

void trainerwindow::on_addButton_clicked()
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

void trainerwindow::updateUI(bool s1, bool s2, bool f1, bool f2, const QString &ps1, const QString &ps2, const QString &pf1, const QString &pf2)
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

void trainerwindow::on_anySearchField_changed()
{
    QMap<int, QString> filters;
    if (currentTable == "groups")
    {
        QString groupName = ui->searchLineEdit1->text().trimmed();
        if (!groupName.isEmpty())
        {
            filters.insert(1, groupName);
        }
        QString direction = ui->filterLineEdit1->text().trimmed();
        if (!direction.isEmpty())
        {
            filters.insert(2, direction);
        }
        QString trainer = ui->filterLineEdit2->text().trimmed();
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

void trainerwindow::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
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

void trainerwindow::setupDelegatesForCurrentTable()
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
