    #include "trainerwindow.h"
    #include "ui_trainerwindow.h"
    #include <QKeyEvent>
    #include <QEvent>
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

        tableManager->setupTable(tableName, ui->tableView);

        auto model = tableManager->getModel();
        if (!model)
        {
            return;
        }


        if (tableName == "groups")
        {
            QString filter = QString("trainer_id = %1").arg(currentUserId);
            model->setFilter(filter);
            model->select();
            ui->tableView->setItemDelegateForColumn(3, new RelationComboBoxDelegate("users", "full_name", "role='trainer'", this));
            model->setHeaderData(1, Qt::Horizontal, "Название группы");
            model->setHeaderData(2, Qt::Horizontal, "Направление");
            model->setHeaderData(3, Qt::Horizontal, "Тренер");
            model->setHeaderData(4, Qt::Horizontal, "Состав");

            ui->labelFilters->show();
            ui->addButton->hide();
    //        ui->slashLabel->hide();
    //        ui->importButton->hide();
    //        ui->exportButton->hide();
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
    //        ui->slashLabel->show();
    //        ui->importButton->show();
    //        ui->exportButton->show();
        }
        else if (tableName == "attendance")
        {
            QString filter = QString("group_id IN (SELECT id FROM groups WHERE trainer_id = %1)").arg(currentUserId);
            model->setFilter(filter);
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
    //        ui->slashLabel->show();
    //        ui->importButton->show();
    //        ui->exportButton->show();
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
        updateUI(true, false, true, true, "Название группы", "", "Тренер", "Направление");
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
