#ifndef TABLEMANAGER_H
#define TABLEMANAGER_H

#include <QStyledItemDelegate>
#include <QPushButton>
#include <QApplication>
#include <QObject>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QItemDelegate>
#include <QComboBox>
#include <QSqlQuery>

class TableManager : public QObject
{
    Q_OBJECT
public:
    explicit TableManager(QObject *parent = nullptr);

    void setupTable(const QString &tableName, QTableView *view);

    void applyMultiFilter(const QMap<int, QString> &columnFilters);

    QSqlRelationalTableModel *getModel() const { return model; }
    QSortFilterProxyModel *getProxyModel() const { return proxyModel; }

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

class RelationComboBoxDelegate : public QItemDelegate {
    Q_OBJECT
public:
    RelationComboBoxDelegate(const QString &tableName, const QString &displayCol, const QString &filter = "", QObject *parent = nullptr)
        : QItemDelegate(parent), m_table(tableName), m_displayCol(displayCol), m_filter(filter) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override {
        QComboBox *editor = new QComboBox(parent);

        // Загружаем и ФИО, и ID в ComboBox
        // UserRole будет хранить ID, а DisplayRole — ФИО
        QString sql = QString("SELECT id, %1 FROM %2").arg(m_displayCol, m_table);
        if (!m_filter.isEmpty()) sql += " WHERE " + m_filter;

        QSqlQuery query(sql);
        while (query.next()) {
            editor->addItem(query.value(1).toString(), query.value(0)); // Текст и скрытый ID
        }
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QString value = index.data(Qt::DisplayRole).toString();
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentText(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int currentIdx = comboBox->currentIndex();

        if (currentIdx >= 0) {
            // Берем скрытый ID из ComboBox
            QVariant id = comboBox->itemData(currentIdx);
            // Записываем ID (число) в модель. Теперь модель не очистит поле!
            model->setData(index, id, Qt::EditRole);
        }
    }

private:
    QString m_table;
    QString m_displayCol;
    QString m_filter;
};

#endif // TABLEMANAGER_H
