#include "dropdialog.h"
#include "ui_dropdialog.h"

dropDialog::dropDialog(int id, QSqlDatabase db, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::dropDialog)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    resize(500, 150);
    setMinimumSize(500, 150);
    ui->loadFileBtn->setEnabled(false);
    idList = id;
    defaultColor = palette().color(backgroundRole());
    database = db;

    GENLIB = new gentickets();
    GENLIB->setConnection(database);
}

dropDialog::~dropDialog()
{
    delete ui;
}

void dropDialog::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();

        QPalette Pal(palette());
        QPixmap bkgnd(":/images/background/bkgnd.jpg");
        bkgnd = bkgnd.scaled(this->size(), Qt::IgnoreAspectRatio);
        Pal.setBrush(QPalette::Background, bkgnd);
        setPalette(Pal);
    }
}

void dropDialog::dragLeaveEvent(QDragLeaveEvent *event)
{
    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, defaultColor);
    setAutoFillBackground(true);
    setPalette(Pal);
}

void dropDialog::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString fileName = url.toLocalFile();

        if(fileName.right(4) != ".odt")
        {
            QMessageBox::critical(this, "Внимание!", "Необходимо выбрать файл формата odt!");
            QPalette Pal(palette());
            Pal.setColor(QPalette::Background, defaultColor);
            setAutoFillBackground(true);
            setPalette(Pal);
            return;
        }
        ui->lineEdit->setText(fileName);
        ui->loadFileBtn->setEnabled(true);
        qDebug() << "Dropped file:" << fileName;
    }
    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, defaultColor);
    setAutoFillBackground(true);
    setPalette(Pal);

    setFocus();
}

void dropDialog::on_openFileBtn_clicked()
{
    // имя файла
    QString filename = QFileDialog::getOpenFileName(this, QString::fromUtf8("Открыть файл"),
                                                        QDir::currentPath(), "Text (*.odt);;");
    if(filename.isEmpty())
    {
        return;
    }
    // если формат не odt
    else if(filename.right(4) != ".odt")
    {
        QMessageBox::critical(this, "Внимание!", "Необходимо выбрать файл формата odt");
        return;
    }
    else
    {
        ui->lineEdit->setText(filename);
        ui->loadFileBtn->setEnabled(true);
    }
}

void dropDialog::on_loadFileBtn_clicked()
{
    QFile file(ui->lineEdit->text());
    QString filename = ui->lineEdit->text();
    if(!file.exists())
    {
        QMessageBox::critical(this, "Внимание!", "Файл не существует!");
        return;
    }
    else if(filename.right(4) != ".odt")
    {
        QMessageBox::critical(this, "Внимание!", "Необходимо выбрать файл формата odt");
        return;
    }
    else
    {
        int result =  GENLIB->loadQuestionsToList(idList, ui->lineEdit->text());

        switch (result) {
        case 0:
            QMessageBox::information(this, "Загрузка вопросов в список вопросов", "Вопросы успешно загружены");
            break;
        case 1:
            QMessageBox::information(this, "Загрузка вопросов в список вопросов", "Указанный файл не существует");
            break;
        case 2:
            QMessageBox::information(this, "Загрузка вопросов в список вопросов", "Невозможно открыть файл manifest.xml");
            break;
        case 3:
            QMessageBox::information(this, "Загрузка вопросов в список вопросов", "Невозможно открыть файл content.xml");
            break;
        case 4:
            QMessageBox::information(this, "Загрузка вопросов в список вопросов", "Невозможно создать архив");
            break;
        default:
            break;
        }
        close();
    }
}

void dropDialog::on_cancelBtn_clicked()
{
    close();
}

void dropDialog::on_lineEdit_textEdited(const QString &arg1)
{
    if(ui->lineEdit->text().isEmpty())
        ui->loadFileBtn->setEnabled(false);
    else
        ui->loadFileBtn->setEnabled(true);
}

void dropDialog::on_editFileBtn_clicked()
{
    QString filepath = ui->lineEdit->text();
    qDebug() << filepath;
    QFile file(filepath);
    if(!file.exists())
    {
        QMessageBox::critical(this, "Ошибка", "Невозможно открыть файл!");
        return;
    }
    else
    {
        proc = new QProcess();
        proc->start("swriter -o '" + filepath + "'");
        if(!proc->waitForStarted())
        {
            qDebug() << "Процесс не запущен!";
            return;
        }
        if(!proc->waitForFinished())
        {
            qDebug() << "Процесс не может завершиться!";
            return;
        }
    }
}
