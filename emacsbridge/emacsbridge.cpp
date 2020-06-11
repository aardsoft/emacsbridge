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

  QSettings settings;
  m_drawerComponents=settings.value("drawerComponents").toStringList();

  m_repNode.connectToNode(QUrl(QStringLiteral("local:replica")));
  m_rep=QSharedPointer<EmacsBridgeRemoteReplica>(m_repNode.acquire<EmacsBridgeRemoteReplica>());

#ifndef __ANDROID_API__
  if (!m_rep->waitForSource(1)){
    qDebug()<< "Starting service process";
    QStringList serviceArguments;
    serviceArguments << "-service";
    QProcess::startDetached(QCoreApplication::applicationFilePath(),
                            serviceArguments);
  }else{
    qDebug()<< "Connected to existing process";
  }
#else
  QAndroidJniObject::callStaticMethod<void>(
    "fi/aardsoft/emacsbridge/EmacsBridgeService",
    "startEmacsBridgeService",
    "(Landroid/content/Context;)V",
    QtAndroid::androidActivity().object());
#endif

  if (!m_rep->isInitialized()){
    bool res=m_rep->waitForSource();
    Q_ASSERT(res);
  }

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
  connect(m_rep.data(), SIGNAL(componentAdded(QmlFileContainer)),
          this, SLOT(addComponent(QmlFileContainer)));
  connect(m_rep.data(), SIGNAL(componentRemoved(QString)),
          this, SLOT(removeComponent(QString)));
  connect(m_rep.data(), SIGNAL(dataSet(JsonDataContainer)),
          this, SLOT(setData(JsonDataContainer)));
}

EmacsBridge::~EmacsBridge(){
#ifndef __ANDROID_API__
  if (m_isDummy)
    qDebug()<< "Killing dummy class";
#endif
}

void EmacsBridge::addComponent(const QmlFileContainer &qmlFile){
  QSettings settings;
  qDebug()<<"Adding component";
  //m_drawerComponents.insert(qmlFile.fileName, qmlFile.title);
  settings.beginGroup("component_"+qmlFile.fileName);
  settings.setValue("title", qmlFile.title);
  settings.setValue("inDrawer", qmlFile.inDrawer);
  settings.endGroup();

  if (qmlFile.inDrawer && !m_drawerComponents.contains(qmlFile.fileName)){
    m_drawerComponents.append(qmlFile.fileName);
    settings.setValue("drawerComponents", m_drawerComponents);
  }

  if (!qmlFile.inDrawer && m_drawerComponents.contains(qmlFile.fileName)){
    m_drawerComponents.removeAll(qmlFile.fileName);
    settings.setValue("drawerComponents", m_drawerComponents);
  }
  emit componentAdded(qmlFile);
}

void EmacsBridge::removeComponent(const QString &qmlFile){
  QSettings settings;
  qDebug()<<"Removing component";

  m_drawerComponents.removeAll(qmlFile);
  settings.setValue("drawerComponents", m_drawerComponents);

  emit componentRemoved(qmlFile);
}

void EmacsBridge::runQuery(const QString &queryKey, const QString &query){
  m_rep->setQuery(queryKey, query);
}

void EmacsBridge::initDrawer(){
  QSettings settings;

  for (const auto& i: m_drawerComponents){
    QmlFileContainer c;
    c.fileName=i;
    settings.beginGroup("component_"+i);
    c.title=settings.value("title", i).toString();
    settings.endGroup();
    c.inDrawer=true;
    emit componentAdded(c);
  }
}

void EmacsBridge::setData(const JsonDataContainer &jsonContainer){
  Q_ASSERT(qmlEngine(this));
  const auto& global = qmlEngine(this)->globalObject();
  const auto& json = global.property("JSON");
  QJSValue parse = json.property("parse");
  QJSValue parsedJson = parse.call(QList<QJSValue>() <<
                              QJSValue(jsonContainer.jsonData));

  emit dataSet(jsonContainer.requesterId, parsedJson);
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
