#ifndef TABLEFORM_H
#define TABLEFORM_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QMessageBox>

#include <QtSql/QSqlDatabase>
#include <QtSql>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlQuery>

#include <iostream>

using namespace std;
namespace Ui {
class TableForm;
}

struct table
{
    table(int number, QString tableName, QString actionName, QString filter, QString id,
          QAction* action, QSqlTableModel* model, QSqlQueryModel* sub, QSqlDatabase db) :
        number(number), tableName(tableName), actionName(actionName), filter(filter), id(id),
        action(action), model(model), sub(sub), db(db){ }
    table() { }
    int number;
    QString tableName;
    QString actionName;
    QString filter;
    QString id;
    QAction* action;
    QSqlTableModel* model;
    QSqlQueryModel* sub;
    QSqlDatabase db;
};

class TableForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit TableForm(table* t, QSqlRelationalTableModel* questionTable, QWidget *parent = 0);
    ~TableForm();

signals:
    void closed();

private slots:
    void on_addButton_clicked();

    void on_deleteButton_clicked();

    void on_searchLine_textChanged(const QString &arg1);

    void keyPressEvent(QKeyEvent* event);

    void on_comboDiscipline_currentIndexChanged(int index);

    void closeEvent(QCloseEvent *event);

    void accept();

private:
    Ui::TableForm                   *ui;
    table                           *t;
    QSqlRelationalTableModel        *questTable;
};

#endif // TABLEFORM_H
