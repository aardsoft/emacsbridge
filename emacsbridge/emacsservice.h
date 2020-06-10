/**
 * @file emacsservice.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EMACSSERVICE_H
#define _EMACSSERVICE_H

#include <QtCore>

#include "emacsserver.h"
#include "emacsclient.h"
#include "emacsbridgeremote.h"

class EmacsService: public QObject{
    Q_OBJECT
    QThread serverThread;

  public:
    EmacsService();
    ~EmacsService();

  public slots:

  private:
    QRemoteObjectHost m_srcNode;
    EmacsBridgeRemote m_remote;
    QDateTime m_startupTime;
    EmacsClient *m_client;
    EmacsServer *m_server;

  private slots:
    void settingChanged(const QString &key);
    void displayNotification(const QString &title, const QString &message);

  signals:
    void notificationAdded(const QString &title, const QString &message);
    void startServer();

};

#endif
