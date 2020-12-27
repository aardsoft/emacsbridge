/**
 * @file emacsserver.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifdef __ANDROID_API__
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#endif

#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>

#include "emacsserver.h"
#include "emacsbridgesettings.h"
#include "emacsclient.h"
#include "emacsbridge.h"

EmacsServer::EmacsServer(): QObject(){
  qDebug()<< "Emacs server starting up.";
  m_startupTime=QDateTime::currentDateTime();
  m_dataPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  m_htmlTemplate="<!DOCTYPE html>\n<html><head><title>%1</title></head><body><h1>%1</h1>%2</body></html>";

  // everything more complex needs to go to startServer() to make sure it is in
  // the correct thread
}

EmacsServer::~EmacsServer(){
  qDebug()<< "Closing webserver.";
}

void EmacsServer::startServer(){
  m_morseInterpreter=new EmacsBridgeMorse();
  m_ambientLightSensor=new QAmbientLightSensor(this);
#ifdef _WITH_PROXIMITY
  m_proximitySensor=new QProximitySensor(this);
#endif

  connect(this, SIGNAL(parseMorse(int)),
          m_morseInterpreter, SLOT(stateChange(int)));

  connect(m_ambientLightSensor, &QAmbientLightSensor::readingChanged,
          this,
          [=](){m_morseInterpreter->
              stateChange(m_ambientLightSensor->
                          reading()->lightLevel());});
#ifdef _WITH_PROXIMITY
  connect(m_proximitySensor, &QProximitySensor::readingChanged,
          this,
          [=](){qDebug()<<"Proximity:" <<m_proximitySensor->reading()->close();
            qDebug()<<"Proximity:" <<(m_proximitySensor->reading()->close()?1:3);
          });
#endif

  m_ambientLightSensor->setDataRate(1);
  m_ambientLightSensor->start();

#ifdef _WITH_PROXIMITY
  m_proximitySensor->setDataRate(1);
  m_proximitySensor->start();
#endif
  startHttpServer();
}

void EmacsServer::startHttpServer(){
  qDebug()<<"startServer" << QThread::currentThreadId();
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();
  m_server=new QHttpServer();
  m_server->route("/", [this](){
    return parseFile(":/html/index.html");
  });

  m_server->route("/lisp/<arg>", [this](const QUrl &url){
                                  if (url.path()=="")
                                    return QHttpServerResponse(listDirectory(":/lisp"));
                                  else
                                    return QHttpServerResponse::fromFile(QStringLiteral(":/lisp/%1").arg(url.path()));
                                 });

  m_server->route("/icons/<arg>", [this](const QUrl &url){
                                  if (url.path()=="")
                                    return QHttpServerResponse(listDirectory(":/icons"));
                                  else
                                    return QHttpServerResponse::fromFile(QStringLiteral(":/icons/%1").arg(url.path()));
                                 });

  m_server->route("/scripts/<arg>", [this](const QUrl &url){
    if (url.path()=="")
      return QHttpServerResponse(listDirectory(":/scripts"));
    else
      return parseFile(QStringLiteral(":/scripts/%1").arg(url.path()));
  });

  m_server->route("/html/<arg>", [this](const QUrl &url){
    if (url.path()=="")
      return parseFile(":/html/index.html");
    else
      return parseFile(QStringLiteral(":/html/%1").arg(url.path()));
  });

  m_server->route("/test_connection",
                 [](){
                   EmacsClient *client=EmacsClient::instance();
                   if (client->isConnected())
                     return tr("Emacs connection working.");
                   else
                     return tr("Emacs connection is not working.");
                 });

  m_server->route("/rpc",
                 "POST",
                 [this,settings](const QHttpServerRequest &request)->QHttpServerResponse{
                   if (settings->value("core/configured", false).toBool()==false){
                     QString error=tr("Emacs bridge is not configured. Please follow the documentation for initial configuration.");
                     return QHttpServerResponse("text/plain",
                                                error.toUtf8(),
                                                QHttpServerResponder::StatusCode::Forbidden);
                   }
                   QString authHeader=request.headers()["auth-token"].toString();
                   if (authHeader=="" || authHeader!=settings->value("core/auth-token").toString()){
                     QString error=
                       tr("Invalid or missing auth token (%1). RPC API requires an auth token for access.")
                       .arg(authHeader);
                     return QHttpServerResponse("text/plain",
                                                error.toUtf8(),
                                                QHttpServerResponder::StatusCode::Forbidden);
                   }
                   QString methodHeader=request.headers()["method"].toString();
                   return methodCall(methodHeader, request.body());
                 });


  m_server->route("/settings",
                 "POST",
                 [this,settings](const QHttpServerRequest &request)->QHttpServerResponse{
                   if (settings->value("core/configured", false).toBool()==true){
                     QString authHeader=request.headers()["auth-token"].toString();
                     // once initial configuration is done re-configuration requires
                     // a valid auth token
                     if (authHeader=="" || authHeader!=settings->value("core/auth-token").toString()){
                       QString error=
                         tr("Invalid or missing auth token (%1). This instance is configured, and requires an auth token for further changes.")
                         .arg(authHeader);
                       return QHttpServerResponse("text/plain",
                                                  error.toUtf8(),
                                                  QHttpServerResponder::StatusCode::Forbidden);
                     }
                   }

                   QString settingsHeader=request.headers()["setting"].toString();
                   qDebug()<< "Setting header: " << settingsHeader;

                   return settingCall(settingsHeader, request.body());
                 });

  m_server->listen(QHostAddress(settings->value("http/bindAddress", "127.0.0.1").toString()),
                  settings->value("http/bindPort", 1616).toInt());
  qDebug()<<"HTTP server initialized at"
          <<settings->value("http/bindAddress", "127.0.0.1").toString()
          <<"port"
          <<settings->value("http/bindPort", 1616).toInt();
  emit activePortChanged(activeServerPort());
}

void EmacsServer::restartServer(){
  qDebug()<< "Restarting HTTP server";
  delete m_server;
  startHttpServer();
}

QString EmacsServer::listDirectory(const QString &directory){
  QDirIterator it(directory, QDirIterator::Subdirectories);
  QString ret="<ul><li><a href=\"..\">..</a></li>\n";
  while (it.hasNext()) {
    QFileInfo f(it.next());
    ret+="<li><a href=\""%f.fileName()%"\">"%f.fileName()%"</a></li>\n";
  }
  ret+="</ul>\n";

  return m_htmlTemplate.arg(tr("Directory listing for ")%directory).arg(ret);
}

QHttpServerResponse EmacsServer::methodCall(const QString &method, const QByteArray &payload){
  qDebug()<<"Server" << QThread::currentThreadId();

  QJsonParseError jsonError;
  QJsonDocument document=QJsonDocument::fromJson(payload, &jsonError);

  if (document.isNull()){
    QString error=tr("Invalid JSON data in request: %1").arg(jsonError.errorString());
    return QHttpServerResponse("text/plain",
                               error.toUtf8(),
                               QHttpServerResponder::StatusCode::BadRequest);
  }

  QJsonObject jsonObject=document.object();

  if (method=="addComponent"){
    return addComponent(jsonObject);
  }else if (method=="setData"){
    return setData(jsonObject, QString::fromUtf8(payload));
  }else if (method=="notification"){
    return addNotification(jsonObject);
#ifdef __ANDROID_API__
  }else if (method=="intent"){
    return handleIntent(jsonObject, QString::fromUtf8(payload));
#endif
  }else if (method=="sensor"){
    return handleSensorCall(jsonObject);
  }else if (method=="removeComponent"){
    return removeComponent(jsonObject);
  }

  QString error=tr("Invalid or unknown method %1").arg(method);
  return QHttpServerResponse("text/plain",
                             error.toUtf8(),
                             QHttpServerResponder::StatusCode::BadRequest);
}

QHttpServerResponse EmacsServer::settingCall(const QString &setting, const QByteArray &payload){
  QString data=QString::fromUtf8(payload);
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();

  // server-key and server-socket are two special cases configurable with a mix
  // of the key in the header, and the data without padding in POST to make
  // things easier
  if (setting=="server-key"){
    if (data.length()==64){
      settings->setValue("networkSocket/secret", data);
      return tr("Accepted auth key: %1\n\nYou may kill this buffer now.").arg(data);
    } else {
      QString error=tr("Invalid key length %1 (expected: 64) for submitted key %2")
        .arg(data.length())
        .arg(data);
      return QHttpServerResponse("text/plain",
                                 error.toUtf8(),
                                 QHttpServerResponder::StatusCode::BadRequest);
    }
  } else if (setting=="server-socket"){
    settings->setValue("localSocket/socket", data);
    return tr("Server socket is set to %1").arg(data);
  }

  // if we got to this point we have a POST string with key-value pairs
  // we need to parse.
  qDebug()<<payload;
  QString queryString=QString::fromUtf8(payload);
  queryString.replace("+", " ");

  QUrlQuery query;
  query.setQuery(queryString);
  qDebug()<<query.toString(QUrl::FullyDecoded);
  QString error=tr("Invalid or unknown setting %1").arg(setting);
  return QHttpServerResponse("text/plain",
                             error.toUtf8(),
                             QHttpServerResponder::StatusCode::BadRequest);
}

QHttpServerResponse EmacsServer::addComponent(const QJsonObject &jsonObject){
  qDebug()<< "Uploading QML";
  QmlFileContainer qmlFileContainer;
  qDebug()<<jsonObject["in-drawer"].toString("<empty>")
          <<jsonObject["in-drawer"].toBool(false);
  qmlFileContainer.inDrawer=jsonObject["in-drawer"].toBool(false);
  QString qmlData=jsonObject["qml-data"].toString("");

  if (qmlData.isEmpty()){
    return QHttpServerResponse("text/plain",
                               tr("QML data missing in request").toUtf8(),
                               QHttpServerResponder::StatusCode::BadRequest);
  }

  QString qmlFile=jsonObject["file-name"].toString("");

  if (qmlFile.isEmpty()){
    return QHttpServerResponse("text/plain",
                               tr("QML filename missing in request").toUtf8(),
                               QHttpServerResponder::StatusCode::BadRequest);
  }

  QFileInfo info(qmlFile);
  QFile stagingFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                    +"/qml-staging/"
                    +info.fileName());
  QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
             +"/qml/"
             +info.fileName());

  qmlFileContainer.title=jsonObject["title"].toString("");
  if (qmlFileContainer.title=="")
    qmlFileContainer.title=info.fileName();

  qmlFileContainer.fileName=info.fileName();

  if (!stagingFile.open(QIODevice::WriteOnly | QIODevice::Text)){
    return QHttpServerResponse("text/plain",
                               tr("Unable to open file for writing").toUtf8(),
                               QHttpServerResponder::StatusCode::BadRequest);
  }

  stagingFile.write(qmlData.toUtf8());
  stagingFile.flush();
  stagingFile.close();

  EmacsBridge emacsBridge("");
  QQmlEngine engine;
  qmlRegisterType<EmacsBridge>("fi.aardsoft.emacsbridge",1,0,"EmacsBridge");
  engine.rootContext()->setContextProperty("emacsBridge", &emacsBridge);
  QQmlComponent component(&engine, QUrl::fromLocalFile(stagingFile.fileName()));
  if (component.isError()){
    qDebug()<<"Component load error";
    QList<QQmlError> qmlErrors=component.errors();
    QList<QQmlError>::const_iterator i;
    QString errors="Unable to push QML file:\n\n";
    for (i=qmlErrors.constBegin(); i!=qmlErrors.constEnd(); ++i){
      qDebug()<<(*i).toString();
      errors+=(*i).toString();
    }
    return QHttpServerResponse("text/plain",
                               errors.toUtf8(),
                               QHttpServerResponder::StatusCode::BadRequest);
  }else{
    QDir dir;

    if (dir.exists(stagingFile.fileName()))
      dir.remove(file.fileName());

    if (!dir.rename(stagingFile.fileName(),
                    file.fileName())){
      qDebug()<< "Rename failed.";
      return QHttpServerResponse("text/plain",
                                 tr("Unable to rename uploaded file").toUtf8(),
                                 QHttpServerResponder::StatusCode::BadRequest);
    }

    qDebug()<< "Notify!";
    emit componentAdded(qmlFileContainer);
    return "OK";
  }
}

QHttpServerResponse EmacsServer::addNotification(const QJsonObject &jsonObject){
  QString notificationTitle=jsonObject["title"].toString("Missing notification title");
  QString notificationText=jsonObject["text"].toString("Missing notification text");

  qDebug()<< "Sending notification: " << notificationText;
  emit notificationAdded(notificationTitle,
                         notificationText);
  return "OK";
}

#ifdef __ANDROID_API__
QString EmacsServer::callIntent(const QString &jsonString){
  QAndroidJniEnvironment env;

  QString returnString="OK";
  QAndroidJniObject jJsonData=QAndroidJniObject::fromString(jsonString);
  QAndroidJniObject::callStaticMethod<void>(
    "fi/aardsoft/emacsbridge/EmacsBridgeService",
    "callIntentFromJson",
    "(Landroid/content/Context;Ljava/lang/String;)V",
    QtAndroid::androidContext().object(),
    jJsonData.object<jstring>());

  if (env->ExceptionCheck()) {
    jthrowable exception=env->ExceptionOccurred();
    // JNI blocks pretty much everything without clearing the exception
    env->ExceptionClear();
    jmethodID jToString=
      env->GetMethodID(env->FindClass("java/lang/Object"),
                       "toString",
                       "()Ljava/lang/String;");

    jstring exceptionString=(jstring)env->CallObjectMethod(exception, jToString);
    returnString=env->GetStringUTFChars(exceptionString, 0);
  }

  return returnString;
}

QString EmacsServer::checkPermissions(const QString &intentClass){
  QString permissionName="";

  if (intentClass=="com.termux.app.RunCommandService"){
    permissionName="com.termux.permission.RUN_COMMAND";
  }

  if (!permissionName.isEmpty()){
    auto result=QtAndroid::checkPermission(permissionName);
    if(result==QtAndroid::PermissionResult::Denied){
      // permissions can't be requested from android services -> just
      // fail quickly after notifying the UI to ask for permissions
      emit androidPermissionDenied(permissionName);
      return permissionName;
    }
  }

  return "";
}

QHttpServerResponse EmacsServer::handleIntent(const QJsonObject &jsonObject,
                                            const QString &jsonString){
  QString className=jsonObject["class"].toString("");
  QString ret;

  if (!className.isEmpty()){
    ret=checkPermissions(className);
    if (!ret.isEmpty())
          return QHttpServerResponse("text/plain",
                                     tr("Permission denied for %1")
                                     .arg(ret)
                                     .toUtf8(),
                                     QHttpServerResponder::StatusCode::BadRequest);
  }

  ret=callIntent(jsonString);
  if (ret=="OK"){
    return "OK";
  } else {
    qDebug()<< "Returning" << ret;
    return QHttpServerResponse("text/plain",
                               ret.toUtf8(),
                               QHttpServerResponder::StatusCode::BadRequest);
  }
}
#endif

/*
 * Allow:
 * - starting a sensor
 * - stopping a senosr
 * - checking sensor status
 * - mapping specific sensor values to actions to be executed inside of emacs
 */
QHttpServerResponse EmacsServer::handleSensorCall(const QJsonObject &jsonObject){
  QString sensorName=jsonObject["sensor"].toString("");

}

QHttpServerResponse EmacsServer::parseFile(const QString &fileName){
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text))
    return QHttpServerResponse("text/plain",
                               tr("Unable to open file: %1.")
                               .arg(fileName)
                               .toUtf8(),
                               QHttpServerResponder::StatusCode::BadRequest);

  QTextStream fileStream(&file);
  QString fileContent=fileStream.readAll();
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();
  // probably simple replace of some tokens is enough for what we're doing
  for (const auto& i: EmacsBridgeSettings::validKeys()){
    fileContent.replace(QString("{{%1}}").arg(i),
                        settings->value(i).toString());
  }
  return fileContent;
}

QHttpServerResponse EmacsServer::removeComponent(const QJsonObject &jsonObject){
  QString qmlFile=jsonObject["file-name"].toString("");

  if (qmlFile.isEmpty()){
    return QHttpServerResponse("text/plain",
                               tr("QML filename missing in request").toUtf8(),
                               QHttpServerResponder::StatusCode::BadRequest);
  }

  QFileInfo info(qmlFile);
  QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
             +"/qml/"
             +info.fileName());

  if (file.exists()){
    if (file.remove()){
      emit componentRemoved(info.fileName());
      return "OK";
    }
  }

  return QHttpServerResponse("text/plain",
                             tr("Unable to remove file").toUtf8(),
                             QHttpServerResponder::StatusCode::BadRequest);
}

QHttpServerResponse EmacsServer::setData(const QJsonObject &jsonObject, const QString &jsonString){
  JsonDataContainer jsonContainer;
  jsonContainer.requesterId=jsonObject["requester-id"].toString("");

  // just check if there's a requesterId, and then throw it up the stack to have
  // the UI deal with it.
  if (jsonContainer.requesterId.isEmpty()){
    qDebug()<< "Invalid data:" << jsonContainer.requesterId << jsonString;
    return QHttpServerResponse("text/plain",
                               tr("Requester ID missing in setData request").toUtf8(),
                               QHttpServerResponder::StatusCode::BadRequest);
  }

  jsonContainer.jsonData=jsonString;
  qDebug()<< "Setting data:" << jsonContainer.requesterId << jsonObject;
  emit dataSet(jsonContainer);
  return "OK";
}

quint16 EmacsServer::activeServerPort(){
  QVector<quint16> serverPorts=m_server->serverPorts();

  if (serverPorts.count() <= 0){
    qDebug()<< "HTTP server not listening on any port";
    return 0;
  } else if (serverPorts.count() > 1){
    qDebug()<< "HTTP server listening to wrong number of ports" << serverPorts;
  }

  return serverPorts.at(0);
}
