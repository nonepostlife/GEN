#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    createActions();
    createMenus();
    initDatabase();

    ui->showQuestInList->setIcon(QIcon(":/images/icon/downarrow.png"));
    ui->questionTopic->setVisible(false);
    ui->showQuestInList->setStatusTip(tr("Отобразить вопросы для выделенного списка"));
    ui->addQuestionList->setStatusTip(tr("Добавить список вопросов в таблицу"));
    ui->removeQuestionList->setStatusTip(tr("Удалить список вопросов из таблицы"));
    ui->searchLine->setStatusTip(tr("Поиск списка вопросов по названию"));

    GENLIB = new gentickets();
    int result = GENLIB->setConnection(db);
    if(result != 0)
    {
        qDebug() << "База данных не содержит необходимых таблиц";
        return;
    }

    setDefaultTicketsList();

    auto *eqCb = new ComboBoxDelegate(ui->tableQuestList);
    ui->tableQuestList->setItemDelegateForColumn(4, eqCb);
    ui->tableQuestList->setSelectionMode(QAbstractItemView::SingleSelection);
}

#ifndef QT_NO_CONTEXTMENU
void MainWindow::contextMenuEvent(QContextMenuEvent *)
{
    QMenu menu(this);
}


#endif // QT_NO_CONTEXTMENU

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createActions()
{
    genAct = new QAction(tr("Генерация билетов"), this);
    genAct->setShortcuts(QKeySequence::New);
    genAct->setStatusTip(tr("Генерация экзаменационных билетов"));
    genAct->setIcon(QIcon(":/images/icon/gen.png"));
    connect(genAct, &QAction::triggered, this, &MainWindow::generateTickets);

    saveAct = new QAction(tr("Сохранить документ"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Сохранить документ с экзаменационными билетами"));
    saveAct->setIcon(QIcon(":/images/icon/save.png"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveDocument);

    exitAct = new QAction(tr("Выход"), this);
    exitAct->setShortcut(QKeySequence(Qt::Key_Escape));
    exitAct->setStatusTip(tr("Выход из приложения"));
    exitAct->setIcon(QIcon(":/images/icon/EXIT.png"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    tagAct = new QAction(tr("Справка пользователя"), this);
    tagAct->setShortcuts(QKeySequence::AddTab);
    tagAct->setStatusTip(tr("Вывод справки"));
    tagAct->setIcon(QIcon(":/images/icon/info.png"));
    connect(tagAct, &QAction::triggered, this, &MainWindow::showTagHelp);

    aboutAct = new QAction(tr("О программе..."), this);
    aboutAct->setStatusTip(tr("Описание программы"));
    aboutAct->setIcon(QIcon(":/images/icon/info.png"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::aboutProgramm);

    loadQuestInListAct = new QAction(tr("Загрузить вопросы в список"), this);
    loadQuestInListAct->setStatusTip(tr("Загрузить вопросы из файла в выделенный в таблице список вопросов"));
    loadQuestInListAct->setIcon(QIcon(":/images/icon/load.png"));
    connect(loadQuestInListAct, &QAction::triggered, this, &MainWindow::loadQuestInListButton);

    clearQuestInListAct = new QAction(tr("Очистить список"), this);
    clearQuestInListAct->setStatusTip(tr("Удалить вопросы для списка выделенного в таблице списков вопросов"));
    clearQuestInListAct->setIcon(QIcon(":/images/icon/clear.png"));
    connect(clearQuestInListAct, &QAction::triggered, this, &MainWindow::clearQuestInListButton);

    alignmentGroup = new QActionGroup(this);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&Файл"));
    fileMenu->addAction(genAct);
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    questMenu = menuBar()->addMenu(tr("&Вопросы"));
    questMenu->addAction(loadQuestInListAct);
    questMenu->addAction(clearQuestInListAct);

    tableMenu = menuBar()->addMenu(tr("&Справочники"));

    referenceMenu = menuBar()->addMenu(tr("&Справка"));
    referenceMenu->addAction(tagAct);
    referenceMenu->addAction(aboutAct);

    genToolBar = addToolBar(tr("Generate"));
    genToolBar->setIconSize(QSize(35, 35));
    genToolBar->setStyleSheet("QToolButton { padding: 5px;}");
    genToolBar->addAction(genAct);
    genToolBar->addAction(saveAct);
    genToolBar->addSeparator();
    genToolBar->addAction(loadQuestInListAct);
    genToolBar->addAction(clearQuestInListAct);
    genToolBar->addSeparator();
    genToolBar->addAction(tagAct);
    genToolBar->addAction(exitAct);    

}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::closeEvent(QCloseEvent *)
{
    for(int i = 0; i < 2; ++i)
        if(forms[i] != nullptr && forms[i]->isVisible())
            forms[i]->close();
    if(sform != nullptr && sform->isVisible())
        sform->close();
    if(genForm != nullptr && genForm->isVisible())
        genForm->close();

}

void MainWindow::aboutProgramm()
{
    QMessageBox msg;
    msg.setText("Продукт разработан для автоматизации процесса создания экзаменационных билетов.\n\n"
                "Система позволяет хранить и генерировать экзаменационные билеты для указанных дисциплин, включая возможность выбора количества, сложности и тем вопросов.\n\n"
                "Основан на Qt 5.9.0 (MinGW 32 bit)");
    msg.setStandardButtons(QMessageBox::Close);
    msg.setWindowTitle("О программе");
    msg.setIconPixmap(QPixmap(":/images/icon/gen.png"));
    msg.exec();
    return;
}

void MainWindow::showTagHelp()
{
    QDialog *what = new QDialog();
    what->resize(600, 250);
    what->setWindowTitle("Справка по тегам для составления списка вопросов");
    QVBoxLayout * box = new QVBoxLayout();

    QTableWidget * tagTable = new QTableWidget(3, 2);
    tagTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Тег"));
    tagTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Применение"));
    tagTable->verticalHeader()->setVisible(false);
    tagTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tagTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QTableWidgetItem *item = new QTableWidgetItem();
    item->setFlags(Qt::ItemIsEnabled);
    QTableWidgetItem *item2 = new QTableWidgetItem();
    item2->setFlags(Qt::ItemIsEnabled);
    QTableWidgetItem *item3 = new QTableWidgetItem();
    item3->setFlags(Qt::ItemIsEnabled);
    QTableWidgetItem *item4 = new QTableWidgetItem();
    item4->setFlags(Qt::ItemIsEnabled);
    QTableWidgetItem *item5 = new QTableWidgetItem();
    item5->setFlags(Qt::ItemIsEnabled);
    QTableWidgetItem *item6 = new QTableWidgetItem();
    item6->setFlags(Qt::ItemIsEnabled);

    item->setText("#");
    item2->setText("$");
    item3->setText("@");
    item4->setText("Между этими символами пишется тема вопроса\nПример - #Qt#");
    item5->setText("Между этими символами задается сложность вопроса\nПример - $2$");
    item6->setText("Между этими символами пишется текст вопроса\nПример - @Модели Qt для работы с базами данных.@");

    tagTable->setItem(0, 0, item);
    tagTable->setItem(1, 0, item2);
    tagTable->setItem(2, 0, item3);
    tagTable->setItem(0, 1, item4);
    tagTable->setItem(1, 1, item5);
    tagTable->setItem(2, 1, item6);

    QDialogButtonBox * ok = new QDialogButtonBox(QDialogButtonBox::Ok);
    QObject::connect(ok, SIGNAL(accepted()), what, SLOT(accept()));
    QObject::connect(ok, SIGNAL(rejected()), what, SLOT(reject()));
    box->addWidget(tagTable);
    box->addWidget(ok);
    what->setLayout(box);

    auto result = what->exec();
    if(result == QDialog::Accepted)
    {
        what->close();
    }
}

void MainWindow::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("gendb.db");
    //db.setDatabaseName("gentickets.db");
    if (!db.open()){
        qDebug() << "Cannot open database:" << db.lastError();
        return;
    }
    else
    {
        db.exec("pragma foreign_keys=on");
        tables.push_back(table(0, "Disc", "Дисциплина", "discName", "discID", nullptr, nullptr, nullptr, db));
        tables.push_back(table(1, "Dep", "Кафедра", "depName", "depID", nullptr, nullptr, nullptr, db));
        forms.resize(2);
        for(int i = 0; i < 2; ++i)
            forms[i] = nullptr;

        for (auto i = tables.begin(); i != tables.end(); ++i) {
            i->action = tableMenu->addAction(i->actionName);
            i->model = new QSqlTableModel(nullptr, db);
            i->model->setTable(i->tableName);
            i->model->select();
            i->model->setEditStrategy(QSqlTableModel::OnRowChange);

            table* ptr = &(*i);
            connect(i->action, &QAction::triggered, this, [this, ptr](){
                emit openTable(ptr);
            });
        }
        updateTable();
    }
}

void MainWindow::openTable(table *ptr)
{
    if(forms[ptr->number] != nullptr && forms[ptr->number]->isVisible())
    {
        forms[ptr->number]->activateWindow();
        return;
    }
    TableForm * form = new TableForm(ptr, QuestList);

    connect(form, &TableForm::closed, this, [=] {
        updateTable();
        //update2();
        forms[ptr->number] = nullptr;
    });
    forms[ptr->number] = form;
    if(form->isEnabled())
        form->setFocus();
    form->show();
}

void MainWindow::updateTable()
{
    ui->tableQuestList->show();
    QuestList = new QSqlRelationalTableModel(0, db);
    QuestList->setTable("QuestionList");
    QuestList->setHeaderData(1, Qt::Horizontal,tr("Список вопросов"), Qt::EditRole);
    QuestList->setHeaderData(2, Qt::Horizontal,tr("Кафедра"), Qt::EditRole);
    QuestList->setHeaderData(3, Qt::Horizontal,tr("Дисциплина"), Qt::EditRole);
    QuestList->setHeaderData(4, Qt::Horizontal,tr("Семестр"), Qt::EditRole);
    QuestList->setHeaderData(5, Qt::Horizontal,tr("Год"), Qt::EditRole);
    QuestList->setRelation(2, QSqlRelation("Dep", "depID", "depName"));
    QuestList->setRelation(3, QSqlRelation("Disc", "discID", "discName"));
    QuestList->setEditStrategy(QSqlTableModel::OnRowChange);
    QuestList->select();

    TicketsList = new QSqlRelationalTableModel(0, db);
    TicketsList->setTable("TicketsList");
    TicketsList->setHeaderData(1, Qt::Horizontal,tr("Вопросов в билете"), Qt::EditRole);
    TicketsList->setHeaderData(2, Qt::Horizontal,tr("Список билетов"), Qt::EditRole);
    TicketsList->setHeaderData(3, Qt::Horizontal,tr("Кафедра"), Qt::EditRole);
    TicketsList->setHeaderData(4, Qt::Horizontal,tr("Дисциплина"), Qt::EditRole);
    TicketsList->setHeaderData(5, Qt::Horizontal,tr("Семестр"), Qt::EditRole);
    TicketsList->setHeaderData(6, Qt::Horizontal,tr("Год"), Qt::EditRole);
    TicketsList->setHeaderData(7, Qt::Horizontal,tr("Число билетов"), Qt::EditRole);
    TicketsList->setRelation(3, QSqlRelation("Dep", "depID", "depName"));
    TicketsList->setRelation(4, QSqlRelation("Disc", "discID", "discName"));
    TicketsList->setEditStrategy(QSqlTableModel::OnRowChange);
    TicketsList->select();

    ui->tableQuestList->setModel(QuestList);
    ui->tableQuestList->setItemDelegate(new QSqlRelationalDelegate(ui->tableQuestList));
    ui->tableQuestList->setColumnHidden(0, true);
    ui->tableQuestList->setColumnHidden(6, true);
    ui->tableQuestList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableQuestList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tableQuestList->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->tableQuestList->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::generateTickets()
{
    if(genForm != nullptr && genForm->isVisible())
    {
        genForm->activateWindow();
        return;
    }

    QDialog *what = new QDialog();
    what->setMinimumWidth(600);
    what->setMinimumHeight(350);
    what->setMaximumWidth(600);
    what->setMaximumHeight(350);
    what->setWindowIcon(QIcon(":/images/icon/gen.png"));
    what->setWindowTitle("Генерация билетов. Базовая настройка");

    QSqlQueryModel *discModel = new QSqlQueryModel(0), *depModel = new QSqlQueryModel(0);
    discModel->setQuery("select * from Disc");
    depModel->setQuery("select * from Dep");
    QLabel *discLabel = new QLabel("Дисциплина");
    QLabel *depLabel = new QLabel("Кафедра");
    QLabel *nameLabel = new QLabel("Название списка билетов");
    QLabel *sem = new QLabel("Семестр");
    QLabel *yearLabel = new QLabel("Год");
    QLabel *countLabel = new QLabel("Количество билетов");
    QLabel *complexityLabel = new QLabel("Выравнивание по сложности");
    QLabel *topicLabel = new QLabel("Выравнивание по теме");

    QSqlQueryModel *q = new QSqlQueryModel(0);
    q->setQuery("select discName from disc where discID = " + QString::number(lastDiscID));
    QString n1 = q->record(0).field("discName").value().toString();
    QSqlQueryModel *q2 = new QSqlQueryModel(0);
    q2->setQuery("select depName from dep where depID = " + QString::number(lastDepID));
    QString n2 = q2->record(0).field("depName").value().toString();

    QComboBox *discCombo = new QComboBox();
    discCombo->setModel(discModel);
    discCombo->setModelColumn(1);
    discCombo->setCurrentText(n1);
    QComboBox *depCombo = new QComboBox();
    depCombo->setModel(depModel);
    depCombo->setModelColumn(1);
    depCombo->setCurrentText(n2);
    QComboBox *semCombo = new QComboBox();
    semCombo->addItem("Осенний");
    semCombo->addItem("Весенний");
    semCombo->setCurrentText(lastSemester);

    QLineEdit *name = new QLineEdit(lastNameTicketsList);

    QSpinBox *count = new QSpinBox();
    count->setMinimum(1);
    count->setMaximum(100);
    count->setValue(lastCountTickets);

    QSpinBox *year = new QSpinBox();
    year->setMinimum(2000);
    year->setMaximum(10000);
    year->setValue(QDate::currentDate().year());

    QCheckBox *complexityCheck = new QCheckBox();
    complexityCheck->setChecked(lastLevelCheck);
    QCheckBox *topicCheck = new QCheckBox();
    topicCheck->setChecked(lastTopicCheck);

    QHBoxLayout* h1 = new QHBoxLayout();
    h1->addWidget(countLabel);
    h1->addWidget(count);
    QHBoxLayout* h2 = new QHBoxLayout();
    h2->addWidget(complexityLabel);
    h2->addWidget(complexityCheck);
    QHBoxLayout* h3 = new QHBoxLayout();
    h3->addWidget(topicLabel);
    h3->addWidget(topicCheck);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    QPushButton *next = new QPushButton("Далее");
    QPushButton *cancel = new QPushButton("Отмена");
    buttonBox->addButton(next, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(cancel, QDialogButtonBox::RejectRole);

    QObject::connect(buttonBox, SIGNAL(accepted()), what, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), what, SLOT(reject()));

    wLayout = new QVBoxLayout();
    wLayout->addWidget(nameLabel);
    wLayout->addWidget(name);
    wLayout->addWidget(discLabel);
    wLayout->addWidget(discCombo);
    wLayout->addWidget(depLabel);
    wLayout->addWidget(depCombo);
    wLayout->addWidget(sem);
    wLayout->addWidget(semCombo);
    wLayout->addWidget(yearLabel);
    wLayout->addWidget(year);
    wLayout->addLayout(h1);
    wLayout->addLayout(h2);
    wLayout->addLayout(h3);
    wLayout->addSpacing(20);
    wLayout->addWidget(buttonBox);

    what->setLayout(wLayout);
    auto result = what->exec();
    if(result == QDialog::Accepted)
    {
        QString nameList = name->text();
        if(nameList.isEmpty())
        {
            QMessageBox::critical(this, tr("Ошибка!"), "Имя списка не должно быть пустым!");
            return;
        }
        // проверить существует ли список с таким именем
        QSqlQuery q1;
        q1.exec("select ticketsListID from TicketsList where ticketsListName = '" + nameList + "'");
        if(q1.next())
        {
            QMessageBox::critical(this, tr("Ошибка!"), "Список с таким именем уже существует!");
            return;
        }

        lastNameTicketsList = name->text();
        lastCountTickets = count->value();
        lastDepID = depModel->record(depCombo->currentIndex()).field("depID").value().toInt();
        lastDiscID = discModel->record(discCombo->currentIndex()).field("discID").value().toInt();
        lastLevelCheck = complexityCheck->isChecked();
        lastTopicCheck = topicCheck->isChecked();
        lastSemester = semCombo->currentText();

        if(genForm != nullptr && genForm->isVisible())
        {
            genForm->activateWindow();
            return;
        }
        genForm = new GenerateForm(lastNameTicketsList, lastCountTickets, lastDepID, lastDiscID, lastSemester, year->value(), lastLevelCheck, lastTopicCheck, db, this);
        if(genForm->isEnabled())
            genForm->setFocus();
        genForm->show();
    }
    else
    {
        lastNameTicketsList = "";
        lastCountTickets = 1;
        lastDepID = 0;
        lastDiscID = 0;
        lastLevelCheck = false;
        lastTopicCheck = false;
        lastSemester = "Осенний";
    }
}

void MainWindow::setDefaultTicketsList()
{
    lastNameTicketsList = "";
    lastCountTickets = 1;
    lastDepID = 0;
    lastDiscID = 0;
    lastLevelCheck = false;
    lastTopicCheck = false;
    lastSemester = "Осенний";
}

void MainWindow::saveDocument()
{
    if(sform != nullptr && sform->isVisible())
    {
        sform->activateWindow();
        return;
    }
    sform = new saveform(TicketsList, db);
    if(sform->isEnabled())
        sform->setFocus();
    sform->show();
}

/*-----------редактирование списков--------------*/
/*-----------------------------------------------*/

void MainWindow::on_addQuestionList_clicked()
{
    // добавить список - строку таблицы
    int row = QuestList->rowCount();
    QuestList->insertRow(row);
}

void MainWindow::on_removeQuestionList_clicked()
{
    /*  каскадное удаление с вопросом на удаление */
    QModelIndex ind;
    QString str_query, str_query2;
    QSqlQueryModel *find = new QSqlQueryModel;
    QSqlQueryModel *find2 = new QSqlQueryModel;
    QComboBox *temp = new QComboBox;
    QComboBox *temp2 = new QComboBox;

    if(ui->tableQuestList->selectionModel()->currentIndex().isValid())
    {
        int row = ui->tableQuestList->selectionModel()->currentIndex().row();
        QString data = (ui->tableQuestList->model()->data(ui->tableQuestList->model()->index(row, 0))).toString();
        //cout << "data - "<< data.toStdString() << endl;
        str_query = "SELECT questNumInList FROM Questions WHERE questListID = " + data;
        str_query2 = "SELECT ticketsListID FROM QuestOfTickets WHERE questListID = " + data;
        find->setQuery(str_query);
        find2->setQuery(str_query2);
        temp->setModel(find);
        temp2->setModel(find2);

        if(!temp->currentText().isEmpty() || !temp2->currentText().isEmpty()) {
            int result = QMessageBox::question(this, tr("Каскадное удаление!"),
            tr("Внимание! Найдены связанные данные в зависимых таблицах. Удаление приведет к потере зависимых данных. Продолжить?"),
            QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
            if(result == QMessageBox::Yes) {
                // удалить запись и применить изменения в случае согласия пользователя
                QuestList->removeRow(row, ind);
                if(!QuestList->submitAll()) {
                    QMessageBox::critical(this, tr("ERROR!"), QuestList->lastError().databaseText());
                }
            }
        }
        else {
            QuestList->removeRow(row, ind);
            if(!QuestList->submitAll()){
                QMessageBox::critical(this, tr("ERROR!"), QuestList->lastError().databaseText());
            }
        }
    }
    QuestList->select();
}

void MainWindow::loadQuestInListButton()
{
    // FORM
    // проверка выделения списка в таблице
    if(ui->tableQuestList->selectionModel()->currentIndex().isValid())
    {
        int row = ui->tableQuestList->selectionModel()->currentIndex().row();
        int idQuestionList = (ui->tableQuestList->model()->data(ui->tableQuestList->model()->index(row, 0))).toInt();
        // наличие вопросов в списке вопрсов
        QSqlQuery q;
        q.exec("select * from questions where questlistid = " + QString::number(idQuestionList));
        // вопросы имеются
        if(q.next())
        {
            int result = QMessageBox::question(this, tr("Очистка списка!"),
            tr("В список загружены вопросы. При очистке списка вопросов будут удалены все списки билетов, в которых учавствовал данный список вопросов. Вы уверены что хотите очистить список вопросов и продолжить загрузку?"),
            QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
            if(result == QMessageBox::Yes)
            {
                QSqlQuery q;
                q.prepare("select distinct ticketsListID from questoftickets where questlistid = ?");
                q.addBindValue(idQuestionList);
                q.exec();

                while(q.next())
                {
                    int ticketsListID = q.value(0).toInt();
                    QSqlQuery delQuery;
                    delQuery.prepare("delete from ticketsList where ticketsListID = ?");
                    delQuery.addBindValue(ticketsListID);
                    delQuery.exec();
                }

                int res = GENLIB->removeQuestionsFromList(idQuestionList);
                if(res != 0)
                {
                    QMessageBox::critical(this, tr("Ошибка!"), "Невозможно очистить список");
                    return;
                }
                // очистить список выведенных вопросов если удаляется текущий список
                if(idQuestionList == lastQuestListID)
                {
                    ui->tableQuestList->clearSelection();
                    ui->questionTopic->clear();
                    ui->questionTopic->setVisible(false);
                    ui->questListView->setModel(nullptr);
                }
                dropDialog *dialog = new dropDialog(idQuestionList, db);
                dialog->show();
            }
        }
        else
        {
            dropDialog *dialog = new dropDialog(idQuestionList, db);
            dialog->show();
        }
    }
    // если список не выделен
    else
    {
        QMessageBox::critical(this, tr("Ошибка!"), "Выберите список для загрузки вопросов");
    }
}

void MainWindow::on_showQuestInList_clicked()
{
    // проверка выделения списка в таблице
    if(ui->tableQuestList->selectionModel()->currentIndex().isValid())
    {
        int row = ui->tableQuestList->selectionModel()->currentIndex().row();
        int idQuestionList = (ui->tableQuestList->model()->data(ui->tableQuestList->model()->index(row, 0))).toInt();

        // ID последнего списка для комбобокса
        lastQuestListID = idQuestionList;
        // отобразить вопросы
        showQuestionsInList(idQuestionList);
        //ui->questionTopic->setCurrentIndex(0);

        QModelIndex index = ui->tableQuestList->currentIndex();
        ui->tableQuestList->setFocus();
        ui->tableQuestList->selectRow(index.row());
    }
    // если список не выделен
    else
    {
        QMessageBox::information(this, tr("Внимание!"), "Выберите список для которого необходимо показать вопросы");
        return;
    }
}

void MainWindow::showQuestionsInList(int questListID)
{
    // запрос на выборку вопрос для текущего списка
    questions = new QSqlQueryModel(0);
    questions->setQuery("select questLevel, questContent from questions where questlistid = " + QString::number(questListID));

    // проверка выполнения запроса
    if (questions->lastError().isValid()){
        QMessageBox::critical(this, tr("Ошибка!"), questions->lastError().text());
        qDebug() << questions->lastError();
        return;
    }
    // если для списка существуют вопросы - выводим вопросы для этого списка
    if(questions->hasIndex(0, 0))
    {
        // настройка таблицы
        ui->questListView->setModel(questions);
        questions->setHeaderData(0, Qt::Horizontal,tr("Сложность"), Qt::EditRole);
        questions->setHeaderData(1, Qt::Horizontal,tr("Содержание вопроса"), Qt::EditRole);
        ui->questListView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        ui->questListView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        // настройка комбобокса
        ui->questionTopic->setVisible(true);
        QSqlQueryModel* comboQuery = new QSqlQueryModel(0);
        comboQuery->setQuery("select distinct questtopic from questions where questlistid = " + QString::number(questListID));

        // проверка выполнения запроса
        if (comboQuery->lastError().isValid()){
            QMessageBox::critical(this, tr("Ошибка!"), comboQuery->lastError().text());
            qDebug() << comboQuery->lastError();
            return;
        }
        ui->questionTopic->setModel(comboQuery);
    }
    // иначе выводим ошибку
    else
    {
        QMessageBox::critical(this, tr("Ошибка!"), "В данный список не загружены вопросы");
        ui->questionTopic->clear();
        ui->questionTopic->setVisible(false);
        ui->questListView->setModel(nullptr);
    }
}

void MainWindow::clearQuestInListButton()
{
    // проверка выделения списка в таблице
    if(ui->tableQuestList->selectionModel()->currentIndex().isValid())
    {
        int row = ui->tableQuestList->selectionModel()->currentIndex().row();
        int idQuestionList = (ui->tableQuestList->model()->data(ui->tableQuestList->model()->index(row, 0))).toInt();

        // проверить загружены ли в список вопросы
        QSqlQuery query;
        query.exec("select * from questions where questlistid = " + QString::number(idQuestionList));
        if(!query.next())
        {
            QMessageBox::critical(this, tr("Ошибка!"), "В данный список не загружены вопросы. Список вопросов пуст. Невозможно очистить");
            return;
        }
        // если в список загружены вопросы
        int result = QMessageBox::question(this, tr("Очистка списка!"),
        tr("В список загружены вопросы. При очистке списка вопросов будут удалены все списки билетов, в которых учавствовал данный список вопросов. Вы уверены что хотите очистить список вопросов?"),
        QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
        if(result == QMessageBox::Yes)
        {
            QSqlQuery q;
            q.prepare("select distinct ticketsListID from questoftickets where questlistid = ?");
            q.addBindValue(idQuestionList);
            q.exec();

            while(q.next())
            {
                int ticketsListID = q.value(0).toInt();
                QSqlQuery delQuery;
                delQuery.prepare("delete from ticketsList where ticketsListID = ?");
                delQuery.addBindValue(ticketsListID);
                delQuery.exec();
            }

            int result = GENLIB->removeQuestionsFromList(idQuestionList);
            if(result != 0)
            {
                QMessageBox::critical(this, tr("Ошибка!"), "Невозможно очистить список");
                return;
            }

            // очистить список выведенных вопросов если удаляется текущий список
            if(idQuestionList == lastQuestListID)
            {
                ui->tableQuestList->clearSelection();
                ui->questionTopic->clear();
                ui->questionTopic->setVisible(false);
                ui->questListView->setModel(nullptr);
            }
        }
    }
    // если список не выделен
    else
    {
        QMessageBox::information(this, tr("Внимание!"), "Выберите список который хотите очистить");
        return;
    }
}

void MainWindow::on_questionTopic_currentIndexChanged(const QString &arg1)
{
    // запрос на выборку вопрос для текущего списка
    questions = new QSqlQueryModel(0);
    questions->setQuery("select questLevel, questContent from questions where questlistid = " + QString::number(lastQuestListID) +
                        " and questtopic = '" + arg1 +"'");

    // проверка выполнения запроса
    if (questions->lastError().isValid()){
        QMessageBox::critical(this, tr("Ошибка!"), questions->lastError().text());
        qDebug() << questions->lastError();
        return;
    }

    //ui->questListView->setModel(questions);

    // настройка таблицы
    ui->questListView->setModel(questions);
    questions->setHeaderData(0, Qt::Horizontal,tr("Сложность"), Qt::EditRole);
    questions->setHeaderData(1, Qt::Horizontal,tr("Содержание вопроса"), Qt::EditRole);
    ui->questListView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->questListView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::on_searchLine_textChanged(const QString &arg1)
{
    if(arg1.isEmpty())
        QuestList->setFilter("");
    else
        QuestList->setFilter("UPPER(questListName) like upper('%" + arg1 + "%')");
        QuestList->select();
}
