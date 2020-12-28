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
    setValue("version", 2);
    endGroup();

    beginGroup("localSocket");
    setValue("socketTemplate", "/tmp/emacs%1/server");
    endGroup();

    beginGroup("http");
    setValue("bindAddress", "127.0.0.1");
    setValue("bindPort", "1616");
    setValue("protocol", "http");
    endGroup();

    beginGroup("networkSocket");
    setValue("address", "127.0.0.1");
    setValue("port", "60325");
    setValue("secret", "");
    endGroup();
  }

  if (settingsVersion<=2){
    beginGroup("core");
    //setValue("version", 3);
    // set to 0 to disable
    setValue("tempTokenValidity", 600);
    endGroup();

    beginGroup("emacs");
    setValue("lispDirectory", "$HOME/.emacs.d/emacsbridge");
    setValue("checkTimer", 0);
#ifdef __ANDROID_API__
    setValue("startScript", "");
    // if empty, use the default termux intent
    setValue("startIntent", "");
#else
    setValue("startScript", "systemctl --user start emacs.service");
#endif
    endGroup();
  }

  if (value("core/auth-token", "").toString().isEmpty()){
    qInfo()<< "Missing UUID";
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

QStringList EmacsBridgeSettings::validKeys(){
  return QStringList({
      "core/socketType",
      "core/tempTokenValidity",
      "emacs/checkTimer",
      "emacs/lispDirectory",
#ifdef __ANDROID_API__
      "emacs/startIntent",
#endif
      "emacs/startScript",
      "localSocket/socketTemplate",
      "http/bindAddress",
      "http/bindPort",
      "http/protocol",
      "networkSocket/address",
      "networkSocket/port",
      "networkSocket/secret"
    });
}
