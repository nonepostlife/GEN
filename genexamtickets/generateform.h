#ifndef GENERATEFORM_H
#define GENERATEFORM_H

#include <QStringList>
#include <QMainWindow>
#include <QMenu>
#include <QIcon>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QKeyEvent>
#include <QFileDialog>
#include <QTableWidget>
#include <QStandardItem>
#include <QCheckBox>
#include <QListView>
#include <QDebug>

#include <QtSql/QSqlDatabase>
#include <QtSql>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlQuery>

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <mainwindow.h>

#include "gentickets.h"
#include "gentickets_global.h"

#include <dragdroplistmodel.h>

using namespace std;

class QAction;
class QMenu;
class QToolBar;
class QMainWindow;

namespace Ui {
class GenerateForm;
}

class GenerateForm : public QMainWindow
{
    Q_OBJECT
signals:
    void sendData();
    bool someSignal(QMap<int, QList<int> > map, int ticketsNum);

public:
    explicit GenerateForm(QString innameTicketsList, int incountTickets, int indepID, int indiscID,
                          QString inlastSemester, int year, bool inlevel, bool intopic, QSqlDatabase indb,
                          QMainWindow *mainForm, QWidget *parent = 0);
    ~GenerateForm();

private slots:
    void on_toLeft_clicked();

    void on_toRight_clicked();

    void on_sortUp_clicked();

    void on_sortDown_clicked();

    int on_generate_clicked();

    void on_backForm_clicked();

private:
    Ui::GenerateForm                *ui;

    gentickets                      *GENLIB;
    QSqlDatabase                    db;
    DragDropListModel                *listModel;
    QMainWindow                     *mform;
    QString                         nameTicket, semester;
    int                             count, year, depID, discID;
    bool                            flevel, ftopic;
};

#endif // GENERATEFORM_H
