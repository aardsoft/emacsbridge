/**
 * @file emacsclient.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EMACSCLIENT_H
#define _EMACSCLIENT_H

#include <QtCore>

class EmacsClient: public QThread{
    Q_OBJECT

  public:
    static EmacsClient *instance();

  public slots:
    bool queryAgent(const QString &query);
    QString queryResult() const;
    bool isConnected();
    bool isSetup();

  private:
    EmacsClient();
    ~EmacsClient();
    static EmacsClient *emacsClient;
    QWaitCondition m_cond;
    bool m_exitThread, m_queryActive;
    QMutex m_mutex;
    QString m_queryString, m_queryResult;

  private slots:
    QString doQuery(const QString &query, bool ownQuery=false);

  signals:
    void queryStarted(const QString &query);
    void queryFinished(const QString &queryKey, const QString &queryResult);
};

#endif
