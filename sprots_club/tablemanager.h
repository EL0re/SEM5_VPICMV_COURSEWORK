#ifndef TABLEMANAGER_H
#define TABLEMANAGER_H

#include <QStyledItemDelegate>
#include <QPushButton>
#include <QApplication>
#include <QObject>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>
#include <QTableView>

class TableManager : public QObject
{
    Q_OBJECT
public:
    explicit TableManager(QObject *parent = nullptr);

    void setupTable(const QString &tableName, QTableView *view);

    void applyMultiFilter(const QMap<QString, QString> &filters);

    QSqlRelationalTableModel *getModel() const { return model; }

private:
    QSqlRelationalTableModel *model = nullptr;
    QSortFilterProxyModel *proxyModel = nullptr;
};

class ButtonDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ButtonDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
signals:
    void buttonClicked(int row);
};

// Делегат для чекбокса (present/absent) в таблице Посещаемость
class CheckBoxDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit CheckBoxDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif // TABLEMANAGER_H
