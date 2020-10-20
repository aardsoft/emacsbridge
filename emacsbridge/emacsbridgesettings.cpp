/**
 * @file emacsbridgesettings.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#include <QDebug>
#include <QUuid>
#include <QStandardPaths>
#include "emacsbridgesettings.h"

EmacsBridgeSettings *EmacsBridgeSettings::emacsBridgeSettings=0;

EmacsBridgeSettings *EmacsBridgeSettings::instance(){
  if (!emacsBridgeSettings){
    emacsBridgeSettings=new EmacsBridgeSettings();
  }

  return emacsBridgeSettings;
}

EmacsBridgeSettings::EmacsBridgeSettings():
  QSettings(
    QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)+"/servicesettings.ini",
    QSettings::IniFormat){
  int settingsVersion=value("core/version", 1).toInt();

  if (settingsVersion<=1){
    beginGroup("core");
#ifdef __ANDROID_API__
    setValue("socketType", 1);
#else
    setValue("socketType", 0);
#endif
    endGroup();

    beginGroup("localSocket");
    setValue("socketTemplate", "/tmp/emacs%1/server");
    endGroup();

    beginGroup("networkSocket");
    setValue("address", "127.0.0.1");
    setValue("port", "60325");
    setValue("secret", "");
    endGroup();
  }

  if (value("core/auth-token", "").toString().isEmpty()){
    qDebug()<< "Missing UUID";
    QString uuid=QUuid::createUuid().toString();
    uuid.replace("-", "");
    setValue("core/auth-token", uuid);
  }

  m_initialized=true;
}

void EmacsBridgeSettings::setValue(const QString &key, const QVariant &value){
  if (QSettings::value(key) == QVariant()){
    qDebug()<< "setting initial value on " << key;
    QSettings::setValue(key, value);
    if (m_initialized)
      emit settingChanged(key);
  } else if (QSettings::value(key) == value){
    qDebug()<< "value didn't change for " << key;
  } else {
    QSettings::setValue(key, value);
    qDebug()<< "new value for " << key;
    if (m_initialized)
      emit settingChanged(key);
  }
}
