#ifndef GENTICKETS_H
#define GENTICKETS_H

#include "gentickets_global.h"
#include <QtSql>
#include <QtXml>
#include <QtXmlPatterns/QXmlQuery>
#include <QVariantList>

#include "JlCompress.h"

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

    void                    setConnection(QSqlDatabase openedDB);

    int                     loadQuestionsToList(int questListID, QString filepath);
    int                     removeQuestionsFromList(int questListID);
    int                     generateTickets(int ticketsListID, double lvlrate = -1, double topicrate = -1);
    int                     createOutputFile(int ticketsListID, QString filepath);

//    void                    createOutputFile(QString ticketsListID, QString filename);
//    QVector<QDomElement>    tickets(QDomDocument& domDoc, QString ticketsIdForGenerating);
//    QDomElement             makeElement(QDomDocument& domDoc, const QString& strName,
//                                        const QString& strAttr = QString::null,
//                                        const QString& strText = QString::null
//                                        );

    //int                     generateTickets(QString ticketsListID);
    //int                     loadQuestionsList(int questListID, QString filename);    // заполняем таблицу вопросы

private:
    QSqlDatabase            db;
    QSqlTableModel          model;
    QFile                   file;
    bool                    dbExists = false;

    void                    ShuffleList(QList<int> &list);
    int                     maximumListItem(QList<int> list);
    int                     minimumListItem(QList<int> list);

    void                    addTicketsToDatabase(QMap<int, QList<int> > map, int ticketsListID);
    bool                    checkLeveling(QMap<int, QList<int> > map, QList<int> idsList, int ticketsNum, double levelrate);
    bool                    checkTopicAlign(QMap<int, QList<int> > map, QList<int> idsList, int ticketsNum, double topicrate);

    int                     createManifestFile(int ticketsListID);
    int                     createContentFile(int ticketsListID);
};

#endif // GENTICKETS_H
