#include "studentschedulefilterproxy.h"
#include <QSqlQuery>
#include <QDebug>

StudentScheduleFilterProxy::StudentScheduleFilterProxy(int studentId, QObject *parent)
    : QSortFilterProxyModel(parent), m_studentId(studentId)
{
}

bool StudentScheduleFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QAbstractItemModel* source = sourceModel();
    if (!source) {
        return false;
    }

    if (!QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
        return false;
    }

    QModelIndex groupIdIndex = source->index(sourceRow, 1, sourceParent);
    int groupId = source->data(groupIdIndex).toInt();

    QSqlQuery studentQuery;
    studentQuery.prepare(
        "SELECT COUNT(*) FROM group_students WHERE group_id = ? AND student_id = ?"
    );
    studentQuery.addBindValue(groupId);
    studentQuery.addBindValue(m_studentId);

    if (studentQuery.exec() && studentQuery.next()) {
        bool accepted = studentQuery.value(0).toInt() > 0;
        return accepted;
    }

    return false;
}
