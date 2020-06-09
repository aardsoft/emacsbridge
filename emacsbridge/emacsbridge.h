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
    Q_PROPERTY(bool mobile READ mobile CONSTANT)
    Q_PROPERTY(QDateTime serviceStartupTime READ serviceStartupTime CONSTANT)
    Q_PROPERTY(QDateTime startupTime READ startupTime CONSTANT)
  public:
    explicit EmacsBridge(QObject *parent=0);
    ~EmacsBridge();

    Q_INVOKABLE void runQuery(const QString &queryKey, const QString &query);
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
    void queryFinished(const QString &queryKey, const QString &queryResult);
    void queryError(const QString &queryKey, const QString &errorMessage);

  private slots:
#ifndef __ANDROID_API__
    void updateNotification(const QString &title, const QString &message);
#endif
    void updateQueryResult(const QString &queryKey, const QString &result);

  private:
    QDateTime m_startupTime;
#ifndef __ANDROID_API__
    void startServiceProcess();
    QSystemTrayIcon *m_trayIcon;
    QProcess m_serviceProcess;
#endif
    QRemoteObjectNode m_repNode;
    QSharedPointer<EmacsBridgeRemoteReplica> m_rep;
};

#endif
