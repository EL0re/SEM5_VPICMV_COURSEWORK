#include "tablemanager.h"
#include <QRegularExpression>
#include <QSqlDatabase>

TableManager::TableManager(QObject *parent) : QObject(parent)
{

}

void TableManager::setupTable(const QString &tableName, QTableView *view)
{
    if (model) model->deleteLater();
    if (proxyModel) proxyModel->deleteLater();

    model = new QSqlTableModel(this, QSqlDatabase::database());
    model->setTable(tableName);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    view->setModel(proxyModel);
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
