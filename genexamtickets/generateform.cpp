#include "generateform.h"
#include "ui_generateform.h"

GenerateForm::GenerateForm(QString nameTicketsList, int countTickets, int indepID, int indiscID, QString inlastSemester, int year, bool level, bool topic, QSqlDatabase indb, QMainWindow *mainForm, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GenerateForm)
{
    ui->setupUi(this);
    setWindowTitle("Генерация билетов. Стрктура билетов в списке '" + nameTicketsList + "'");
    resize(600, 330);
    setMinimumSize(600, 330);

    connect(ui->backForm,&QPushButton::clicked, this, &GenerateForm::sendData);
    //connect(ui->generate, SIGNAL(clicked()), mainForm, SLOT(setDefaultTicketsList()));
    connect(this, SIGNAL(sendData()), mainForm, SLOT(generateTickets()));

    nameTicket = nameTicketsList;
    count = countTickets;
    flevel = level;
    ftopic = topic;
    db = indb;
    depID = indepID;
    discID = indiscID;
    semester = inlastSemester;
    this->year = year;

    GENLIB = new gentickets();
    GENLIB->setConnection(db);

    ui->toRight->setIcon(QIcon(":/images/icon/rightarrow.png"));
    ui->toLeft->setIcon(QIcon(":/images/icon/leftarrow.png"));
    ui->sortUp->setIcon(QIcon(":/images/icon/uparrow.png"));
    ui->sortDown->setIcon(QIcon(":/images/icon/downarrow.png"));
    mform = mainForm;


    // список из БД
    QStringList itemsDB;
    QSqlQuery *q = new QSqlQuery();
    q->exec("select questListName from QuestionList where discID = " + QString::number(discID) + " and depID = " + QString::number(depID));
    while(q->next())
        itemsDB.append(q->value(0).toString());
    DragDropListModel *dragmodel = new DragDropListModel(itemsDB);

    ui->QuestListView->setModel(dragmodel);
    ui->QuestListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->QuestListView->setDragEnabled(true);
    //ui->QuestListView->setAcceptDrops(true);
    ui->QuestListView->setDropIndicatorShown(true);


    // список для структуры билета
    QStringList items;

    listModel = new DragDropListModel(items, 1, this);
    ui->listView->setModel(listModel);
    ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listView->setDragEnabled(true);
    ui->listView->setAcceptDrops(true);
    ui->listView->setDropIndicatorShown(true);

    if(!flevel)
    {
        ui->levelLabel->setVisible(false);
        ui->levelRate->setVisible(false);
    }
    if(!ftopic)
    {
        ui->topicLabel->setVisible(false);
        ui->topicRate->setVisible(false);
    }
}

GenerateForm::~GenerateForm()
{
    delete ui;
}

void GenerateForm::on_backForm_clicked()
{
    emit close();
    //emit sendData();
}


void GenerateForm::on_toLeft_clicked()
{
    QModelIndex index = ui->QuestListView->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    listModel->insertRow(listModel->rowCount());
    QModelIndex index2 = listModel->index(listModel->rowCount() - 1);
    listModel->setData(index2, itemText);
}

void GenerateForm::on_toRight_clicked()
{
    if(ui->listView->selectionModel()->currentIndex().isValid())
    {
        int _Row = ui->listView->selectionModel()->currentIndex().row();
        listModel->removeRows(_Row, 1);
    }
}

void GenerateForm::on_sortUp_clicked()
{
    if(ui->listView->selectionModel()->currentIndex().isValid())
    {
        QModelIndex index = ui->listView->selectionModel()->currentIndex();
        QString itemText = index.data(Qt::DisplayRole).toString();
        int _Row = ui->listView->selectionModel()->currentIndex().row();

        listModel->removeRows(_Row, 1);
        listModel->insertRow(_Row - 1);
        QModelIndex index2 = listModel->index(_Row - 1);
        listModel->setData(index2, itemText);
        ui->listView->clearSelection();
    }
}

void GenerateForm::on_sortDown_clicked()
{
    if(ui->listView->selectionModel()->currentIndex().isValid())
    {
        QModelIndex index = ui->listView->selectionModel()->currentIndex();
        QString itemText = index.data(Qt::DisplayRole).toString();
        int _Row = ui->listView->selectionModel()->currentIndex().row();

        listModel->removeRows(_Row, 1);
        listModel->insertRow(_Row + 1);
        QModelIndex index2 = listModel->index(_Row + 1);
        listModel->setData(index2, itemText);
        ui->listView->clearSelection();
    }
}

int GenerateForm::on_generate_clicked()
{
    if(ui->listView->model()->rowCount() == 0)
    {
        QMessageBox::critical(this, tr("Ошибка!"), "Укажите списки вопросов для генерации!");
        return -1;
    }
    // добавление списка билетов
    QSqlQuery query;
    query.prepare("insert into TicketsList(questNumInTicket, ticketsListName, "
                  "depID, discID, semester, year, ticketsNum) values (?,?,?,?,?,?,?)");

    query.addBindValue(ui->listView->model()->rowCount());
    query.addBindValue(nameTicket);
    query.addBindValue(depID);
    query.addBindValue(discID);
    query.addBindValue(semester);
    query.addBindValue(year);
    query.addBindValue(count);
    query.exec();

    if(query.lastError().isValid())
    {
        QMessageBox::critical(this, tr("Ошибка!"), query.lastError().text());
        return -1;
    }
    // ID списка билетов
    QSqlQuery q1;
    q1.exec("select ticketsListID from TicketsList where ticketsListName = '" + nameTicket + "'");
    if(!q1.next())
    {
        QMessageBox::critical(this, tr("Ошибка!"), q1.lastError().text());
        return -1;
    }
    int ticketsListID = q1.record().value(0).toInt();

    for(int i = 0; i < ui->listView->model()->rowCount(); i++)
    {
        QModelIndex index = ui->listView->model()->index(i,0);
        QString itemText = index.data(Qt::DisplayRole).toString();
        QSqlQuery q;
        q.exec("select questListID from QuestionList where questListName = '" + itemText + "'");
        if(!q.next())
        {
            QMessageBox::critical(this, tr("Ошибка!"), q.lastError().text());
            return -1;
        }
        int idList = q.record().value(0).toInt();

        // insert QuestOfTickets
        int numQuestInTicket = i + 1;
        QSqlQuery in;
        in.prepare("insert into QuestOfTickets(ticketsListID, questListID, questNumInTicket) values (?,?,?)");
        in.addBindValue(ticketsListID);
        in.addBindValue(idList);
        in.addBindValue(numQuestInTicket);
        in.exec();
    }

    // если список 1 отключить выравнивание
    if(ui->listView->model()->rowCount() < 2)
    {
        flevel = false;
        ftopic = false;
    }

    int result = 0;
    double levelrate = (double) ui->levelRate->value();
    double topicrate = (double) ui->topicRate->value();
    if(flevel && ftopic)
        result = GENLIB->generateTickets(ticketsListID, levelrate, topicrate);
    else if (flevel)
        result = GENLIB->generateTickets(ticketsListID, levelrate, -1);
    else if (ftopic)
        result = GENLIB->generateTickets(ticketsListID, -1, topicrate);
    else
        result = GENLIB->generateTickets(ticketsListID);

    qDebug() << result;

    switch (result) {
    case 0:
        QMessageBox::information(this, "Генерация билетов", "Билеты успешно сгенерированы!");
        close();
        break;
    case 1:
        QMessageBox::critical(this, "Генерация билетов", "Количество определенных вопросов не равно количеству вопросов в билете!");
        break;
    case 2:
        QMessageBox::critical(this, "Генерация билетов", "Не удалось сгенерировать билеты с выравниваем по сложности и по теме. "
                                                         "Попробуйте увеличить коэффициент, либо изменить списки вопросов!");
        break;
    case 3:
        QMessageBox::critical(this, "Генерация билетов", "Не удалось сгенерировать билеты с выравниваем по сложности. "
                                                         "Попробуйте увеличить коэффициент, либо изменить списки вопросов!");
        break;
    case 4:
        QMessageBox::critical(this, "Генерация билетов", "Не удалось сгенерировать билеты с выравниваем по теме. "
                                                         "Попробуйте увеличить коэффициент, либо изменить списки вопросов!");
        break;
    default:
        break;
    }
    // если билеты не сгенерированы удалить список
    if(result != 0)
    {
        QSqlQuery q;
        q.prepare("delete from TicketsList where ticketsListID = ?");
        q.addBindValue(ticketsListID);
        q.exec();
    }
}
