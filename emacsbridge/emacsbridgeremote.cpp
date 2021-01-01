/**
 * @file emacsbridgeremote.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2020
 */

#include "emacsbridgeremote.h"
#include "emacsbridgesettings.h"
#include "emacsbridgelog.h"

// currently that's not doing much other than just passing on signals (i.e.,
// could be done with less monkey work), but I like the option of filtering,
// caching or modifying some of the data at that level at some point in the
// future - so doing it this way should save some major refacturing later on.

EmacsBridgeRemote::EmacsBridgeRemote(QObject *parent): EmacsBridgeRemoteSimpleSource(){
  m_client=EmacsClient::instance();

  connect(m_client, SIGNAL(queryFinished(QString, QString)),
          this, SLOT(clientQueryFinished(QString, QString)));
  connect(m_client, SIGNAL(queryError(QString, QString)),
          this, SLOT(clientQueryError(QString, QString)));
}

void EmacsBridgeRemote::setQuery(const QString &queryKey, const QString &query){
  m_client->queryAgent(queryKey, query);
}

void EmacsBridgeRemote::addComponent(const QmlFileContainer &qmlFile){
  emit componentAdded(qmlFile);
}

void EmacsBridgeRemote::removeComponent(const QString &qmlFile){
  emit componentRemoved(qmlFile);
}

void EmacsBridgeRemote::addLog(const QString &message){
  emit logAdded(message);
}

void EmacsBridgeRemote::addLogEntry(const QHash<QString, QVariant> entry){
  EmacsBridgeLog::injectLogEntry(entry);
}

void EmacsBridgeRemote::clientQueryError(const QString &queryKey, const QString &error){
  // TODO: keep track of query keys coming from the remote object, and only send those back there
  emit queryFinished(queryKey, error);
}

void EmacsBridgeRemote::clientQueryFinished(const QString &queryKey, const QString &queryResult){
  // TODO: keep track of query keys coming from the remote object, and only send those back there
  emit queryFinished(queryKey, queryResult);
}

void EmacsBridgeRemote::displayNotification(const QString &title, const QString &message){
  emit notificationAdded(title, message);
}

QStringList EmacsBridgeRemote::logBuffer(){
  return EmacsBridgeLog::logBuffer();
}

QVariant EmacsBridgeRemote::serverProperty(const QString &key){
  return EmacsBridgeSettings::serverProperty(key);
}

void EmacsBridgeRemote::setData(const JsonDataContainer &jsonContainer){
  emit dataSet(jsonContainer);
}

void EmacsBridgeRemote::requestAndroidPermission(const QString &permissionName){
  emit androidPermissionRequested(permissionName);
}

void EmacsBridgeRemote::setupTermux(){
  emit termuxSetupRequested();
}
