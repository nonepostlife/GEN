#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QStringListModel>

class DragDropListModel : public QStringListModel
{
    Q_OBJECT

public:
    DragDropListModel(const QStringList &strings, int mode = 0, QObject *parent = 0);

    Qt::ItemFlags   flags(const QModelIndex &index) const;

    bool            dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    QMimeData       *mimeData(const QModelIndexList &indexes) const;
    QStringList     mimeTypes() const;
    Qt::DropActions supportedDropActions() const;

private:
    int             mode;
};

#endif // DRAGDROPLISTMODEL_H
