#include "saveform.h"
#include "ui_saveform.h"

saveform::saveform(QSqlRelationalTableModel *ticketsListTable, QSqlDatabase indb, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::saveform)
{
    ui->setupUi(this);
    setWindowTitle("Сохранение билетов");
    resize(900, 450);
    setMinimumSize(900, 450);

    db = indb;
    TicketsList = ticketsListTable;
    TicketsList->select();

    ui->tableTicketsList->show();
    ui->tableTicketsList->setModel(TicketsList);
    ui->tableTicketsList->setItemDelegate(new QSqlRelationalDelegate(ui->tableTicketsList));
    ui->tableTicketsList->setColumnHidden(0, true);
    ui->tableTicketsList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableTicketsList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tableTicketsList->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tableTicketsList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableTicketsList->setSelectionMode(QAbstractItemView::SingleSelection);

    GENLIB = new gentickets();
    GENLIB->setConnection(db);
}

saveform::~saveform()
{
    delete ui;
}

void saveform::on_exitAction_triggered()
{
    close();
}

void saveform::on_removeButton_clicked()
{
    deleteList();
}

void saveform::on_printButton_clicked()
{
    save();
}

void saveform::on_deleteListAction_triggered()
{
    deleteList();
}

void saveform::on_saveAction_triggered()
{
    save();
}

void saveform::deleteList()
{
    QModelIndex ind;
    QString str_query, str_query2;
    QSqlQueryModel *find = new QSqlQueryModel;
    QSqlQueryModel *find2 = new QSqlQueryModel;
    QComboBox *temp = new QComboBox;
    QComboBox *temp2 = new QComboBox;

    if(ui->tableTicketsList->selectionModel()->currentIndex().isValid())
    {
        int row = ui->tableTicketsList->selectionModel()->currentIndex().row();
        QString data = (ui->tableTicketsList->model()->data(ui->tableTicketsList->model()->index(row, 0))).toString();
        //cout << "data - "<< data.toStdString() << endl;
        str_query = "SELECT questNumInTicket FROM Tickets WHERE ticketsListID = " + data;
        str_query2 = "SELECT questListID FROM QuestOfTickets WHERE ticketsListID = " + data;
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
                TicketsList->removeRow(row, ind);
                if(!TicketsList->submitAll()) {
                    QMessageBox::critical(this, tr("ERROR!"), TicketsList->lastError().databaseText());
                }
            }
        }
        else {
            TicketsList->removeRow(row, ind);
            if(!TicketsList->submitAll()){
                QMessageBox::critical(this, tr("ERROR!"), TicketsList->lastError().databaseText());
            }
        }
    }
    TicketsList->select();
}

void saveform::save()
{
    int row = ui->tableTicketsList->selectionModel()->currentIndex().row();
    int ticketsListID = (ui->tableTicketsList->model()->data(ui->tableTicketsList->model()->index(row, 0))).toInt();
    qDebug() << ticketsListID;

    QString filepath = QFileDialog::getSaveFileName(this,
        QString::fromUtf8("Сохранить файл"), QDir::currentPath(), "File (*.odt);;All files (*.*)");

    if(filepath.right(4) != ".odt")
    {
        QMessageBox::critical(this, "Сохранение выходного документа", "Имя файла не должно быть пустым");
        return;
    }

    QMap<QString, QString> *arg = new QMap<QString, QString>;
//    arg["HEADER"] = "header";
//    arg["UNIVERSITY_1"] = "Образовательгое учреждение";
//    arg["SPECIALTY"] = "Направление";
    arg->insert("UNIVERSITY_2", "ТОГУ");

    int result = GENLIB->createOutputFile(ticketsListID, filepath);

            //GENLIB->createOutputFile(ticketsListID, filepath);
            //createOutputFile(ticketsListID, filepath);
    switch (result) {
    case 0:
        QMessageBox::information(this, "Сохранение выходного документа", "Билеты успешно сохранены в файл по адресу " + filepath);
        break;
    case 1:
        QMessageBox::critical(this, "Сохранение выходного документа", "Невохможно создать файл метаданных manifest.xml!");
        break;
    case 2:
        QMessageBox::critical(this, "Сохранение выходного документа", "Невохможно создать файл содержимого content.xml");
        break;
    case -1:
        QMessageBox::critical(this, "Сохранение выходного документа", "Для указанного списка билетов билеты не сгенерированы!");
        break;
    default:
        break;
    }
}

void saveform::on_tableTicketsList_clicked(const QModelIndex &index)
{
    int row = index.row();
    int ticketsListID = (ui->tableTicketsList->model()->data(ui->tableTicketsList->model()->index(row, 0))).toInt();
    QSqlQuery h1(db);

    h1.prepare("select ticketsNum, questNumInTicket from TicketsList where ticketsListID = ?");
    h1.addBindValue(ticketsListID);
    h1.exec();
    h1.first();

    int ticketsNum = h1.value(0).toInt();           // количество билетов
    int questNumInTicket = h1.value(1).toInt();

    QVector<double> X, Ylevel, Ytopic;
    QVector<double> averageX, averageY;
    QVector<double> Xticks, Yticks;
    QVector<QString> Xlabels, Ylabels;
    int maxLevel = 0;
    double averageLevel = 0;

    for(int i = 0; i < ticketsNum; i++)
    {
        Xticks.append(i + 1);
        Xlabels.append(QString::number(i + 1));
        X.append(i + 1);
        // запрос на получение данных о билетах
        QSqlQuery q(db);
        q.prepare("SELECT QuestOfTickets.questListID, Tickets.questNumInList FROM Tickets NATURAL JOIN QuestOfTickets WHERE Tickets.ticketsListID = ? AND Tickets.ticketNum = ?");
        q.addBindValue(ticketsListID);
        q.addBindValue(i + 1);
        q.exec();

        int level = 0;
        QStringList lst;

        while(q.next())
        {
            int questListID = q.value(0).toInt();
            int questNumInList = q.value(1).toInt();

            QSqlQuery q(db);
            q.prepare("select questLevel, questTopic from Questions where questListID = ? and questNumInList = ?");
            q.addBindValue(questListID);
            q.addBindValue(questNumInList);
            q.exec();
            q.first();

            level += q.value(0).toInt();
            QString topic = q.value(1).toString();
            if(!lst.contains(topic))
                lst.append(topic);
        }
        averageLevel += level;
        if(level > maxLevel)
            maxLevel = level;

        Ylevel.append(level);
        Ytopic.append(questNumInTicket - lst.size());
    }

    for(int i = 0; i < maxLevel + 3; i++)
    {
        Yticks.append(i);
        Ylabels.append(QString::number(i));
    }
    Ylabels.pop_back();
    Ylabels.append("Сложность");
    averageLevel = (double) averageLevel /  ticketsNum;

    averageX.append(1);
    averageX.append(ticketsNum);
    averageY.append(averageLevel);
    averageY.append(averageLevel);

    QPen redpen;
    redpen.setWidth(2);
    redpen.setColor(Qt::red);

    QPen bluepen;
    bluepen.setWidth(2);
    bluepen.setColor(Qt::blue);

    QPen apen;
    apen.setWidth(1);
    apen.setColor(Qt::black);
    apen.setStyle(Qt::DashLine);

    ui->widget->clearPlottables();

    QCPCurve *curve = new QCPCurve(ui->widget->xAxis, ui->widget->yAxis);
    curve->setPen(redpen);
    curve->setData(X, Ylevel);
    curve->setLineStyle(QCPCurve::lsLine);
    curve->setName("Сложность");
    curve->setScatterStyle(QCPScatterStyle::ssDiamond);
    ui->widget->addPlottable(curve);

    QCPCurve *curve2 = new QCPCurve(ui->widget->xAxis, ui->widget->yAxis);
    curve2->setPen(apen);
    curve2->setData(averageX, averageY);
    curve2->setLineStyle(QCPCurve::lsLine);
    curve2->setName("Средняя\nсложность");
    ui->widget->addPlottable(curve2);

    QCPCurve *curve3 = new QCPCurve(ui->widget->xAxis, ui->widget->yAxis);
    curve3->setPen(bluepen);
    curve3->setData(X, Ytopic);
    curve3->setLineStyle(QCPCurve::lsLine);
    curve3->setName("Число\nпотовряющихся\nвопросов");
    curve3->setScatterStyle(QCPScatterStyle::ssDiamond);
    ui->widget->addPlottable(curve3);

    ui->widget->rescaleAxes();
    ui->widget->xAxis->setAutoTicks(false);
    ui->widget->xAxis->setAutoTickLabels(false);
    ui->widget->xAxis->setTickVector(Xticks);
    ui->widget->xAxis->setTickVectorLabels(Xlabels);
    ui->widget->xAxis->grid()->setVisible(true);

    ui->widget->yAxis->setAutoTicks(false);
    ui->widget->yAxis->setAutoTickLabels(false);
    ui->widget->yAxis->setTickVector(Yticks);
    ui->widget->yAxis->setTickVectorLabels(Ylabels);
    ui->widget->yAxis->grid()->setVisible(true);

    ui->widget->xAxis->setLabel("Номер билета");
    ui->widget->xAxis->setRange(0, ticketsNum + 5);
    ui->widget->yAxis->setRange(0, maxLevel + 3);

    ui->widget->setInteractions(QCP::iRangeDrag | QCP::iSelectPlottables| QCP::iRangeZoom);
    ui->widget->axisRect()->setupFullAxesBox();
    ui->widget->axisRect()->setRangeZoom(Qt::Horizontal);
    ui->widget->axisRect()->setRangeDrag(Qt::Horizontal);

    QLinearGradient gradient(0, 0, 0, 400);
    gradient.setColorAt(0, QColor(240, 240, 240));
    gradient.setColorAt(1, QColor(255, 255, 255));
    ui->widget->setBackground(QBrush(gradient));

    ui->widget->legend->setVisible(true);
    ui->widget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);
    ui->widget->legend->setBrush(QColor(255, 255, 255, 100));
    QPen legendPen;
    legendPen.setColor(QColor(130, 130, 130, 200));
    ui->widget->legend->setBorderPen(legendPen);
    QFont legendFont = font();
    legendFont.setPointSize(8);
    ui->widget->legend->setFont(legendFont);

    ui->widget->replot();
}
