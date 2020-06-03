/**
 * @file emacsservice.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#include "emacsservice.h"
#include "emacsbridgesettings.h"

EmacsService::EmacsService(): QThread(){
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();

  m_startupTime=QDateTime::currentDateTime();
  m_remote.setStartupTime(m_startupTime);

  m_srcNode.setHostUrl(QUrl(QStringLiteral("local:replica")));
  m_srcNode.enableRemoting(&m_remote);

  m_client=EmacsClient::instance();
  connect(settings, SIGNAL(settingChanged(QString)), this, SLOT(settingChanged(QString)));
  start();
}

EmacsService::~EmacsService(){
}

void EmacsService::settingChanged(const QString &key){
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();

  qDebug() << "Setting changed signal for " << key;
  if (key=="remoteSocket/secret"){
    m_remote.setAuthToken(settings->value(key).toString());
  }
}
