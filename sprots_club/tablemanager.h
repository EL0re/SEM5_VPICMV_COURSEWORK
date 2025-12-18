#ifndef TABLEMANAGER_H
#define TABLEMANAGER_H

#include <QObject>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QTableView>

class TableManager : public QObject
{
    Q_OBJECT
public:
    explicit TableManager(QObject *parent = nullptr);

        void setupTable(const QString &tableName, QTableView *view);

        void applyMultiFilter(const QMap<QString, QString> &filters);

        QSqlTableModel *getModel() const { return model; }
        QSortFilterProxyModel *getProxy() const { return proxyModel; }

    private:
        QSqlTableModel *model = nullptr;
        QSortFilterProxyModel *proxyModel = nullptr;
};

#endif // TABLEMANAGER_H
