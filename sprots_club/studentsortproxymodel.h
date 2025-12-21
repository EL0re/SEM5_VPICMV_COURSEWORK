#ifndef STUDENTSORTPROXYMODEL_H
#define STUDENTSORTPROXYMODEL_H


#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>


class StudentSortProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit StudentSortProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent)
    {
        setSortRole(Qt::CheckStateRole);
    }

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
    {
        int leftState = sourceModel()->data(sourceModel()->index(source_left.row(), 1), Qt::CheckStateRole).toInt();
        int rightState = sourceModel()->data(sourceModel()->index(source_right.row(), 1), Qt::CheckStateRole).toInt();

        if (leftState != rightState)
        {
            return leftState < rightState;
        }

        QString leftName = sourceModel()->data(sourceModel()->index(source_left.row(), 0), Qt::DisplayRole).toString();
        QString rightName = sourceModel()->data(sourceModel()->index(source_right.row(), 0), Qt::DisplayRole).toString();

        return QString::localeAwareCompare(leftName, rightName) < 0;
    }
};

class EditGroupStudentsDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditGroupStudentsDialog(int groupId, const QString &groupName, QWidget *parent = nullptr);

    QList<int> getSelectedStudentIds() const;

private slots:
    void onSearchChanged(const QString &text);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    int m_groupId;
    QStandardItemModel *m_model;
    StudentSortProxyModel *m_proxyModel;
    QLineEdit *m_searchEdit;
    QTableView *m_view;

    void loadStudents();
};

#endif // STUDENTSORTPROXYMODEL_H
