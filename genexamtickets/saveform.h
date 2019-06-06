#ifndef SAVEFORM_H
#define SAVEFORM_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>

#include <QtSql/QSqlDatabase>
#include <QtSql>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlQuery>

#include "gentickets.h"
#include "gentickets_global.h"

namespace Ui {
class saveform;
}

class saveform : public QMainWindow
{
    Q_OBJECT

public:
    explicit saveform(QSqlRelationalTableModel* ticketsListTable, QSqlDatabase indb, QWidget *parent = 0);
    ~saveform();

private slots:
    void on_removeButton_clicked();

    void on_printButton_clicked();

    void on_tableTicketsList_clicked(const QModelIndex &index);

    void on_deleteListAction_triggered();

    void on_saveAction_triggered();

    void on_exitAction_triggered();

private:
    Ui::saveform                    *ui;
    gentickets                      *GENLIB;
    QSqlDatabase                    db;
    QSqlRelationalTableModel        *TicketsList;
    QProcess                        *proc;
    void                            deleteList();
    void                            save();
};

#endif // SAVEFORM_H
