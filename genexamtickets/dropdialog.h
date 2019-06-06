#ifndef DROPDIALOG_H
#define DROPDIALOG_H

#include "mainwindow.h"
#include <QMainWindow>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>


#include "gentickets.h"
#include "gentickets_global.h"


namespace Ui {
class dropDialog;
}

class dropDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit dropDialog(int id, QSqlDatabase db, QWidget *parent = 0);
    ~dropDialog();

private slots:


    void on_openFileBtn_clicked();

    void on_loadFileBtn_clicked();

    void on_cancelBtn_clicked();

    void on_lineEdit_textEdited(const QString &arg1);

    void on_editFileBtn_clicked();

protected:
    void        dragEnterEvent(QDragEnterEvent *event);
    void        dragLeaveEvent(QDragLeaveEvent *event);
    void        dropEvent(QDropEvent *e);

private:
    Ui::dropDialog      *ui;
    int                 idList;
    QColor              defaultColor;
    QSqlDatabase        database;
    gentickets          *GENLIB;
    QProcess            *proc;
};

#endif // DROPDIALOG_H
