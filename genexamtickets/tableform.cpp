#include "tableform.h"
#include "ui_tableform.h"

TableForm::TableForm(table *t, QSqlRelationalTableModel *questionTable, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TableForm)
{
    ui->setupUi(this);
    resize(700, 400);
    setWindowTitle(t->actionName);
    ui->label->setVisible(false);
    ui->comboDiscipline->setVisible(false);
    this->t = t;
    ui->tableView->setModel(t->model);
    t->model->select();

    questTable = questionTable;

//    //connect(t->model, &QSqlRelationalTableModel::dataChanged, this, &TableForm::accept);
//    //connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TableForm::accept);
//    //connect(ui->tableView->itemDelegate(), &QAbstractItemDelegate::commitData, this, &TableForm::accept);

    switch (t->number) {
    case 0:
        ui->tableView->setColumnHidden(0, true);
        t->model->setHeaderData(1, Qt::Horizontal,tr("Название дисциплины"));
        ui->label_4->setVisible(false);
        ui->fio->setVisible(false);
        break;
    case 1:
        ui->tableView->setColumnHidden(0, true);
        t->model->setHeaderData(1, Qt::Horizontal,tr("Название кафедры"));
        t->model->setHeaderData(2, Qt::Horizontal,tr("ФИО зав. кафедрой"));
        ui->tableView->setColumnWidth(2, 300);
        break;
    default:
        break;
    }
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    connect(t->model, &QSqlRelationalTableModel::dataChanged, this, &TableForm::accept);
}

TableForm::~TableForm()
{
    delete ui;
}

void TableForm::on_addButton_clicked()
{
//    int row = t->model->rowCount();
//    t->model->insertRow(row);

    if(ui->name->text() == "")
        QMessageBox::warning(this, tr("Ошибка"),tr("Поле названия пустое.\n"
                                          "Введите пожалуйста название"), QMessageBox::Ok);
    else if(ui->fio->text() == "" && t->number == 1)
        QMessageBox::warning(this, tr("Ошибка"),tr("Поле ФИО пустое.\n"
                                          "Введите пожалуйста ФИО"), QMessageBox::Ok);
    else
    {
        QSqlQuery q;
        if(t->number == 0)
        {
            try
            {
                q.exec("insert into " + t->tableName + "(" + t->filter + ") values('" + ui->name->text() + "')");
                t->model->select();
            }
            catch(QSqlError error)
            {
                QMessageBox::critical(this, tr("ERROR!"), t->model->lastError().databaseText());
            }

        }
        if(t->number == 1)
            q.exec("insert into " + t->tableName + "(" + t->filter + ", zavDep) values('" + ui->name->text() + "', '" + ui->fio->text() +"')");


        t->model->select();
        ui->name->clear();
        ui->fio->clear();
    }
}

void TableForm::on_deleteButton_clicked()
{
    QModelIndex ind;
    QString str_query, str_query2;
    QSqlQueryModel *find = new QSqlQueryModel;
    QSqlQueryModel *find2 = new QSqlQueryModel;
    QComboBox *temp = new QComboBox;   
    QComboBox *temp2 = new QComboBox;

    if(ui->tableView->selectionModel()->currentIndex().isValid())
    {
        int row = ui->tableView->selectionModel()->currentIndex().row();
        QString data = (ui->tableView->model()->data(ui->tableView->model()->index(row, 0))).toString();
        //cout << "data - "<< data.toStdString() << endl;
        if(t->tableName == "Disc")
        {
            str_query = "SELECT discID FROM QuestionList WHERE discID = " + data;
            str_query2 = "SELECT discID FROM TicketsList WHERE discID = " + data;
        }
        else if(t->tableName == "Dep")
        {
            str_query = "SELECT depID FROM QuestionList WHERE depID=" + data;
            str_query2 = "SELECT depID FROM TicketsList WHERE depID=" + data;
        }
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
                t->model->removeRow(row, ind);
                if(!t->model->submitAll()) {
                    QMessageBox::critical(this, tr("ERROR!"), t->model->lastError().databaseText());
                }
            }
        }
        else {
            t->model->removeRow(row, ind);
            if(!t->model->submitAll()){
                QMessageBox::critical(this, tr("ERROR!"), t->model->lastError().databaseText());
            }
        }
    }
    t->model->select();

//    int row = ui->tableView->selectionModel()->currentIndex().row();
//    QString data = (ui->tableView->model()->data(ui->tableView->model()->index(row, 0))).toString();
//    cout << "data - "<< data.toStdString() << endl;
//    QSqlQuery q;
//    q.exec("delete from " + t->tableName + " where " + t->id + " = " + data + "");
//    t->model->select();

//    QSqlQuery q;
//    q.exec("delete from Disc where discID = 2");
//    t->model->select();
}

void TableForm::on_searchLine_textChanged(const QString &arg1)
{
//    if (arg1.isEmpty() && t->tableName == "Laboratory" && ui->comboDiscipline->currentIndex()!= -1)
//    {
//        int Cur = t->sub->record(ui->comboDiscipline->currentIndex()).field("idDiscip").value().toInt();
//        t->model->setFilter("idDiscip=" + QString::number(Cur));
//    }
//    else if(arg1.isEmpty())
//        t->model->setFilter("");
//    else if(t->tableName == "Laboratory" && ui->comboDiscipline->currentIndex()!= -1)
//    {
//        int Cur = t->sub->record(ui->comboDiscipline->currentIndex()).field("idDiscip").value().toInt();
//        t->model->setFilter("UPPER(" + t->filter + ") like upper('%" + arg1 + "%') and idDiscip=" + QString::number(Cur));
//    }
//    else
//        t->model->setFilter("UPPER(" + t->filter + ") like upper('%" + arg1 + "%')");
//    t->model->select();

    if(arg1.isEmpty())
        t->model->setFilter("");
    else
        t->model->setFilter("UPPER(" + t->filter + ") like upper('%" + arg1 + "%')");
    t->model->select();
}

void TableForm::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            cout<<"1"<<endl;
            t->model->select();
            //ui->tableView->update();
            return;
        }
    if (event->key() == Qt::Key_Escape)
    {
        if(ui->searchLine->text() == "")
            return;
        ui->searchLine->clear();
    }
}

void TableForm::on_comboDiscipline_currentIndexChanged(int index)
{
    int idDiscip = t->sub->record(index).field("idDiscip").value().toInt();
    t->model->setFilter("idDiscip=" + QString::number(idDiscip));
}

void TableForm::closeEvent(QCloseEvent *event)
{
    emit closed();
}

void TableForm::accept()
{
    questTable->select();
//    t->model->submitAll();
//    cout<<"2"<<endl;
//    t->model->select();
}
