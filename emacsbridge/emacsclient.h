/**
 * @file emacsclient.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EMACSCLIENT_H
#define _EMACSCLIENT_H

#include <QtCore>
#include <QIODevice>

class EmacsClient: public QObject{
    Q_OBJECT

  public:
    enum LispType{
      Invalid,
      String,
      Number,
      Object,
      Error,
    };
    struct Query{
        QString key;
        QString queryString;
    };
    struct QueryResult{
        QString key;
        QString result;
        LispType type;
        QDateTime timestamp;
    };
    static EmacsClient *instance();

  public slots:
    bool queryAgent(const QString &queryKey, const QString &queryString);
    bool isConnected();
    bool isSetup();

  private:
    EmacsClient();
    ~EmacsClient();
    static EmacsClient *emacsClient;
    QString query(const QString &queryKey, const QString &queryString, QIODevice &socket);
    QQueue<Query> queries;

  private slots:
    QString doQuery(const QString &queryKey, const QString &queryString);

  signals:
    void queryStarted(const QString &queryKey, const QString &queryString);
    void queryFinished(const QString &queryKey, const QString &queryResult);
    void queryFinished(const QueryResult &result);
    void queryError(const QString &queryKey, const QString &errorMessage);
};

#endif
