#ifndef STUDENTSCHEDULEFILTERPROXY_H
#define STUDENTSCHEDULEFILTERPROXY_H

#include <QSortFilterProxyModel>
#include <QSqlQuery>

class StudentScheduleFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit StudentScheduleFilterProxy(int studentId, QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    int m_studentId;
};

#endif // STUDENTSCHEDULEFILTERPROXY_H
