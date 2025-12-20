#include "tablemanager.h"
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


TableManager::TableManager(QObject *parent) : QObject(parent)
{

}

void TableManager::setupTable(const QString &tableName, QTableView *view) {
    if (model) model->deleteLater();
    if (proxyModel) proxyModel->deleteLater();

    model = new QSqlRelationalTableModel(this, QSqlDatabase::database());
    model->setTable(tableName);

    // Настройка связей (Relations)
    if (tableName == "groups") {
        model->setRelation(3, QSqlRelation("users", "id", "full_name"));

        // Проверяем, добавлен ли столбец (если нет в БД)
        if (model->columnCount() <= 4) {
            model->insertColumn(4);
            model->setHeaderData(4, Qt::Horizontal, "Состав");
        }

        // Создаем делегат и вешаем его ТОЛЬКО на 4-й столбец
        ButtonDelegate *btnDelegate = new ButtonDelegate(view);
        view->setItemDelegateForColumn(4, btnDelegate);

        // Соединяем сигнал (теперь в лямбде можно открыть ваше модальное окно)
        connect(btnDelegate, &ButtonDelegate::buttonClicked, this, [this](const QModelIndex &index) {
            qDebug() << "Нажата кнопка в строке:" << index.row();
            // Здесь вызывайте AttendanceAddDialog или другое окно
        });
    }
    else if (tableName == "schedule") {
        // Столбец 1 (group_id) -> таблица groups (id), показ name
        model->setRelation(1, QSqlRelation("groups", "id", "name"));
    }
    else if (tableName == "attendance") {
        model->setRelation(1, QSqlRelation("users", "id", "full_name"));
        model->setRelation(2, QSqlRelation("groups", "id", "name"));
    }

    // Устанавливаем стратегию ManualSubmit, чтобы onModelDataChanged мог валидировать данные
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Загружаем данные
    if (!model->select()) {
        qDebug() << "Ошибка загрузки таблицы:" << model->lastError().text();
    }

    // Добавляем виртуальный столбец для кнопки, если это Группы
    if (tableName == "groups") {
        model->insertColumn(4);
    }

    // Настройка нашей кастомной мульти-фильтр прокси-модели
    proxyModel = new MultiFilterProxyModel();
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    view->setModel(proxyModel);

    view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
    if (event->type() == QEvent::MouseButtonRelease) {
        QString currentStatus = index.data().toString();
        model->setData(index, (currentStatus == "present" ? "absent" : "present"));
        return true;
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

        // Повторяем расчет геометрии кнопки для проверки попадания
        int btnWidth = option.rect.width() / 2;
        int xOffset = (option.rect.width() - btnWidth) / 2;
        QRect btnRect = option.rect.adjusted(xOffset, 4, -xOffset, -4);

        if (btnRect.contains(mouseEvent->pos())) {
            // Исправляем вызов: передаем index, а не только row, если нужно больше данных
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
