/**
 * @file emacsserver.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#include "emacsserver.h"
#include "emacsbridgesettings.h"
#include "emacsclient.h"

EmacsServer::EmacsServer(): QThread(){
  qDebug()<< "Emacs server starting up.";
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();
  m_startupTime=QDateTime::currentDateTime();

  m_htmlTemplate="<!DOCTYPE html>\n<html><head><title>%1</title></head><body><h1>%1</h1>%2</body></html>";

  m_server.route("/", [](){
                        return QHttpServerResponse::fromFile(QStringLiteral(":/html/index.html"));
                      });

  m_server.route("/assets/<arg>", [](const QUrl &url){
                                     return url.path();
                                   });

  m_server.route("/lisp/<arg>", [this](const QUrl &url){
                                  if (url.path()=="")
                                    return QHttpServerResponse(listDirectory(":/lisp"));
                                  else
                                    return QHttpServerResponse::fromFile(QStringLiteral(":/lisp/%1").arg(url.path()));
                                 });

  m_server.route("/images/<arg>", [this](const QUrl &url){
                                  if (url.path()=="")
                                    return QHttpServerResponse(listDirectory(":/images"));
                                  else
                                    return QHttpServerResponse::fromFile(QStringLiteral(":/images/%1").arg(url.path()));
                                 });

  m_server.route("/test_connection",
                 [](){
                   EmacsClient *client=EmacsClient::instance();
                   if (client->isConnected())
                     return tr("Emacs connection working.");
                   else
                     return tr("Emacs connection is not working.");
                 });

  m_server.route("/rpc",
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


  m_server.route("/settings",
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

  m_server.listen(QHostAddress(settings->value("http/bindAddress", "127.0.0.1").toString()),
                  settings->value("http/bindPort", 1616).toInt());

  start();
}

EmacsServer::~EmacsServer(){
  qDebug()<< "Closing webserver.";
  wait();
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
  QJsonParseError jsonError;
  QJsonDocument document=QJsonDocument::fromJson(payload, &jsonError);

  if (document.isNull()){
    QString error=tr("Invalid JSON data in request: %1").arg(jsonError.errorString());
    return QHttpServerResponse("text/plain",
                               error.toUtf8(),
                               QHttpServerResponder::StatusCode::BadRequest);
  }

  if (method=="notification"){
    QJsonObject notificationObject=document.object();
    QString notificationTitle=notificationObject["title"].toString("Missing notification title");
    QString notificationText=notificationObject["text"].toString("Missing notification text");

    qDebug()<< "Sending notification: " << notificationText;
    emit notificationAdded(notificationTitle,
                           notificationText);
    return "OK";
  }

  QString error=tr("Invalid or unknown method %1").arg(method);
  return QHttpServerResponse("text/plain",
                             error.toUtf8(),
                             QHttpServerResponder::StatusCode::BadRequest);
}

QHttpServerResponse EmacsServer::settingCall(const QString &setting, const QByteArray &payload){
  QString data=QString::fromUtf8(payload);
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();

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

  QString error=tr("Invalid or unknown setting %1").arg(setting);
  return QHttpServerResponse("text/plain",
                             error.toUtf8(),
                             QHttpServerResponder::StatusCode::BadRequest);
}
