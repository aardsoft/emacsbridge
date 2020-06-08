/**
 * @file emacsserver.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EMACSSERVER_H
#define _EMACSSERVER_H

#include <QtCore>
#include <QHttpServer>

class EmacsServer: public QThread{
    Q_OBJECT

  public:
    EmacsServer();
    ~EmacsServer();

  public slots:

  private:
    QHttpServer m_server;
    QDateTime m_startupTime;
    QString m_htmlTemplate;

    QString listDirectory(const QString &directory);
    QHttpServerResponse methodCall(const QString &method, const QByteArray &payload);
    QHttpServerResponse settingCall(const QString &setting, const QByteArray &payload);

  private slots:

  signals:
    void notificationAdded(const QString &title, const QString &message);
};

#endif
