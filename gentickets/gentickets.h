#ifndef GENTICKETS_H
#define GENTICKETS_H

#include "gentickets_global.h"
#include <QtSql>
#include <QtXml>

#include "JlCompress.h"

// струткура вопроса
struct Quest{
    int level;
    QString topic;
    QString body;
    QString manifest;
    QStringList hrefList;
};

class GENTICKETSSHARED_EXPORT gentickets
{
public:
    gentickets();

    int                     setConnection(QSqlDatabase openedDB);

    int                     loadQuestionsToList(int questListID, QString filepath);
    int                     removeQuestionsFromList(int questListID);
    int                     generateTickets(int ticketsListID, int lvlrate = -1, double topicrate = -1);
    int                     createOutputFile(int ticketsListID, QString filepath, QMap<QString, QString> *arguments = nullptr);

private:
    QSqlDatabase            db;
    QSqlTableModel          model;
    QFile                   file;
    bool                    dbExists = false;

    bool                    tableExist(QSqlDatabase db, QString tableName);

    void                    ShuffleList(QList<int> &list);
    int                     maximumListItem(QList<int> list);
    int                     minimumListItem(QList<int> list);

    void                    addTicketsToDatabase(QMap<int, QList<int> > map, int ticketsListID);
    bool                    checkLeveling(QMap<int, QList<int> > map, QList<int> idsList, int ticketsNum, int levelrate);
    bool                    checkTopicAlign(QMap<int, QList<int> > map, QList<int> idsList, int ticketsNum, double topicrate);

    int                     createManifestFile(int ticketsListID);
    int                     createContentFile(int ticketsListID, QMap<QString, QString> *arguments = nullptr);
};

#endif // GENTICKETS_H
