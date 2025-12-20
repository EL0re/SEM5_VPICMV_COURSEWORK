#include "tablemanager.h"
#include "studentsortproxymodel.h"
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QSqlRelation>
#include <QSqlRelationalDelegate>
#include <QDebug>
#include <QSqlError>
#include <QHeaderView>
#include <QSqlRecord>


TableManager::TableManager(QObject *parent) : QObject(parent)
{

}

void TableManager::setupTable(const QString &tableName, QTableView *view) {
    // 1. Полная очистка предыдущих моделей
    if (model) { model->deleteLater(); model = nullptr; }
    if (proxyModel) { proxyModel->deleteLater(); proxyModel = nullptr; }

    model = new QSqlRelationalTableModel(this, QSqlDatabase::database());
    model->setTable(tableName);

    // 2. Настройка связей (Relations)
    if (tableName == "groups") {
        model->setRelation(3, QSqlRelation("users", "id", "full_name"));
    } else if (tableName == "schedule") {
        model->setRelation(1, QSqlRelation("groups", "id", "name"));
    } else if (tableName == "attendance") {
        model->setRelation(1, QSqlRelation("users", "id", "full_name"));
        model->setRelation(2, QSqlRelation("groups", "id", "name"));
    }

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 3. Загрузка данных из БД
    if (!model->select()) {
        qDebug() << "SQL Error:" << model->lastError().text();
    }

    // 4. Добавление виртуальной колонки "Состав" (только для групп)
    int btnColumnIdx = -1;
    if (tableName == "groups") {
        btnColumnIdx = model->columnCount();
        model->insertColumn(btnColumnIdx);
        model->setHeaderData(btnColumnIdx, Qt::Horizontal, "Состав");
    }

    // 5. Настройка прокси-модели
    proxyModel = new MultiFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    view->setModel(proxyModel);

    // 6. Очистка старых делегатов (ставим стандартный для всех колонок)
    for (int i = 0; i < view->model()->columnCount(); ++i) {
        view->setItemDelegateForColumn(i, new QStyledItemDelegate(view));
    }

    // 7. Настройка специфичных делегатов
    if (tableName == "groups" && btnColumnIdx != -1) {
        // Делегат для выбора тренера (Combo Box)
        view->setItemDelegateForColumn(3, new QSqlRelationalDelegate(view));

        // Наш делегат кнопки
        ButtonDelegate *btnDelegate = new ButtonDelegate(view);
        view->setItemDelegateForColumn(btnColumnIdx, btnDelegate);

        // Соединяем сигнал кнопки с открытием диалога
        connect(btnDelegate, &ButtonDelegate::buttonClicked, this, [this, view, btnColumnIdx](const QModelIndex &proxyIndex) {
            if (!proxyIndex.isValid() || proxyIndex.column() != btnColumnIdx) return;

            // Преобразуем индекс прокси в исходный индекс модели
            QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
            int row = sourceIndex.row();

            // Получаем ID группы динамически
            int idCol = model->record().indexOf("id");
            if (idCol == -1) idCol = 0;

            int groupId = model->data(model->index(row, idCol)).toInt();
            QString groupName = model->data(model->index(row, 1)).toString();

            qDebug() << "Попытка открытия состава группы:" << groupName << "ID:" << groupId;

            if (groupId <= 0) {
                qDebug() << "Ошибка: ID записи не найден. Сохраните строку перед редактированием.";
                return;
            }

            // Создаем и открываем диалог
            EditGroupStudentsDialog dialog(groupId, groupName, view->window());
            if (dialog.exec() == QDialog::Accepted) {
                QList<int> selectedIds = dialog.getSelectedStudentIds();
                QSqlDatabase db = QSqlDatabase::database();

                if (db.transaction()) {
                    QSqlQuery q;
                    q.prepare("DELETE FROM group_students WHERE group_id = ?");
                    q.addBindValue(groupId);
                    q.exec();

                    q.prepare("INSERT INTO group_students (group_id, student_id) VALUES (?, ?)");
                    for (int sId : selectedIds) {
                        q.addBindValue(groupId);
                        q.addBindValue(sId);
                        q.exec();
                    }
                    if (db.commit()) {
                        qDebug() << "Состав успешно обновлен.";
                    } else {
                        db.rollback();
                        qDebug() << "Ошибка при сохранении состава:" << db.lastError().text();
                    }
                }
            }
        }, Qt::UniqueConnection);
    }
    else if (tableName == "attendance") {
        // Ставим чекбокс для посещаемости
        view->setItemDelegateForColumn(4, new CheckBoxDelegate(view));
    }

    // 8. Настройки отображения таблицы
    view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->viewport()->update();
}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // Возвращаем nullptr, чтобы текстовое поле (QLineEdit) не создавалось при клике
    return nullptr;
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
    // Проверяем, что это отпускание кнопки мыши
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QString currentStatus = index.data().toString();
            // Инвертируем статус
            QString newStatus = (currentStatus == "present" ? "absent" : "present");

            // Записываем данные в модель
            model->setData(index, newStatus, Qt::EditRole);
            return true;
        }
    }
    return false;
}

void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // Рассчитываем размеры: ширина в 2 раза меньше ячейки
    int btnWidth = option.rect.width() / 2;
    int xOffset = (option.rect.width() - btnWidth) / 2;

    // Создаем прямоугольник кнопки с отступами сверху/снизу (4px)
    QRect btnRect = option.rect.adjusted(xOffset, 4, -xOffset, -4);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Рисуем синий фон кнопки (стиль как у QLineEdit:focus или кнопок сохранения)
    painter->setBrush(QColor("#2196F3"));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(btnRect, 8, 8); // Скругление 8px

    // Рисуем белый текст по центру
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(btnRect, Qt::AlignCenter, "Ред.");

    painter->restore();
}

bool ButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        // Расчет области кнопки
        int btnWidth = option.rect.width() / 2;
        int xOffset = (option.rect.width() - btnWidth) / 2;
        QRect btnRect = option.rect.adjusted(xOffset, 4, -xOffset, -4);

        if (btnRect.contains(mouseEvent->pos())) {
            // Убедитесь, что здесь НЕТ qDebug. Только сигнал:
            emit buttonClicked(index);
            return true;
        }
    }
    return false;
}

void TableManager::applyMultiFilter(const QMap<int, QString> &columnFilters) {
    // Приводим к нашему типу MultiFilterProxyModel
    auto multiProxy = qobject_cast<MultiFilterProxyModel*>(proxyModel);

    if (multiProxy) {
        // Вызываем наш публичный метод, который мы только что создали
        multiProxy->setColumnFilters(columnFilters);
    } else {
        // Если вдруг proxyModel еще старого типа, просто логируем
        qDebug() << "Ошибка: proxyModel не является MultiFilterProxyModel";
    }
}
