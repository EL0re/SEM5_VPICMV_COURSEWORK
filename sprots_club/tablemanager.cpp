#include "tablemanager.h"
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QSqlRelation>
#include <QSqlRelationalDelegate>

TableManager::TableManager(QObject *parent) : QObject(parent)
{

}

void TableManager::setupTable(const QString &tableName, QTableView *view) {
    if (model) model->deleteLater();
    if (proxyModel) proxyModel->deleteLater();

    model = new QSqlRelationalTableModel(this, QSqlDatabase::database());
    model->setTable(tableName);

    // 1. Устанавливаем связи
    if (tableName == "schedule") {
        model->setRelation(1, QSqlRelation("groups", "id", "name"));
    }
    else if (tableName == "attendance") {
        model->setRelation(1, QSqlRelation("users", "id", "full_name"));
        model->setRelation(2, QSqlRelation("groups", "id", "name"));
    }
    else if (tableName == "groups") {
        model->setRelation(3, QSqlRelation("users", "id", "full_name"));
    }

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 2. Сначала выполняем SELECT, чтобы получить данные и структуру из БД
    model->select();

    // 3. ТОЛЬКО ПОСЛЕ select() добавляем виртуальную колонку
    if (tableName == "groups") {
        model->insertColumn(4); // Теперь она не исчезнет до следующего select()
    }

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    view->setModel(proxyModel);

    // Стандартный делегат для комбобоксов (выбор тренера/группы из списка)
    view->setItemDelegate(new QSqlRelationalDelegate(view));
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

void TableManager::applyMultiFilter(const QMap<QString, QString> &filters) {
    if (!model) return;

    QStringList filterConditions;
    QMapIterator<QString, QString> i(filters);

    while (i.hasNext()) {
        i.next();
        QString column = i.key();
        QString value = i.value().trimmed();

        if (!value.isEmpty()) {
            // Используем LIKE для частичного совпадения (поиск и фильтр)
            filterConditions << QString("%1 LIKE '%%2%'").arg(column).arg(value);
        }
    }

    model->setFilter(filterConditions.join(" AND "));
    model->select();
}
