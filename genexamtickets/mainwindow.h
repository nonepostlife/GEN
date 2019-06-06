#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
#include <QSpinBox>

#include <QtSql/QSqlDatabase>
#include <QtSql>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QListView>
#include <QDebug>

#include <comboboxdelegate.h>
#include <dropdialog.h>
#include <tableform.h>
#include <saveform.h>
#include <generateform.h>
#include <gentickets.h>
#include <gentickets_global.h>

#include <JlCompress.h>

using namespace std;

class QAction;
class QMenu;
class QToolBar;
class GenerateForm;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

public slots:
    void                        openTable(table* ptr);
    void                        closeEvent(QCloseEvent *);
    void                        generateTickets();
    void                        setDefaultTicketsList();

private slots:
    void                        on_addQuestionList_clicked();
    void                        on_removeQuestionList_clicked();
    void                        on_showQuestInList_clicked();
    void                        on_questionTopic_currentIndexChanged(const QString &arg1);
    void                        on_searchLine_textChanged(const QString &arg1);

    void                        saveDocument();
    void                        aboutProgramm();
    void                        showTagHelp();
    void                        loadQuestInListButton();
    void                        clearQuestInListButton();

private:
    Ui::MainWindow *ui;

    void                        createActions();
    void                        createMenus();
    void                        createStatusBar();

    void                        initDatabase();
    void                        updateTable();
    void                        showQuestionsInList(int questListID);

    QSqlDatabase                    db;
    QSqlQueryModel                  *questions;
    QSqlTableModel                  *Disc, *Dep;
    QSqlRelationalTableModel        *QuestList, *TicketsList;
    QVector<table>                  tables;

    QVector<TableForm*>             forms;


    QMenu                           *fileMenu, *tableMenu, *referenceMenu, *questMenu;

    QToolBar                        *genToolBar;

    QAction                         *genAct, *saveAct, *exitAct,
                                    *editAct, *disciplineAct,
                                    *tagAct, *aboutAct,
                                    *loadQuestInListAct, *clearQuestInListAct;

    QActionGroup                    *alignmentGroup;

    QVBoxLayout                     *wLayout = nullptr;
    GenerateForm                    *genForm = nullptr;
    saveform                        *sform = nullptr;

    int                             lastQuestListID, lastDepID, lastDiscID;
    QString                         lastNameTicketsList, lastSemester;
    int                             lastCountTickets;
    bool                            lastTopicCheck, lastLevelCheck;

    gentickets                      *GENLIB;
    QProcess                        *proc;
};

#endif // MAINWINDOW_H
