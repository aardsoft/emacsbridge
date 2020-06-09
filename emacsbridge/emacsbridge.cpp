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

#include <QUuid>

#include "emacsbridgesettings.h"

EmacsBridge::EmacsBridge(QObject *parent)
    : QObject(parent){
  m_startupTime=QDateTime::currentDateTime();

#ifndef __ANDROID_API__
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

#ifndef __ANDROID_API__
  QMenu *trayIconMenu=new QMenu();
  trayIconMenu->setObjectName("trayIconMenu");
  trayIconMenu->installEventFilter(this);

  m_trayIcon=new QSystemTrayIcon(this);
  m_trayIcon->setContextMenu(trayIconMenu);
  m_trayIcon->setIcon(QIcon(":/images/icon_mini.png"));
  m_trayIcon->show();

  connect(m_rep.data(), SIGNAL(notificationAdded(QString, QString)),
          this, SLOT(updateNotification(QString, QString)));
#endif

  connect(m_rep.data(), SIGNAL(queryFinished(QString, QString)),
          this, SLOT(updateQueryResult(QString, QString)));
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

void EmacsBridge::runQuery(const QString &queryKey, const QString &query){
  m_rep->setQuery(queryKey, query);
}

QDateTime EmacsBridge::startupTime() const{
  return m_startupTime;
}

QDateTime EmacsBridge::serviceStartupTime() const{
  return m_rep->startupTime();
}

#ifndef __ANDROID_API__
void EmacsBridge::updateNotification(const QString &title, const QString &message){
  m_trayIcon->showMessage(title,
                          message);
}
#endif

void EmacsBridge::updateQueryResult(const QString &queryKey, const QString &result){
  // TODO: should cache query key, result and timestamp
  emit queryFinished(queryKey, result);
}
