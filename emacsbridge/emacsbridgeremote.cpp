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
}

void EmacsBridgeRemote::setQuery(const QString &query){
  qDebug()<< "Received query from replica: " << query;
  m_client->queryAgent(query);
}

void EmacsBridgeRemote::clientQueryFinished(const QString &queryKey, const QString &queryResult){
  // TODO: keep track of query keys coming from the remote object, and only send those back there
  emit queryFinished(queryResult);
}
