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

    // Настройка связей
    if (tableName == "groups") {
        // Связываем 3-й столбец (тренер) с таблицей users (поле id), отображаем full_name
        model->setRelation(3, QSqlRelation("users", "id", "full_name"));
        QSqlTableModel *relModel = model->relationModel(3);
        if (relModel) {
            relModel->setFilter("role = 'trainer'");
            relModel->select();
        }
    } else if (tableName == "attendance") {
        model->setRelation(1, QSqlRelation("users", "id", "full_name"));
        model->setRelation(2, QSqlRelation("groups", "id", "name"));
    }

    // Оставляем стратегию ManualSubmit для контроля заполнения
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    if (tableName == "groups") {
        model->insertColumn(4);
    }

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);

    view->setModel(proxyModel);

    // ВОТ ЭТА СТРОКА РЕШАЕТ ПРОБЛЕМУ ОЧИСТКИ:
    // Она создает ComboBox, и пользователь просто выбирает тренера из существующих
    view->setItemDelegate(new QSqlRelationalDelegate(view));

    // Чтобы список открывался удобнее
    view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
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
    if (!proxyModel) return;

    QMapIterator<int, QString> i(columnFilters);
    while (i.hasNext()) {
        i.next();
        if (!i.value().isEmpty()) {
            proxyModel->setFilterKeyColumn(i.key());
            proxyModel->setFilterFixedString(i.value());
            return;
        }
    }
    proxyModel->setFilterFixedString("");
}
