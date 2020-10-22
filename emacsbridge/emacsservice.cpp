/**
 * @file emacsservice.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifdef __ANDROID_API__
#include <QtAndroid>
#endif

#include "emacsservice.h"
#include "emacsbridgesettings.h"

EmacsService::EmacsService(): QObject(){
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();

  m_remote.setServerListenPort(settings->value("http/bindPort", 1616).toInt());
  m_remote.setServerListenAddress(settings->value("http/bindAddress", "127.0.0.1").toString());
  m_remote.setActiveServerListenPort(0);

  m_startupTime=QDateTime::currentDateTime();
  m_remote.setStartupTime(m_startupTime);

  m_srcNode.setHostUrl(QUrl(QStringLiteral("local:replica")));
  m_srcNode.enableRemoting(&m_remote);

  m_client=EmacsClient::instance();

  qDebug()<<"Service" << QThread::currentThreadId();

  m_server=new EmacsServer();
  m_server->moveToThread(&serverThread);

  connect(settings, SIGNAL(settingChanged(QString)), this, SLOT(settingChanged(QString)));

  // on Android the notification needs to come from the service, while on PC
  // it comes from the GUI process, making this a bit complicated.
#ifdef __ANDROID_API__
  connect(m_server, SIGNAL(notificationAdded(QString, QString)),
          this, SLOT(displayNotification(QString, QString)));
  connect(&m_remote, SIGNAL(termuxSetupRequested()),
          this, SLOT(triggerTermuxSetup()));
#else
  connect(m_server, SIGNAL(notificationAdded(QString, QString)),
          &m_remote, SLOT(displayNotification(QString, QString)));
#endif

  connect(m_server, SIGNAL(componentAdded(QmlFileContainer)),
          &m_remote, SLOT(addComponent(QmlFileContainer)));
  connect(m_server, SIGNAL(componentRemoved(QString)),
          &m_remote, SLOT(removeComponent(QString)));
  // 'this' here is required to make sure the lambda gets called in the correct thread
  connect(m_server, &EmacsServer::activePortChanged,
          this,
          [=](const quint16 serverPort){m_remote.setActiveServerListenPort(serverPort);});

  connect(&m_remote, SIGNAL(serverListenPortChanged(quint16)),
          this, SLOT(changeServerListenPort(quint16)));
  connect(&m_remote, SIGNAL(serverListenAddressChanged(QString)),
          this, SLOT(changeServerListenAddress(QString)));
  connect(m_server, SIGNAL(dataSet(JsonDataContainer)),
          &m_remote, SLOT(setData(JsonDataContainer)));
#ifdef __ANDROID_API__
  connect(m_server, SIGNAL(androidPermissionDenied(QString)),
          &m_remote, SLOT(requestAndroidPermission(QString)));
#endif

  connect(this, SIGNAL(startServer()),
          m_server, SLOT(startServer()));
  connect(this, SIGNAL(restartServer()),
          m_server, SLOT(restartServer()));

  serverThread.start();
  emit startServer();
}

EmacsService::~EmacsService(){
}

void EmacsService::displayNotification(const QString &title, const QString &message){
#ifdef __ANDROID_API__
  QAndroidJniObject javaNotification=QAndroidJniObject::fromString(message);
  QAndroidJniObject jNotificationTitle=QAndroidJniObject::fromString(title);
  QAndroidJniObject::callStaticMethod<void>(
    "fi/aardsoft/emacsbridge/EmacsBridgeNotification",
    "notify",
    "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",
    QtAndroid::androidContext().object(),
    javaNotification.object<jstring>(),
    jNotificationTitle.object<jstring>());
#endif
}

void EmacsService::settingChanged(const QString &key){
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();

  qDebug() << "Setting changed signal for " << key;
  if (key.startsWith("localSocket/") || key.startsWith("networkSocket/")){
    if (m_client->isSetup()){
      settings->setValue("core/configured", true);
    }
  }
  if (key.startsWith("http/")){
    qDebug()<<"Triggering server restart";
    emit restartServer();
  }
}

void EmacsService::changeServerListenPort(const quint16 serverPort){
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();
  qDebug()<<"Server port changed to" << serverPort;
  settings->setValue("http/bindPort", serverPort);
}

void EmacsService::changeServerListenAddress(const QString &serverAddress){
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();
  qDebug()<<"Server address changed to" << serverAddress;
  settings->setValue("http/bindAddress", serverAddress);
}

#ifdef __ANDROID_API__
void EmacsService::triggerTermuxSetup(){
  qDebug()<<"Triggering termux setup";

  if (m_server->checkPermissions("com.termux.app.RunCommandService")!="")
    return;

  QFile file(":/json/termux-setup.json");
  if (!file.open(QFile::ReadOnly | QFile::Text))
    return;

  QTextStream fileStream(&file);
  QString fileContent=fileStream.readAll();
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();

  fileContent=fileContent.arg("http")
    .arg(settings->value("http/bindAddress").toString())
    .arg(settings->value("http/bindPort").toString())
    .arg("/scripts/termux-init.sh");

  m_server->callIntent(fileContent);
}
#endif
