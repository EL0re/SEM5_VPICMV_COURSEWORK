#include "tablemanager.h"
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


TableManager::TableManager(QObject *parent) : QObject(parent)
{

}

void TableManager::setupTable(const QString &tableName, QTableView *view) {
    if (model) model->deleteLater();
    if (proxyModel) proxyModel->deleteLater();

    model = new QSqlRelationalTableModel(this, QSqlDatabase::database());
    model->setTable(tableName);

    // Настройка связей (Relations)
    if (tableName == "groups") {
        // Столбец 3 (trainer_id) -> таблица users (id), показ full_name
        model->setRelation(3, QSqlRelation("users", "id", "full_name"));
    }
    else if (tableName == "schedule") {
        // Столбец 1 (group_id) -> таблица groups (id), показ name
        model->setRelation(1, QSqlRelation("groups", "id", "name"));
    }
    else if (tableName == "attendance") {
        model->setRelation(1, QSqlRelation("users", "id", "full_name"));
        model->setRelation(2, QSqlRelation("groups", "id", "name"));
    }

    // Устанавливаем стратегию ManualSubmit, чтобы onModelDataChanged мог валидировать данные
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Загружаем данные
    if (!model->select()) {
        qDebug() << "Ошибка загрузки таблицы:" << model->lastError().text();
    }

    // Добавляем виртуальный столбец для кнопки, если это Группы
    if (tableName == "groups") {
        model->insertColumn(4);
    }

    // Настройка нашей кастомной мульти-фильтр прокси-модели
    proxyModel = new MultiFilterProxyModel();
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    view->setModel(proxyModel);

    view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}


void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QString status = index.data().toString();
    QStyleOptionButton checkBoxOption;
    checkBoxOption.rect = option.rect;
    checkBoxOption.state = QStyle::State_Enabled | (status == "present" ? QStyle::State_On : QStyle::State_Off);
    checkBoxOption.rect.setLeft(option.rect.left() + option.rect.width()/2 - 10); // Центрируем

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
}

bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QString currentStatus = index.data().toString();
        model->setData(index, (currentStatus == "present" ? "absent" : "present"));
        return true;
    }
    return false;
}

void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionButton button;
    button.rect = option.rect.adjusted(2, 2, -2, -2);
    button.text = "Ред.";
    button.state = QStyle::State_Enabled;
    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
}

bool ButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        emit buttonClicked(index.row());
        return true;
    }
    return false;
}

void TableManager::applyMultiFilter(const QMap<int, QString> &columnFilters) {
    // Приводим к нашему типу MultiFilterProxyModel
    auto multiProxy = qobject_cast<MultiFilterProxyModel*>(proxyModel);

    if (multiProxy) {
        // Вызываем наш публичный метод, который мы только что создали
        multiProxy->setColumnFilters(columnFilters);
    } else {
        // Если вдруг proxyModel еще старого типа, просто логируем
        qDebug() << "Ошибка: proxyModel не является MultiFilterProxyModel";
    }
}
