#ifndef TABLEMANAGER_H
#define TABLEMANAGER_H

#include <QStyledItemDelegate>
#include <QPushButton>
#include <QApplication>
#include <QObject>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>
#include <QMap>
#include <QTableView>
#include <QItemDelegate>
#include <QComboBox>
#include <QSqlQuery>
#include <QTimeEdit>



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

class MultiFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    QMap<int, QString> columnFilters;

    // Добавляем этот публичный метод
    void setColumnFilters(const QMap<int, QString> &filters) {
        columnFilters = filters;
        invalidateFilter(); // Здесь вызов разрешен, так как это внутри класса
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        if (columnFilters.isEmpty()) return true;

        QMapIterator<int, QString> it(columnFilters);
        while (it.hasNext()) {
            it.next();
            int column = it.key();
            QString filterValue = it.value().toLower().trimmed();

            if (filterValue.isEmpty()) continue;

            QModelIndex index = sourceModel()->index(source_row, column, source_parent);
            QString rowValue = sourceModel()->data(index).toString().toLower();

            if (!rowValue.contains(filterValue)) {
                return false;
            }
        }
        return true;
    }
};

class FixedListDelegate : public QItemDelegate {
    Q_OBJECT
public:
    FixedListDelegate(const QStringList &items, QObject *parent = nullptr)
        : QItemDelegate(parent), m_items(items) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override {
        QComboBox *editor = new QComboBox(parent);
        editor->addItems(m_items);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QString value = index.data(Qt::EditRole).toString();
        QComboBox *cb = static_cast<QComboBox*>(editor);
        cb->setCurrentText(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QComboBox *cb = static_cast<QComboBox*>(editor);
        model->setData(index, cb->currentText(), Qt::EditRole);
    }

private:
    QStringList m_items;
};

// 2. Делегат для выбора времени (TIME)
class TimeEditDelegate : public QItemDelegate {
    Q_OBJECT
public:
    TimeEditDelegate(QObject *parent = nullptr) : QItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override {
        QTimeEdit *editor = new QTimeEdit(parent);
        editor->setDisplayFormat("HH:mm"); // Формат без секунд для удобства
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QTime value = index.data(Qt::EditRole).toTime();
        QTimeEdit *te = static_cast<QTimeEdit*>(editor);
        te->setTime(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QTimeEdit *te = static_cast<QTimeEdit*>(editor);
        model->setData(index, te->time().toString("HH:mm"), Qt::EditRole);
    }
};

#endif // TABLEMANAGER_H
