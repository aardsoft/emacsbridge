/**
 * @file emacsserver.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#include "emacsserver.h"
#include "emacsbridgesettings.h"

EmacsServer::EmacsServer(): QThread(){
  qDebug()<< "Emacs server starting up.";
  m_startupTime=QDateTime::currentDateTime();

  m_server.route("/hello_world", []() {
                                   return "hello world";
                                 });

  m_server.route("/", [](){
                        qDebug()<< "Root";
                        return QHttpServerResponse::fromFile(QStringLiteral(":/html/index.html"));
                      });

  m_server.route("/assets/<arg>", [](const QUrl &url){
                                     return url.path();
                                   });

  m_server.route("/images/<arg>", [](const QUrl &url){
                                   qDebug()<< url.path();
                                   return QHttpServerResponse::fromFile(QStringLiteral(":/images/%1").arg(url.path()));
                                 });

  // run a connection check on the configured socket. If it succeeds, set the
  // `configured' setting, disabling the init URLs
  /*
  m_server.route("/test_connection",
                 [](){
                 });
  */

  m_server.route("/settings",
                 "POST",
                 [](const QHttpServerRequest &request){
                   EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();

                   if (settings->value("core/configured", false).toBool()==true){
                     QString authHeader=request.headers()["auth-token"].toString();
                     // once initial configuration is done re-configuration requires
                     // a valid auth token
                     if (authHeader!=settings->value("core/auth-token").toString())
                       return QHttpServerResponse(QHttpServerResponder::StatusCode::Forbidden);
                   }

                   QString settingsHeader=request.headers()["setting"].toString();
                   qDebug()<< "Setting header: " << settingsHeader;

                   if (settingsHeader=="server-key"){
                     if (request.body().length()==64){
                       qDebug()<< "Found valid auth key " << request.body();
                       // TODO: include data here
                       QString serverKey=QString::fromUtf8(request.body());
                       settings->setValue("networkSocket/secret", serverKey);
                       return QHttpServerResponse(QHttpServerResponder::StatusCode::Ok);
                     } else {
                       qDebug()<< "Invalid key length " << request.body().length() << " for " << request.body();
                       return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
                     }
                   } else if (settingsHeader=="server-socket"){
                     QString serverSocket=QString::fromUtf8(request.body());
                     settings->setValue("localSocket/socket", serverSocket);
                   } else {
                     // TODO, include message about invalid header here
                     return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
                   }
                   // fallthrough, all non-OK codes should be handled above
                   return QHttpServerResponse(QHttpServerResponder::StatusCode::Ok);
                 });

// NotFound, ExpectationFailed, Unauthorized
  m_server.route("/forbidden", [](){
                                 return QHttpServerResponse(QHttpServerResponder::StatusCode::Forbidden);
                               });

  m_server.route("/unauthorized", [](){
                                 return QHttpServerResponse(QHttpServerResponder::StatusCode::Unauthorized);
                               });

  m_server.listen(QHostAddress::Any, 1616);


  start();
}

EmacsServer::~EmacsServer(){
  qDebug()<< "Closing webserver.";
  wait();
}
