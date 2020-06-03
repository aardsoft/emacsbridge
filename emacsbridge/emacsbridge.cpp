/**
 * @file emacsbridge.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#include "emacsbridge.h"

#ifdef __ANDROID_API__
#include <QtAndroid>
#else
#include <QMenu>
#include <QIcon>
#include <QCoreApplication>
#endif

#include "emacsbridgesettings.h"

EmacsBridge::EmacsBridge(QObject *parent)
    : QObject(parent){
  m_startupTime=QDateTime::currentDateTime();

#ifndef __ANDROID_API__
  QMenu *trayIconMenu=new QMenu();
  trayIconMenu->setObjectName("trayIconMenu");
  trayIconMenu->installEventFilter(this);

  m_trayIcon=new QSystemTrayIcon(this);
  m_trayIcon->setContextMenu(trayIconMenu);
  m_trayIcon->setIcon(QIcon(":/images/icon_mini.png"));
  m_trayIcon->show();

  qDebug()<< "Starting service process";
  startServiceProcess();

#else
  QAndroidJniObject::callStaticMethod<void>(
    "fi/aardsoft/emacsbridge/EmacsBridgeService",
    "startEmacsBridgeService",
    "(Landroid/content/Context;)V",
    QtAndroid::androidActivity().object());
#endif

  m_repNode.connectToNode(QUrl(QStringLiteral("local:replica")));
  m_rep=QSharedPointer<EmacsBridgeRemoteReplica>(m_repNode.acquire<EmacsBridgeRemoteReplica>());
  bool res=m_rep->waitForSource();
  Q_ASSERT(res);


  connect(m_rep.data(), SIGNAL(queryFinished(QString)),
          this, SLOT(updateQueryResult(QString)));

  connect(this, SIGNAL(notificationChanged()), this, SLOT(updateNotification()));
}

void EmacsBridge::setNotification(const QString &notification){
  if (m_notification==notification)
    return;

  m_notification=notification;
  emit notificationChanged();
}

EmacsBridge::~EmacsBridge(){
#ifndef __ANDROID_API__
  qDebug()<< "Shutting down service process";
  m_serviceProcess.terminate();
  m_serviceProcess.waitForFinished();

  if (m_serviceProcess.state()==QProcess::Running){
    qDebug()<< "Taking a shotgun to service processes' brain.";
    m_serviceProcess.kill();
  }
#endif
}

#ifndef __ANDROID_API__
void EmacsBridge::startServiceProcess(){
  QStringList serviceArguments;
  serviceArguments << "-service";

  m_serviceProcess.setProcessChannelMode(QProcess::ForwardedChannels);
  m_serviceProcess.start(QCoreApplication::applicationFilePath(),
                       serviceArguments);
}
#endif

QString EmacsBridge::notification() const{
  return m_notification;
}

// TODO: would be better to add a sequence number here
void EmacsBridge::setQuery(const QString &query){
  qDebug()<< "Sending query to emacs: " << query;

  m_query=query;
  m_rep->setQuery(m_query);
}

QDateTime EmacsBridge::startupTime() const{
  return m_startupTime;
}

QDateTime EmacsBridge::serviceStartupTime() const{
  return m_rep->startupTime();
}

QString EmacsBridge::query() const{
  return m_query;
}

QString EmacsBridge::queryResult() const{
  return m_queryResult;
}

void EmacsBridge::updateNotification(){
#ifdef __ANDROID_API__
  QAndroidJniObject javaNotification=QAndroidJniObject::fromString(m_notification);
  QAndroidJniObject jNotificationTitle=QAndroidJniObject::fromString("Notification title");
  QAndroidJniObject::callStaticMethod<void>(
    "fi/aardsoft/emacsbridge/EmacsBridgeNotification",
    "notify",
    "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",
    QtAndroid::androidContext().object(),
    javaNotification.object<jstring>(),
    jNotificationTitle.object<jstring>());
#else
  m_trayIcon->showMessage("Title",
                          m_notification);

#endif
}

void EmacsBridge::updateQueryResult(const QString &result){
  qDebug()<< "Update query result";
  m_queryResult=result;
  emit queryFinished();
}
