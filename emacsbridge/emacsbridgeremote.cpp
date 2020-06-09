/**
 * @file emacsbridgeremote.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2020
 */

#include "emacsbridgeremote.h"

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
