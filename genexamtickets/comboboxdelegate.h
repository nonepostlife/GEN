#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QtCore>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>


class ComboBoxDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    ComboBoxDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // COMBOBOXDELEGATE_H
