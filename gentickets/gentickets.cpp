#include "gentickets.h"

gentickets::gentickets()
{
    dbExists = false;
}

int gentickets::setConnection(QSqlDatabase openedDB)
{
    db = openedDB;
    dbExists = true;

    QStringList tableName;
    tableName << "Dep" << "Disc" <<"QuestionList" << "Questions" << "TicketsList" << "QuestOfTickets" << "Tickets";

    int result = 0;
    for (int i = 0; i < tableName.count(); i++)
    {
        if(!tableExist(db, tableName[i]))
            result++;
    }

    return result;
}

bool gentickets::tableExist(QSqlDatabase db, QString tableName)
{
    if(tableName == nullptr || !db.isOpen())
    {
        return false;
    }

    QSqlQuery q(db);
    q.prepare("SELECT name FROM sqlite_master where type='table' and name = ?");
    q.addBindValue(tableName);
    q.exec();
    if(q.next())
    {
        return true;
    }
    else {
        return false;
    }
}

int gentickets::loadQuestionsToList(int questListID, QString filepath)
{
    // ---------- ПРОВЕРКА НА НАЛИЧИЕ СПИСКА ВОПРОСОВ --------
    // получение вспомогательных данных
    QSqlQuery h1(db);
    h1.prepare("select questListName from QuestionList where questListID = ?");
    h1.addBindValue(questListID);
    h1.exec();

    if(!h1.next())
        return -1;

    // ---------- ПОДГОТОВКА ПАПОК И ФАЙЛОВ ----------

    // создание открытие/файла .odt и папки для архива
    QString fileName = "input.odt";
    QString fileNameZip = "input.zip";
    QString filePath = "input/";
    QString zipPath = filePath + "zip/";
    QDir().mkdir("input");
    QFile::copy(filepath, QDir::currentPath() + "/input/input.odt");
    QDir().mkdir(zipPath);
    QFile inputFile(filePath + fileName);
    if(!inputFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        return 1;
    }
    inputFile.close();

    // разархивация архива в папку zip
    QFile::copy(filePath + fileName, zipPath + fileName);
    QFile::rename(zipPath + fileName, zipPath + fileNameZip);
    JlCompress::extractDir(zipPath + fileNameZip, zipPath);

    // ---------- ПАРСЕР ----------

    // обработка метаданных - файл manifest.xml
    QFile * manifestFile = new QFile(zipPath + "META-INF/manifest.xml");
    if (!manifestFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        manifestFile->close();
        QDir dir(filePath);
        dir.removeRecursively();
        dir.remove(filePath);
        return 2;
    }

    QDomDocument manifestDomDoc;
    manifestDomDoc.setContent(manifestFile);
    QDomElement manifestRootElement = manifestDomDoc.documentElement();
    QDomNodeList manifestNodeList = manifestRootElement.elementsByTagName("manifest:file-entry");

    QString aManifest;
    for (int i = 0; i < manifestNodeList.length(); i++)
    {
        QDomNode a = manifestNodeList.at(i);
        QTextStream ts(&aManifest);
        a.save(ts, 0);
    }
    manifestFile->close();

    QStringList manifestList = aManifest.split("\n");

    // обработка содержания - файл content.xml
    QFile * contentfile = new QFile(zipPath + "content.xml");
    if (!contentfile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        contentfile->close();
        QDir dir(filePath);
        dir.removeRecursively();
        dir.remove(filePath);
        return 3;
    }

    QList<int>              LEVEL_LIST;
    QStringList             BODY_LIST;
    QStringList             TOPIC_LIST;
    QMap<int, QString>      manifestMap;
    QMap<int, QStringList>  hrefListMap;
    QMap<QString, QString>  objectName; // oldObjectName - newObjectName для переименования в content

    // подсчет cell
    int i = 0;
    int quest = 0;
    int object = 1;
    // вектор поиска ячеек таблицы true - ячейка 1 уровня, false - иного уровня
    QVector<bool> vec;
    // lvl - переход с 1 уровня ячеек
    bool lvl = false;
    // глубина ячеек
    int depth = 0;
    QXmlStreamReader reader(contentfile);

    while (!reader.atEnd() && !reader.hasError())
    {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (reader.name() != "table-cell")
            continue;
        if (reader.name() == "table-cell")
        {
            if(token == QXmlStreamReader::StartElement)
                depth++;
            else if(token == QXmlStreamReader::EndElement)
                depth--;
            if(depth == 2)
                lvl = true;
            else if(depth == 0)
                lvl = false;
            if(depth == 1 && !lvl)
            {
                i++;
                vec.append(true);
                if(i%3 == 1)
                {
                    reader.readNext();
                    reader.readNext();
                    if(reader.text().toInt() == 0)
                        LEVEL_LIST.append(3);
                    else
                        LEVEL_LIST.append(reader.text().toInt());
                    qDebug() << "level " << LEVEL_LIST.last();
                }
                else if(i%3 == 2)
                {
                    reader.readNext();
                    reader.readNext();
                    if(reader.text().toString() == "")
                        TOPIC_LIST.append("Без темы");
                    else
                        TOPIC_LIST.append(reader.text().toString());
                    qDebug() << "topic " << TOPIC_LIST.last();
                }
                else if (i%3 == 0)
                {
                    QString manifestString = "";
                    QStringList hrefList;
                    while (reader.readNext() && lvl == false)
                    {
                        // обработка ссылок на объекты
                        if(reader.attributes().hasAttribute("xlink:href"))
                        {
                            // старое имя объекта
                            QString oldObjectName = reader.attributes().value("xlink:href").toString();
                            // если такой объект уже встречался
                            if(objectName.contains(oldObjectName))
                            {
                                // если встречался в этом же вопросе
                                if(hrefList.contains(oldObjectName))
                                    continue;
                                //если встречался в другом вопросе
                                else
                                {
                                    hrefList.append(objectName.value(oldObjectName));
                                    continue;
                                }
                            }
                            // если объект картинка
                            if(oldObjectName.left(8) == "Pictures")
                            {
                                hrefList.append(oldObjectName);

                                for(QString s : manifestList)
                                    if(s.contains(oldObjectName))
                                        manifestString.append(s + "\n");
                            }
                            // если формула
                            else if(oldObjectName.left(20) == "./ObjectReplacements")
                            {
                                // переименовываем
                                QString newObjectReplacementName = QString("%1 %2_%3").arg("./ObjectReplacements/Object").arg(questListID).arg(object);
                                hrefList.append(newObjectReplacementName);

                                QString oldPath = zipPath + oldObjectName.mid(2);
                                QString newPath = zipPath + newObjectReplacementName.mid(2);
                                QFile::rename(oldPath, newPath);

                                objectName[oldObjectName] = newObjectReplacementName;
                                qDebug() << newObjectReplacementName;

                                object++;
                            }
                            else if(oldObjectName.left(8) == "./Object")
                            {
                                // переименовываем
                                QString newObjectName = QString("%1 %2_%3").arg("./Object").arg(questListID).arg(object);
                                QString oldObj = oldObjectName.mid(2);
                                QString newObj = newObjectName.mid(2);
                                hrefList.append(newObjectName);

                                QString oldPath = zipPath + oldObj;
                                QString newPath = zipPath + newObj;
                                QFile::rename( oldPath, newPath);

                                objectName[oldObjectName] = newObjectName;
                                qDebug() << newObjectName;

                                for(QString s : manifestList)
                                {
                                    if(s.contains(oldObj))
                                    {
                                        QString str = s.replace(oldObj, newObj);
                                        manifestString.append(str + "\n");
                                    }
                                }
                            }
                        }
                        // внутренние таблицы
                        if (reader.name() == "table-cell")
                        {
                            if(reader.tokenType() == QXmlStreamReader::StartElement)
                                depth++;
                            else if(reader.tokenType() == QXmlStreamReader::EndElement)
                                depth--;
                            if(depth == 1)
                                vec.append(true);
                            if(depth == 2)
                            {
                                lvl = true;
                                vec.append(false);
                            }
                            else if(depth == 0)
                            {
                                lvl = false;
                                break;
                            }
                        }
                    }
                    hrefListMap[quest] = hrefList;
                    manifestMap[quest] = manifestString;
                    // новый вопрос
                    quest++;
                }
            }
            else if(depth == 2)
                vec.append(false);
        }
    }
    contentfile->seek(0);

    QDomDocument domDoc;
    domDoc.setContent(contentfile);
    QDomElement rootElement = domDoc.documentElement();
    QDomNodeList alist = rootElement.elementsByTagName("table:table-cell");
    QStringList list;

    // обход всех тегов table-cell и изъятие содержимого body
    for (int i = 0; i < alist.length(); i++)
    {
        QDomNode a = alist.at(i);
        QString aContent;
        QTextStream ts(&aContent);
        QDomNodeList child = a.childNodes();
        for(int j = 0; j < child.size(); j++)
            child.at(j).save(ts, 0);
        list.append(aContent);
    }

    // выделение нужных строк
    for (int i = 0; i < list.length(); i++)
    {
        qDebug() << vec[i];
        if(vec[i])
            BODY_LIST.append(list[i+=2]);
    }

    // переименовываем имена объектов в поле content вопросов
    for (int i = 0; i < BODY_LIST.length(); i++)
    {
        QString body = BODY_LIST[i];
        QMap<QString, QString>::iterator iter;
        for(iter = objectName.begin(); iter != objectName.end(); iter++)
        {
            QString old = iter.key();
            if(body.contains(old))
            {
                if(body.contains(old + "_"))
                    continue;
                else
                    body.replace(old, iter.value());
            }
        }
        BODY_LIST[i] = body;
        qDebug() << body << "\n";
    }
    contentfile->seek(0);

    QList<Quest> questList;

    for(int i = 0; i < quest; i++)
    {
        Quest a;
        a.level = LEVEL_LIST[i];
        a.topic = TOPIC_LIST[i];
        a.body = BODY_LIST[i];
        a.hrefList = hrefListMap[i];
        a.manifest = manifestMap.value(i);
        qDebug() << a.manifest;
        questList.append(a);
    }
    contentfile->close();

    // загрузка в БД

    for(int i = 0; i < quest; i++)
    {
        QSqlQuery query(db);
        query.prepare("INSERT INTO Questions "
                      "(questNumInList, questListID, questBody, questLevel, questTopic, questManifest, questContent) "
                      "values (:number , :listID, :bodyData, :level, :topic, :manifest, :content)");

        QDir().mkdir(zipPath + "temp");
        for(QString s : questList[i].hrefList)
        {
            QString oldPath = zipPath + s;
            QString newPath = zipPath + "temp/" + s;
            if(s.contains("Pictures"))
            {
                QDir().mkdir(zipPath + "temp/Pictures");
                oldPath = zipPath + s;
                newPath = zipPath + "temp/" + s;
            }
            else
            {
                if(s.contains("Replacements"))
                    QDir().mkdir(zipPath + "temp/ObjectReplacements");
                oldPath = zipPath + s.mid(2);
                newPath = zipPath + "temp/" + s.mid(2);
            }
            QFile::rename(oldPath, newPath);
        }

        JlCompress::compressDir(zipPath + "/temp.zip", zipPath + "temp");

        QFile file(zipPath + "temp.zip");
        if(!file.open(QIODevice::ReadOnly))
            return 4;
        QByteArray bodyByteArray = file.readAll();

        query.bindValue(":number",      i + 1);
        query.bindValue(":listID",      questListID);
        query.bindValue(":bodyData",    bodyByteArray);
        query.bindValue(":level",       questList[i].level);
        query.bindValue(":topic",       questList[i].topic);
        query.bindValue(":manifest",    questList[i].manifest);
        query.bindValue(":content",     questList[i].body);
        query.exec();

        file.remove();
        QDir tempDir(zipPath + "temp");
        tempDir.removeRecursively();
    }

    QDir dir(filePath);
    dir.removeRecursively();
    dir.remove(filePath);
    return 0;
}

int gentickets::removeQuestionsFromList(int questListID)
{
    QSqlQuery * questions = new QSqlQuery(db);
    questions->exec("delete from questions where questlistid = " + QString::number(questListID));
    // проверка ошибок
    if (questions->lastError().isValid()){
        return 1;
    }
    else
        return 0;
}

int gentickets::generateTickets(int ticketsListID, int lvlrate, double topicrate)
{
    // ---------- ПРОВЕРКА НА НАЛИЧИЕ СПИСКА БИЛЕТОВ --------
    // получение вспомогательных данных
    QSqlQuery h0(db);
    h0.prepare("select ticketsListName from TicketsList where ticketsListID = ?");
    h0.addBindValue(ticketsListID);
    h0.exec();

    if(!h0.next())
        return -1;

    srand(time(0));
    QList<int> idList;
    QMap<int, QList<int> > map;

    QSqlQuery h1(db);
    h1.prepare("select ticketsNum, questNumInTicket from TicketsList where ticketsListID = ?");
    h1.addBindValue(ticketsListID);
    h1.exec();
    h1.first();

    int ticketsNum = h1.value(0).toInt();        // количество билетов
    int questNumInTicket = h1.value(1).toInt();        // количество билетов

    QSqlQuery h2(db);
    h2.prepare("select count(*) from QuestOfTickets where ticketsListID = ?");
    h2.addBindValue(ticketsListID);
    h2.exec();
    h2.first();
    int questNumInTicketDeclared = h2.value(0).toInt();

    if(questNumInTicket != questNumInTicketDeclared)
        return 1;


    QSqlQuery q2(db);
    q2.prepare("select questNumInTicket, questListID from QuestOfTickets where ticketsListID = ?");
    q2.addBindValue(ticketsListID);
    q2.exec();

    while(q2.next())
        idList.append(q2.value(1).toInt());

    int number = 0;
    // заполнение мэпа
    // id списка вопросов - список id вопросов
    for(int i : idList)
    {
        QSqlQuery q2(db);
        q2.prepare("select questNumInList from Questions where questListID = ?");
        q2.addBindValue(i);
        q2.exec();

        QList<int> lst;
        while(q2.next())
            lst.append(q2.value(0).toInt());
        map[number++] = lst;
    }

    // выранивание списков id вопросов по длине указаанной в ticketslist
    QMap<int, QList<int> >::iterator k;
    for(k = map.begin(); k  != map.end(); k++)
    {
        QList<int> j = k.value();
        int diff = ticketsNum - j.length();
        // если вопросов больше - обрезаем до нужного значения
        if(diff < 0)
        {
            while(diff != 0)
            {
                j.pop_back();
                diff++;
            }
        }
        // если вопросов меньше - дополняем до нужного значения
        else
        {
            int id = 0;
            while(diff != 0)
            {
                j.push_back(j.value(id));
                id++;
                diff--;
            }
        }
        // перемешиваем списки с id вопросов
        ShuffleList(j);
        map[k.key()] = j;
    }

    int ITER_COUNT = 250;

    // если выравнивание по сложности и по теме
    if(lvlrate != -1 && topicrate != -1)
    {
        for(int i = 0; i < ITER_COUNT; i++)      //
        {
            // проверка на удовлетворение выравниванию
            bool leveling = checkLeveling(map, idList, ticketsNum, lvlrate);
            bool topicAlign = checkTopicAlign(map, idList, ticketsNum, topicrate);
            // если удовлитворительно заполняем базу
            if(leveling && topicAlign)
            {
                addTicketsToDatabase(map, ticketsListID);
                return 0;
            }
            // иначе перемешиваем список и пробуем снова
            else
            {
                QMap<int, QList<int> >::iterator k;
                for(k = map.begin(); k  != map.end(); k++)
                {
                    int idList = k.key();
                    QList<int> idsQuest = k.value();
                    ShuffleList(idsQuest);
                    map[idList] = idsQuest;
                }
            }
        }
        return 2;
    }
    // если выравнивание по сложности
    else if(lvlrate != -1)
    {
        for(int i = 0; i < ITER_COUNT; i++)      //
        {
            bool leveling = checkLeveling(map, idList, ticketsNum, lvlrate);
            if(leveling == true)
            {
                addTicketsToDatabase(map, ticketsListID);
                return 0;
            }
            else
            {
                QMap<int, QList<int> >::iterator k;
                for(k = map.begin(); k  != map.end(); k++)
                {
                    int idList = k.key();
                    QList<int> idsQuest = k.value();
                    ShuffleList(idsQuest);
                    map[idList] = idsQuest;
                }
            }
        }
        return 3;
    }
    // если выравнивание по теме
    else if(topicrate != -1)
    {
        for(int i = 0; i < ITER_COUNT; i++)
        {
            bool topicAlign = checkTopicAlign(map, idList, ticketsNum, topicrate);
            if(topicAlign == true)
            {
                addTicketsToDatabase(map, ticketsListID);
                return 0;
            }
            else
            {
                QMap<int, QList<int> >::iterator k;
                for(k = map.begin(); k  != map.end(); k++)
                {
                    int idList = k.key();
                    QList<int> idsQuest = k.value();
                    ShuffleList(idsQuest);
                    map[idList] = idsQuest;
                }
            }
        }
        return 4;
    }
    // если выранивание не выбрано
    else
        addTicketsToDatabase(map, ticketsListID);

    return 0;
}

void gentickets::ShuffleList(QList<int> &list)
{
    for(int i = list.size() - 1; i >= 1; i--)
    {
        int j = rand() % (i + 1);
        int temp = list[j];
        list[j] = list[i];
        list[i] = temp;
    }
    return;
}

bool gentickets::checkLeveling(QMap<int, QList<int> > map, QList<int> idsList, int ticketsNum, int levelrate)
{
    // список весов (сумм сложностей вопросов)
    QList<int> sum;
    for(int i = 0; i < ticketsNum; i++)
        sum.append(0);

    // получение сложностей билетов (сумма сложностей вопросов в каждом билете)
    int number = 0;
    QMap<int, QList<int> >::iterator k;
    for(k = map.begin(); k  != map.end(); k++)
    {
        int idList = idsList[number++];
        QList<int> idsQuest = k.value();
        for(int i = 0; i < idsQuest.length(); i++)
        {
            QSqlQuery q(db);
            q.prepare("select questLevel from Questions where questListID = ? and questNumInList = ?");
            q.addBindValue(idList);
            q.addBindValue(idsQuest[i]);
            q.exec();
            q.first();

            int level =  q.value(0).toInt();
            sum[i] += level;
        }
    }

    int max = maximumListItem(sum);
    int min = minimumListItem(sum);
    int diff = max - min;

    if(diff <= levelrate)
    {
        qDebug() << "diff " << diff;
        return true;
    }
    else
        return false;
}

bool gentickets::checkTopicAlign(QMap<int, QList<int> > map, QList<int> idsList, int ticketsNum, double topicrate)
{
    // список весов (сумма повторяющихся тем вопросов вопросов)
    QList<QStringList> sum;
    for(int i = 0; i < ticketsNum; i++)
    {
        QStringList a;
        sum.append(a);
    }

    // получение списка тем вопросов для билетов
    int number = 0;
    QMap<int, QList<int> >::iterator k;
    for(k = map.begin(); k  != map.end(); k++)
    {
        int idList = idsList[number++];
        QList<int> idsQuest = k.value();
        for(int i = 0; i < idsQuest.length(); i++)
        {
            QSqlQuery q(db);
            q.prepare("select questTopic from Questions where questListID = ? and questNumInList = ?");
            q.addBindValue(idList);
            q.addBindValue(idsQuest[i]);
            q.exec();
            q.first();

            QString topic =  q.value(0).toString();
            sum[i].append(topic);
        }
    }

    double fitness = 0;

    // количество уникальных тем вопросов ков сем вопросам
    for(QStringList i : sum)
        fitness += i.removeDuplicates();
    fitness = (double) fitness / (sum.size() * (sum[0].size() - 1));

    if(fitness <= topicrate)
    {
        qDebug() << "fitness " << fitness;
        return true;
    }
    else
        return false;
}

int gentickets::maximumListItem(QList<int> list)
{
    int max = list[0];
    for(int i = 1; i < list.length(); i++)
        if(list[i] > max)
            max = list[i];
    return max;
}

int gentickets::minimumListItem(QList<int> list)
{
    int min = list[0];
    for(int i = 1; i < list.length(); i++)
        if(list[i] < min)
            min = list[i];
    return min;
}

void gentickets::addTicketsToDatabase(QMap<int, QList<int> > map, int ticketsListID)
{
    // загрузка билетов вБД
    int q = 1;
    QMap<int, QList<int> >::iterator k;
    for(k = map.begin(); k  != map.end(); k++)
    {
        QList<int> idsQuest = k.value();
        for(int j = 0; j < idsQuest.length(); j++)
        {
            int idQuest = idsQuest[j];

            QSqlQuery query(db);
            query.prepare("INSERT INTO Tickets "
                          "(ticketsListID, ticketNum, questNumInTicket, questNumInList) "
                          "VALUES (:ticketsListID , :numberTickets, :numberQuest, :idQuest)");

            query.bindValue(":ticketsListID",       ticketsListID);
            query.bindValue(":numberTickets",       j + 1);
            query.bindValue(":numberQuest",         q);
            query.bindValue(":idQuest",             idQuest);
            query.exec();
        }
        q++;
    }
    return;
}

int gentickets::createOutputFile(int ticketsListID, QString filepath, QMap<QString, QString> *arguments)
{
    // ---------- ПРОВЕРКА НА НАЛИЧИЕ БИЛЕТОВ --------
    // получение вспомогательных данных
    QSqlQuery h1(db);
    h1.prepare("select ticketNum from Tickets where ticketsListID = ?");
    h1.addBindValue(ticketsListID);
    h1.exec();

    if(!h1.next())
        return -1;

    // ---------- ПОДГОТОВКА ПАПОК И ФАЙЛОВ ----------
    // создание открытие/файла .odt и папки для архива
    //QString fileName = "output.odt";
    //QString fileNameZip = "output.zip";
    QString filePath = "output/";
    QString zipPath = filePath + "zip/";
    QDir().mkdir("output");
    QDir().mkdir(zipPath);

    // создание файла метаданных manifest.xml
    int res1 = createManifestFile(ticketsListID);
    if(res1 != 0)
        return res1;

    // создание файла содержимого content.xml И изъятие объектов из БД
    int res2 = createContentFile(ticketsListID, arguments);
    if(res2 != 0)
        return res2;

    // архивирование файлов и получение файла формата ODF
    JlCompress::compressDir(zipPath + "tickets.zip", zipPath);
    QFile::rename(zipPath + "tickets.zip", filepath);

    // рекурсивное удаление временных папок
    QDir dir(filePath);
    dir.removeRecursively();
    dir.remove(filePath);
    return 0;
}

int gentickets::createManifestFile(int ticketsListID)
{
    // путь к метафайлу
    QString filePath = "output/zip/META-INF/";
    QDir().mkdir(filePath);

    // получение вспомогательных данных
    QSqlQuery h1(db);
    h1.prepare("select ticketsNum, questNumInTicket from TicketsList where ticketsListID = ?");
    h1.addBindValue(ticketsListID);
    h1.exec();
    h1.first();

    int ticketsNum = h1.value(0).toInt();           // количество билетов
    //int questNumInTicket = h1.value(1).toInt();        // количество вопросов в билете

    QString manifestStart = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                            "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\" xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\">"
                            "<manifest:file-entry manifest:version=\"1.2\" manifest:media-type=\"application/vnd.oasis.opendocument.text\" manifest:full-path=\"/\"/>"
                            "<manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"content.xml\"/>";
    QString manifestEnd = "</manifest:manifest>";
    QString manifestContent;

    for(int i = 0; i < ticketsNum; i++)
    {
        // запрос на получение данных о билетах
        QSqlQuery q(db);
        q.prepare("SELECT QuestOfTickets.questListID, Tickets.questNumInList FROM Tickets NATURAL JOIN QuestOfTickets WHERE Tickets.ticketsListID = ? AND Tickets.ticketNum = ?");
        q.addBindValue(ticketsListID);
        q.addBindValue(i);
        q.exec();

        while(q.next())
        {
            int questListID = q.value(0).toInt();
            int questNumInList = q.value(1).toInt();

            QSqlQuery q(db);
            q.prepare("select questManifest from Questions where questListID = ? and questNumInList = ?");
            q.addBindValue(questListID);
            q.addBindValue(questNumInList);
            q.exec();
            q.first();

            QString s = q.value(0).toString();
            if(manifestContent.contains(s))
                continue;
            else
                manifestContent.append(s);
        }
    }

    // создание файла
    QFile file1(filePath + "manifest.xml");
    if(file1.open(QIODevice::WriteOnly)) {
        QTextStream(&file1) << manifestStart + manifestContent + manifestEnd; //doc.toString();
        file1.close();
    }
    else
        return 1;
    return 0;
}

int gentickets::createContentFile(int ticketsListID, QMap<QString, QString> *arguments)
{
    srand(time(0));
    QString filePath = "output/zip/";
    QString zipName = filePath + "temp.zip";

    QSqlQuery h1(db);
    h1.prepare("select ticketsNum, questNumInTicket from TicketsList where ticketsListID = ?");
    h1.addBindValue(ticketsListID);
    h1.exec();
    h1.first();

    int ticketsNum = h1.value(0).toInt();           // количество билетов
    //int questNumInTicket = h1.value(1).toInt();        // количество вопросов в билете

    // получение id кафедры
    QSqlQuery h2(db);
    h2.prepare("select depID, discID from TicketsList where ticketsListID = ?");
    h2.addBindValue(ticketsListID);
    h2.exec();
    h2.first();

    int depID = h2.value(0).toInt();           // id кафедры
    int discID = h2.value(1).toInt();           // id кафедры

    // получение названия кафедры
    QSqlQuery h3(db);
    h3.prepare("select depName, zavDep from Dep where depID = ?");
    h3.addBindValue(depID);
    h3.exec();
    h3.first();

    QString depName = h3.value(0).toString();          // имя кафедры
    QString zavDep = h3.value(1).toString();          // имя завкафедры

    // получение названия кафедры
    QSqlQuery h4(db);
    h4.prepare("select discName from Disc where discID = ?");
    h4.addBindValue(discID);
    h4.exec();
    h4.first();

    QString discName = h4.value(0).toString();          // имя дисциплины

    // получение данных об университете и направлении
    QString university_1;
    QString university_2;
    QString header;
    QString specialty;

    if(arguments == nullptr)
    {
        header = "МИНИСТЕРСТВО НАУКИ И ВЫСШЕГО ОБРАЗОВАНИЯ РОССИЙСКОЙ ФЕДЕРАЦИИ";
        university_1 = "Федеральное государственное бюджетное образовательное учреждение высшего образования";
        university_2 = "Тихоокеанский Государственный Университет";
        specialty = "09.03.04 Программная инженерия";
    }
    else
    {
        if(arguments->contains("HEADER"))
            header = arguments->value("HEADER");
        else {
            header = "МИНИСТЕРСТВО НАУКИ И ВЫСШЕГО ОБРАЗОВАНИЯ РОССИЙСКОЙ ФЕДЕРАЦИИ";
        }

        if(arguments->contains("UNIVERSITY_1"))
            university_1 = arguments->value("UNIVERSITY_1");
        else {
            university_1 = "Федеральное государственное бюджетное образовательное учреждение высшего образования";
        }

        if(arguments->contains("UNIVERSITY_2"))
            university_2 = arguments->value("UNIVERSITY_2");
        else {
            university_2 = "Тихоокеанский Государственный Университет";
        }

        if(arguments->contains("SPECIALTY"))
            specialty = arguments->value("SPECIALTY");
        else {
            specialty = "09.03.04 Программная инженерия";
        }
    }

    QString contentStart = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<office:document-content office:version=\"1.2\" xmlns:css3t=\"http://www.w3.org/TR/css3-text/\" xmlns:formx=\"urn:openoffice:names:experimental:ooxml-odf-interop:xmlns:form:1.0\" xmlns:field=\"urn:openoffice:names:experimental:ooo-ms-interop:xmlns:field:1.0\" xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\" xmlns:calcext=\"urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0\" xmlns:drawooo=\"http://openoffice.org/2010/draw\" xmlns:tableooo=\"http://openoffice.org/2009/table\" xmlns:officeooo=\"http://openoffice.org/2009/office\" xmlns:grddl=\"http://www.w3.org/2003/g/data-view#\" xmlns:xhtml=\"http://www.w3.org/1999/xhtml\" xmlns:of=\"urn:oasis:names:tc:opendocument:xmlns:of:1.2\" xmlns:rpt=\"http://openoffice.org/2005/report\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xforms=\"http://www.w3.org/2002/xforms\" xmlns:dom=\"http://www.w3.org/2001/xml-events\" xmlns:oooc=\"http://openoffice.org/2004/calc\" xmlns:ooow=\"http://openoffice.org/2004/writer\" xmlns:ooo=\"http://openoffice.org/2004/office\" xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\" xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\" xmlns:math=\"http://www.w3.org/1998/Math/MathML\" xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\">"
    "<office:scripts/>"
    "<office:font-face-decls>"
        "<style:font-face svg:font-family=\"Mangal\" style:name=\"Mangal1\"/>"
        "<style:font-face svg:font-family=\"'Liberation Mono'\" style:name=\"Liberation Mono\" style:font-pitch=\"fixed\" style:font-family-generic=\"modern\"/>"
        "<style:font-face svg:font-family=\"NSimSun\" style:name=\"NSimSun\" style:font-pitch=\"fixed\" style:font-family-generic=\"modern\"/>"
        "<style:font-face svg:font-family=\"'Liberation Serif'\" style:name=\"Liberation Serif\" style:font-pitch=\"variable\" style:font-family-generic=\"roman\"/>"
        "<style:font-face svg:font-family=\"'Liberation Sans'\" style:name=\"Liberation Sans\" style:font-pitch=\"variable\" style:font-family-generic=\"swiss\"/>"
        "<style:font-face svg:font-family=\"Mangal\" style:name=\"Mangal\" style:font-pitch=\"variable\" style:font-family-generic=\"system\"/>"
        "<style:font-face svg:font-family=\"'Microsoft YaHei'\" style:name=\"Microsoft YaHei\" style:font-pitch=\"variable\" style:font-family-generic=\"system\"/>"
        "<style:font-face svg:font-family=\"SimSun\" style:name=\"SimSun\" style:font-pitch=\"variable\" style:font-family-generic=\"system\"/>"
        "<style:font-face svg:font-family=\"Tahoma\" style:name=\"Tahoma1\"/>"
        "<style:font-face svg:font-family=\"'Times New Roman'\" style:name=\"Times New Roman\" style:font-pitch=\"variable\" style:font-family-generic=\"roman\"/>"
        "<style:font-face svg:font-family=\"Arial\" style:name=\"Arial\" style:font-pitch=\"variable\" style:font-family-generic=\"swiss\"/>"
        "<style:font-face svg:font-family=\"'Andale Sans UI'\" style:name=\"Andale Sans UI\" style:font-pitch=\"variable\" style:font-family-generic=\"system\"/>"
        "<style:font-face svg:font-family=\"Tahoma\" style:name=\"Tahoma\" style:font-pitch=\"variable\" style:font-family-generic=\"system\"/>"
    "</office:font-face-decls>"
    "<office:automatic-styles>"

        "<style:style style:name=\"P10\" style:parent-style-name=\"Table_20_Contents\" style:family=\"paragraph\">"
        "<style:paragraph-properties style:justify-single-word=\"false\" fo:text-align=\"end\"/>"
        "</style:style>"

        "<style:style style:name=\"P20\" style:parent-style-name=\"Table_20_Contents\" style:family=\"paragraph\">"
        "<style:paragraph-properties style:justify-single-word=\"false\" fo:text-align=\"center\" fo:language=\"ru\" fo:country=\"RU\"/>"
        "</style:style>"

        "<style:style style:name=\"P30\" style:parent-style-name=\"Standard\" style:family=\"paragraph\">"
        "<style:paragraph-properties style:justify-single-word=\"false\" fo:text-align=\"center\"/>"
        "<style:text-properties style:text-underline-style=\"none\"/>"
        "</style:style>"

        "<style:style style:name=\"T10\" style:family=\"text\">"
        "<style:text-properties style:text-underline-style=\"none\"/>"
        "</style:style>"

        "<style:style style:name=\"T20\" style:family=\"text\">"
        "<style:text-properties fo:language=\"ru\" fo:country=\"RU\"/>"
        "</style:style>"

        "<text:list-style style:name=\"L20\">"
            "<text:list-level-style-number style:num-format=\"1\" style:num-suffix=\".\" text:style-name=\"Numbering_20_Symbols\" text:level=\"1\">"
            "<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
            "<style:list-level-label-alignment fo:margin-left=\"1.27cm\" fo:text-indent=\"-0.635cm\" text:list-tab-stop-position=\"1.27cm\" text:label-followed-by=\"listtab\"/>"
            "</style:list-level-properties>"
            "</text:list-level-style-number>"
        "</text:list-style>"

    "</office:automatic-styles>"
    "<office:body>"
    "<office:text>"
    "<text:sequence-decls>"
        "<text:sequence-decl text:name=\"Illustration\" text:display-outline-level=\"0\"/>"
        "<text:sequence-decl text:name=\"Table\" text:display-outline-level=\"0\"/>"
        "<text:sequence-decl text:name=\"Text\" text:display-outline-level=\"0\"/>"
        "<text:sequence-decl text:name=\"Drawing\" text:display-outline-level=\"0\"/>"
    "</text:sequence-decls>";
    QString contentEnd = "</office:text></office:body></office:document-content>";

    QString ticketsHeader = "<text:p text:style-name=\"P20\">"
    "%1"
    "<text:line-break/>"
    "%2"
    "<text:line-break/>"
    "«%3»"
    "</text:p>"
    "<text:p text:style-name=\"Table_20_Contents\"/>"
    "<text:p text:style-name=\"P20\">Направление: %4</text:p>"
    "<text:p text:style-name=\"Table_20_Contents\"/>"
    "<text:p text:style-name=\"P20\">Дисциплина: %5</text:p>"
    "<text:p text:style-name=\"Table_20_Contents\"/>";

    QString ticketsNumber = "<text:p text:style-name=\"P20\">Экзаменационный билет № %1</text:p>"
    "<text:p text:style-name=\"Table_20_Contents\"/>";

    QString ticketsFooter = "<text:p text:style-name=\"Table_20_Contents\"/>"
    "<text:p text:style-name=\"P10\">Зав. кафедрой %1 __________ %2</text:p>"
    "<text:p text:style-name=\"Table_20_Contents\"/>"
    "<text:p text:style-name=\"Table_20_Contents\"/>";

    QString listHeader = "<text:list text:style-name=\"L20\" xml:id=\"list%1\">";

    QString content;
    content.append(contentStart);
    for(int i = 0; i < ticketsNum; i++)
    {
        int currentListID = (i+1)*10000;
        content.append(ticketsHeader.arg(header, university_1, university_2, specialty, discName));
        content.append(ticketsNumber.arg(QString::number(i+1)));
        content.append(listHeader.arg(QString::number(currentListID)));

        // запрос на получение данных о билетах
        QSqlQuery q(db);
        q.prepare("SELECT QuestOfTickets.questListID, Tickets.questNumInList FROM Tickets NATURAL JOIN QuestOfTickets WHERE Tickets.ticketsListID = ? AND Tickets.ticketNum = ?");
        q.addBindValue(ticketsListID);
        q.addBindValue(i + 1);
        q.exec();

        while(q.next())
        {
            int questListID = q.value(0).toInt();
            int questNumInList = q.value(1).toInt();

            QSqlQuery q(db);
            q.prepare("select questBody, questContent from Questions where questListID = ? and questNumInList = ?");
            q.addBindValue(questListID);
            q.addBindValue(questNumInList);
            q.exec();
            q.first();

            // ---------- ИЗЪЯТИЕ ОБЪЕКТОВ ИЗ БД ----------
            QByteArray data = q.value(0).toByteArray();
            QFile zip(zipName);
            if(zip.open(QIODevice::WriteOnly))
            {
                zip.write(data);
                zip.close();
            }
            else {
                return 3;
            }
            JlCompress::extractDir(zipName, filePath);
            zip.remove();

            // ---------- СОЗДАНИЕ ФАЙЛА CONTENT.XML ----------
            QString s = q.value(1).toString();

            if(s.contains("table"))
            {
                int tablepos = s.indexOf("<table:table");
                QString text = s.left(tablepos);
                content.append("<text:list-item>");
                content.append(text);
                content.append("</text:list-item>");
                content.append("</text:list>");
                QString table = s.right(s.length() - tablepos);
                content.append(table);

                int prevListID = currentListID;
                currentListID +=  1 + rand() % 1000;

                QString newList = "<text:list text:style-name=\"L20\" xml:id=\"list%1\" text:continue-list=\"list%2\">";
                content.append(newList.arg(QString::number(currentListID), QString::number(prevListID)));
            }
            else
            {
                content.append("<text:list-item>");
                content.append(s);
                content.append("</text:list-item>");
            }
        }
        content.append("</text:list>");
        content.append(ticketsFooter.arg(depName, zavDep));
    }
    content.append(contentEnd);

    // создание файла
    QFile file1(filePath + "content.xml");
    if(file1.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file1);
        out.setCodec("UTF-8");
        out.setGenerateByteOrderMark(true);
        out << content;
        file1.close();
    }
    else
        return 2;
    return 0;
}
