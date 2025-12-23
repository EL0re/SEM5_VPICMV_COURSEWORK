#include "tablemanager.h"
#include "studentsortproxymodel.h"
#include "mainwindow.h"
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QSqlRelation>
#include <QSqlRelationalDelegate>
#include <QDebug>
#include <QSqlError>
#include <QHeaderView>
#include <QSqlRecord>
#include <QSqlQuery>

/* заглушка для методов */
TableManager::TableManager(QObject *parent) : QObject(parent)
{
}

/* Функция setupTable отвечает за комплексную инициализацию отображения таблицы БД в QTableView.
   очистка памяти моделей, создание новой модели,
   связи между таблицами для имен вместо ID,
   загрузка данных из бд, создание виртуальных колонок для кнопок,
   модель для фильтрации, отрисовка кнопок(вызывает т к у них свои приколы), чекбоксов и выпадающих списков. */
void TableManager::setupTable(const QString &tableName, QTableView *view, UserRole role)
{
    currentRole = role;

    switch(currentRole) {
    case Admin: {
        // Админ - полный доступ (ваш текущий код)
        if (model)
        {
            model->deleteLater();
            model = nullptr;
        }
        if (proxyModel)
        {
            proxyModel->deleteLater();
            proxyModel = nullptr;
        }

        model = new QSqlRelationalTableModel(this, QSqlDatabase::database());

        MainWindow* mw = qobject_cast<MainWindow*>(this->parent());
        if (mw)
        {
            connect(model, &QSqlRelationalTableModel::dataChanged, mw, &MainWindow::onModelDataChanged);
        }
        model->setTable(tableName);

        if (tableName == "groups")
        {
            model->setRelation(3, QSqlRelation("users", "id", "full_name"));
        }
        else if (tableName == "schedule")
        {
            model->setRelation(1, QSqlRelation("groups", "id", "name"));
        }
        else if (tableName == "attendance")
        {
            model->setRelation(1, QSqlRelation("users", "id", "full_name"));
            model->setRelation(2, QSqlRelation("groups", "id", "name"));
        }

        model->setEditStrategy(QSqlTableModel::OnManualSubmit);

        if (!model->select())
        {
            qDebug() << "SQL Error:" << model->lastError().text();
        }

        int btnColumnIdx = -1;
        if (tableName == "groups")
        {
            btnColumnIdx = model->columnCount();
            model->insertColumn(btnColumnIdx);
            model->setHeaderData(btnColumnIdx, Qt::Horizontal, "Состав");
        }

        proxyModel = new MultiFilterProxyModel(this);
        proxyModel->setSourceModel(model);
        proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        view->setModel(proxyModel);

        for (int i = 0; i < view->model()->columnCount(); ++i)
        {
            view->setItemDelegateForColumn(i, new QStyledItemDelegate(view));
        }

        if (tableName == "groups" && btnColumnIdx != -1)
        {
            view->setItemDelegateForColumn(3, new QSqlRelationalDelegate(view));

            ButtonDelegate *btnDelegate = new ButtonDelegate(view);
            view->setItemDelegateForColumn(btnColumnIdx, btnDelegate);

            connect(btnDelegate, &ButtonDelegate::buttonClicked, this, [this, view, btnColumnIdx](const QModelIndex &proxyIndex)
            {
                if (!proxyIndex.isValid() || proxyIndex.column() != btnColumnIdx)
                {
                    return;
                }

                QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
                int row = sourceIndex.row();

                int idCol = model->record().indexOf("id");
                if (idCol == -1)
                {
                    idCol = 0;
                }

                int groupId = model->data(model->index(row, idCol)).toInt();
                QString groupName = model->data(model->index(row, 1)).toString();

                if (groupId <= 0)
                {
                    return;
                }

                EditGroupStudentsDialog dialog(groupId, groupName, view->window());
                if (dialog.exec() == QDialog::Accepted)
                {
                    QList<int> selectedIds = dialog.getSelectedStudentIds();
                    QSqlDatabase db = QSqlDatabase::database();

                    if (db.transaction())
                    {
                        QSqlQuery q;
                        q.prepare("DELETE FROM group_students WHERE group_id = ?");
                        q.addBindValue(groupId);
                        q.exec();

                        q.prepare("INSERT INTO group_students (group_id, student_id) VALUES (?, ?)");
                        for (int sId : selectedIds)
                        {
                            q.addBindValue(groupId);
                            q.addBindValue(sId);
                            q.exec();
                        }
                        if (db.commit())
                        {
                            qDebug() << "Success";
                        }
                        else
                        {
                            db.rollback();
                        }
                    }
                }
            }, Qt::UniqueConnection);
        }
        else if (tableName == "attendance")
        {
            view->setItemDelegateForColumn(4, new CheckBoxDelegate(view));
        }

        view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        view->viewport()->update();
        break;
    }

    case Trainer: {
        if (model)
        {
            model->deleteLater();
            model = nullptr;
        }
        if (proxyModel)
        {
            proxyModel->deleteLater();
            proxyModel = nullptr;
        }

        model = new QSqlRelationalTableModel(this, QSqlDatabase::database());

        MainWindow* mw = qobject_cast<MainWindow*>(this->parent());
        if (mw)
        {
            connect(model, &QSqlRelationalTableModel::dataChanged, mw, &MainWindow::onModelDataChanged);
        }
        model->setTable(tableName);

        if (tableName == "groups")
        {
            model->setRelation(3, QSqlRelation("users", "id", "full_name"));
        }
        else if (tableName == "schedule")
        {
            model->setRelation(1, QSqlRelation("groups", "id", "name"));
        }
        else if (tableName == "attendance")
        {
            model->setRelation(1, QSqlRelation("users", "id", "full_name"));
            model->setRelation(2, QSqlRelation("groups", "id", "name"));
        }

        model->setEditStrategy(QSqlTableModel::OnManualSubmit);

        if (!model->select())
        {
            qDebug() << "SQL Error:" << model->lastError().text();
        }

        int btnColumnIdx = -1;
        if (tableName == "groups")
        {
            btnColumnIdx = model->columnCount();
            model->insertColumn(btnColumnIdx);
            model->setHeaderData(btnColumnIdx, Qt::Horizontal, "Состав");
        }

        proxyModel = new MultiFilterProxyModel(this);
        proxyModel->setSourceModel(model);
        proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        view->setModel(proxyModel);

        // Для тренера права зависят от таблицы
        if (tableName == "attendance") {
            // Для посещаемости - полный доступ
            for (int i = 0; i < view->model()->columnCount(); ++i)
            {
                view->setItemDelegateForColumn(i, new QStyledItemDelegate(view));
            }

            // Разрешаем редактирование статуса
            view->setItemDelegateForColumn(4, new CheckBoxDelegate(view));
            view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        }
        else {
            // Для groups и schedule - только чтение
            for (int i = 0; i < view->model()->columnCount(); ++i)
            {
                view->setItemDelegateForColumn(i, new QStyledItemDelegate(view));
            }

            // Скрываем кнопку "Состав" в таблице групп
            if (tableName == "groups" && btnColumnIdx != -1)
            {
                view->setColumnHidden(btnColumnIdx, true);
            }

            // Запрещаем редактирование
            view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        }

        view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        view->viewport()->update();
        break;
    }


    }
}

/* крч для чекбокса фигня, чтобы текстовое поле не мелькало при нажатии */
QWidget *CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return nullptr;
}

/* чекбокс сам по себе(визуализация) т к он преобразовывает в текстовые поля present/absent - второ по умолчанию */
void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString status = index.data().toString();
    QStyleOptionButton checkBoxOption;
    checkBoxOption.rect = option.rect;
    checkBoxOption.state = QStyle::State_Enabled | (status == "present" ? QStyle::State_On : QStyle::State_Off);
    checkBoxOption.rect.setLeft(option.rect.left() + option.rect.width()/2 - 10);

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
}

/* клики по чекбоксу для изменений */
bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton)
        {
            QString currentStatus = index.data().toString();
            QString newStatus = (currentStatus == "present" ? "absent" : "present");

            model->setData(index, newStatus, Qt::EditRole);
            return true;
        }
    }
    return false;
}

/* рисовашки для кнопки */
void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int btnWidth = option.rect.width() / 2;
    int xOffset = (option.rect.width() - btnWidth) / 2;
    QRect btnRect = option.rect.adjusted(xOffset, 4, -xOffset, -4);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setBrush(QColor("#2196F3"));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(btnRect, 8, 8);

    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(btnRect, Qt::AlignCenter, "Показать");


    painter->restore();
}

/* нажатие на кнопку(четкое попадание т к та меньше) */
bool ButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        int btnWidth = option.rect.width() / 2;
        int xOffset = (option.rect.width() - btnWidth) / 2;
        QRect btnRect = option.rect.adjusted(xOffset, 4, -xOffset, -4);

        if (btnRect.contains(mouseEvent->pos()))
        {
            emit buttonClicked(index);
            return true;
        }
    }
    return false;
}

/* передача набора фильтров (QAbstractProxyModel к MultiFilterProxyModel). Если приведение успешно,
   вызывается метод установки фильтров для нескольких колонок одновременно (долго мучался, ибо стандартный класс не хотел
работать с неск.полями*/
void TableManager::applyMultiFilter(const QMap<int, QString> &columnFilters)
{
    auto multiProxy = qobject_cast<MultiFilterProxyModel*>(proxyModel);

    if (multiProxy)
    {
        multiProxy->setColumnFilters(columnFilters);
    }
    else
    {
        qDebug() << "Error: proxyModel is not MultiFilterProxyModel";
    }
}
