#include "studentsortproxymodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>
#include <QDebug>

EditGroupStudentsDialog::EditGroupStudentsDialog(int groupId, const QString &groupName, QWidget *parent)
    : QDialog(parent), m_groupId(groupId)
{
    setWindowTitle("Состав группы");
    setModal(true);
    setMinimumSize(600, 500);

    // Общий фон диалога
    setStyleSheet("QDialog { background-color: #ffffff; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 1. Заголовок окна
    QLabel *titleLabel = new QLabel(QString("Состав группы: %1").arg(groupName), this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333; margin-bottom: 5px;");
    mainLayout->addWidget(titleLabel);

    // 2. Панель поиска
    QHBoxLayout *topPanel = new QHBoxLayout();
    QLabel *subTitle = new QLabel("Ученики", this);
    subTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #666;");

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Поиск по ФИО...");
    m_searchEdit->setFixedWidth(250);

    // ПРИМЕНЯЕМ ВАШ СТИЛЬ
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: #f0f0f0;"
        "    border: 1px solid #d1d1d1;"
        "    border-radius: 12px;"
        "    padding: 6px 12px;"
        "    color: #333333;"
        "}"
        "QLineEdit:focus {"
        "    background-color: #ffffff;"
        "    border: 1px solid #2196F3;"
        "}"
    );

    topPanel->addWidget(subTitle);
    topPanel->addStretch();
    topPanel->addWidget(new QLabel("Поиск:", this));
    topPanel->addWidget(m_searchEdit);
    mainLayout->addLayout(topPanel);

    // 3. Таблица
    m_view = new QTableView(this);
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels({"ФИО", ""});

    m_proxyModel = new StudentSortProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterKeyColumn(0);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setSortRole(Qt::CheckStateRole);

    m_view->setModel(m_proxyModel);
    m_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_view->setColumnWidth(1, 50);
    m_view->verticalHeader()->setVisible(false);
    m_view->setSelectionMode(QAbstractItemView::NoSelection);
    m_view->setStyleSheet(
        "QTableView { background-color: white; border: 1px solid #ddd; gridline-color: #f0f0f0; border-radius: 8px; }"
        "QHeaderView::section { background-color: #f8f8f8; padding: 6px; font-weight: bold; border: none; border-bottom: 1px solid #ddd; }"
    );

    mainLayout->addWidget(m_view);

    // 4. Кнопки управления (Исправленный стиль)
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("Отмена", this);
    QPushButton *saveBtn = new QPushButton("Сохранить", this);

    // ВАЖНО: Указываем border явно, чтобы цвет фона (background-color) применился
    QString commonBtnStyle =
        "QPushButton {"
        "    color: #ffffff;"
        "    font-weight: bold;"
        "    font-size: 13px;"
        "    border-radius: 10px;"
        "    padding: 10px 25px;"
        "    min-width: 100px;"
        "    border: 1px solid transparent;"
        "}";

    cancelBtn->setStyleSheet(commonBtnStyle +
        "QPushButton { background-color: #f44336; }"
        "QPushButton:hover { background-color: #d32f2f; }"
    );

    saveBtn->setStyleSheet(commonBtnStyle +
        "QPushButton { background-color: #2196F3; }"
        "QPushButton:hover { background-color: #1976D2; }"
    );

    btnLayout->addWidget(cancelBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);

    // Загрузка и сигналы
    loadStudents();

    connect(m_searchEdit, &QLineEdit::textChanged, this, &EditGroupStudentsDialog::onSearchChanged);
    connect(m_model, &QStandardItemModel::dataChanged, this, &EditGroupStudentsDialog::onDataChanged);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);

    m_proxyModel->sort(1, Qt::DescendingOrder);
}

void EditGroupStudentsDialog::loadStudents() {
    // Получаем текущий состав группы
    QList<int> currentMembers;
    QSqlQuery q;
    q.prepare("SELECT student_id FROM group_students WHERE group_id = ?");
    q.addBindValue(m_groupId);
    if(q.exec()) {
        while(q.next()) currentMembers.append(q.value(0).toInt());
    }

    // Получаем всех учеников
    q.prepare("SELECT id, full_name FROM users WHERE role = 'student' ORDER BY full_name ASC");
    if (q.exec()) {
        while (q.next()) {
            int id = q.value(0).toInt();
            QString name = q.value(1).toString();

            QStandardItem *nameItem = new QStandardItem(name);
            nameItem->setData(id, Qt::UserRole); // Прячем ID в UserRole

            QStandardItem *checkItem = new QStandardItem();
            checkItem->setCheckable(true);
            checkItem->setCheckState(currentMembers.contains(id) ? Qt::Checked : Qt::Unchecked);

            m_model->appendRow({nameItem, checkItem});
        }
    }
    m_proxyModel->sort(1, Qt::DescendingOrder);
}

void EditGroupStudentsDialog::onSearchChanged(const QString &text) {
    m_proxyModel->setFilterFixedString(text);
}

void EditGroupStudentsDialog::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
    if (topLeft.column() <= 1 && bottomRight.column() >= 1) {
            // Принудительно просим прокси-модель пересчитать порядок строк
        m_proxyModel->sort(1, Qt::DescendingOrder);
    }
}

QList<int> EditGroupStudentsDialog::getSelectedStudentIds() const {
    QList<int> selectedIds;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (m_model->item(i, 1)->checkState() == Qt::Checked) {
            selectedIds.append(m_model->item(i, 0)->data(Qt::UserRole).toInt());
        }
    }
    return selectedIds;
}
