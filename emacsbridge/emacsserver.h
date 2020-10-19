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
#include <QJsonObject>

#include "emacsbridgetypes.h"

class EmacsServer: public QObject{
    Q_OBJECT

  public:
    EmacsServer();
    ~EmacsServer();

  public slots:
    void startServer();
    void restartServer();
    quint16 activeServerPort();
#ifdef __ANDROID_API__
    QString callIntent(const QString &jsonString);
#endif

  private:
    QHttpServer *m_server;
    QDateTime m_startupTime;
    QString m_htmlTemplate;
    QString m_dataPath;

    QString listDirectory(const QString &directory);
    QHttpServerResponse methodCall(const QString &method, const QByteArray &payload);
    QHttpServerResponse settingCall(const QString &setting, const QByteArray &payload);

    QHttpServerResponse addComponent(const QJsonObject &jsonObject);
    QHttpServerResponse addNotification(const QJsonObject &jsonObject);
#ifdef __ANDROID_API__
    QHttpServerResponse handleIntent(const QJsonObject &jsonObject,
                                     const QString &jsonString);
#endif
    QHttpServerResponse removeComponent(const QJsonObject &jsonObject);
    QHttpServerResponse setData(const QJsonObject &jsonObject, const QString &jsonString);
  private slots:

  signals:
    void componentAdded(const QmlFileContainer &qmlFile);
    void componentRemoved(const QString &qmlFile);
    void dataSet(const JsonDataContainer &jsonData);
    void notificationAdded(const QString &title, const QString &message);
    void activePortChanged(const quint16 port);
};

#endif
