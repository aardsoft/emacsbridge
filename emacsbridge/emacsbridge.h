/**
 * @file emacsbridge.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef EMACSBRIDGE_H
#define EMACSBRIDGE_H

#ifndef __ANDROID_API__
#include <QSystemTrayIcon>
#endif

#include <QObject>
#include <QProcess>

#include "rep_emacsbridgeremote_replica.h"

class EmacsBridge: public QObject{
    Q_OBJECT
    Q_PROPERTY(QString notification READ notification WRITE setNotification NOTIFY notificationChanged)
    Q_PROPERTY(QString query READ query WRITE setQuery)
    Q_PROPERTY(QString queryResult READ queryResult NOTIFY queryFinished)
    Q_PROPERTY(bool mobile READ mobile CONSTANT)
    Q_PROPERTY(QDateTime serviceStartupTime READ serviceStartupTime CONSTANT)
    Q_PROPERTY(QDateTime startupTime READ startupTime CONSTANT)
  public:
    explicit EmacsBridge(QObject *parent=0);
    ~EmacsBridge();

    void setNotification(const QString &notification);
    QString notification() const;
    void setQuery(const QString &query);
    QString query() const;
    QString queryResult() const;
    QDateTime serviceStartupTime() const;
    QDateTime startupTime() const;
    bool mobile() const{
#ifdef __ANDROID_API__
      return true;
#else
      return false;
#endif
    };

  signals:
    void notificationChanged();
    void queryFinished();

  private slots:
    void updateNotification(const QString &title, const QString &message);
    void updateQueryResult(const QString &result);

  private:
    QDateTime m_startupTime;
#ifndef __ANDROID_API__
    void startServiceProcess();
    QSystemTrayIcon *m_trayIcon;
    QProcess m_serviceProcess;
#endif
    QRemoteObjectNode m_repNode;
    QSharedPointer<EmacsBridgeRemoteReplica> m_rep;
    QString m_notification;
    QString m_query, m_queryResult;
};

#endif
