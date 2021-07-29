/**
 * @file emacsclient.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#define OWN_QUERY_KEY "EmacsClient::OwnQuery"

#ifndef __ANDROID_API__
#include <QLocalSocket>
#endif

#include <QTcpSocket>
#include <QRandomGenerator>

#include "emacsclient.h"
#include "emacsbridgesettings.h"

EmacsClient *EmacsClient::emacsClient=0;

EmacsClient *EmacsClient::instance(){
  if (!emacsClient){
    emacsClient=new EmacsClient();
  }

  return emacsClient;
}

/*

this generally should should just do one action at a time, or things get weird.

store a ready state, and only accept changes if ready state is back to normal.

1. accept message
2. change ready state
3. send message
4. wait for reply/error
5. save reply/error
6. change ready state
7. deliver reply

have configurable timer which pings emacs in regular intervals (important on android where emacs might get killed)
 */
EmacsClient::EmacsClient(): QObject(){
  m_checkTimer=new QTimer();
  m_checkTimer->setSingleShot(true);

  connect(m_checkTimer, SIGNAL(timeout()), this, SLOT(timoutHandler()));
  connect(this, SIGNAL(queryStarted(QString, QString)), this, SLOT(doQuery(QString, QString)));

  // this used to be a thread class with a reimplementation of QThread::run,
  // which didn't behave properly on Android. Currently this is running in the
  // main thread, which can block the service process if it's waiting for emacs
  // to reply. With all calls now having query keys it should now be safe to
  // move just the emacs server query to a new thread, and have this class throw
  // incoming queries into a QQueue
  //start();
}

EmacsClient::~EmacsClient(){
}

bool EmacsClient::isSetup(){
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();
  QString queryTemplate="(customize-save-variable 'emacsbridge-auth-token \"%1\")";
  QString authToken=settings->value("core/auth-token").toString();

  if (authToken=="")
    return false;

  QString stringResult=doQuery(OWN_QUERY_KEY, queryTemplate.arg(authToken));

  if (stringResult==authToken){
    qDebug()<< "Auth token query:" << authToken << "==" << stringResult;
    return true;
  } else {
    qWarning()<< "Auth token query:" << stringResult;
    return false;
  }
}

bool EmacsClient::isConnected(){
  quint32 a=QRandomGenerator::global()->bounded(256);
  quint32 b=QRandomGenerator::global()->bounded(256);
  quint32 r=a+b;
  QString queryTemplate="(+ %1 %2)";

  QString stringResult=doQuery(OWN_QUERY_KEY, queryTemplate.arg(a).arg(b));

  bool ok;
  quint32 qr=stringResult.toInt(&ok, 10);

  if (ok && qr==r){
    qDebug()<<"OK: expected " << r << ", got " << qr;
    return true;
  } else {
    qDebug()<<"Error: expected " << r << ", got " << qr << "(" << ok << ")";
    return true;
  }
}

QString EmacsClient::query(const QString &queryKey, const QString &queryString, QIODevice &socket){
  int returnType=Invalid;
  QString queryResult="";
  QueryResult result;
  result.key=queryKey;
  result.timestamp=QDateTime::currentDateTime();

  socket.write(queryString.toLocal8Bit());
  if (!socket.waitForBytesWritten(100)){
    qCritical()<< "Issue writing";
    return "";
  }

  // this blocks at most 30 seconds, though we're not properly handling
  // when it runs into that timeout. Also, emacs might still be locked up
  // after that.
  while (socket.waitForReadyRead(300000)){
    QByteArray byteArray=socket.readLine();
    QString message=QString::fromUtf8(byteArray.constData());
    qDebug()<< queryKey << "<" << message;
    if (message.startsWith("-print \"")){
      // String
      QRegExp rx("^-print \"(.*)\"\\s*$");
      rx.indexIn(message);
      queryResult=rx.capturedTexts().at(1);
      returnType=EmacsClient::String;
    } else if (message.startsWith("-print ")){
      // Number or lisp object
      // TODO: make it available as number as well
      QRegExp rx("^-print ([^ \\n]*)\\s*$");
      rx.indexIn(message);
      queryResult=rx.capturedTexts().at(1);
      returnType=EmacsClient::Number;
    } else if (message.startsWith("-error ")){
      QRegExp rx("^-error ([^ \\n]*)\\s*$");
      rx.indexIn(message);
      queryResult=rx.capturedTexts().at(1);
      returnType=EmacsClient::Error;
    }
  }

  queryResult.replace("&_", " ");
  queryResult.replace("&n", "\n");
  queryResult.replace("&-", "-");
  queryResult.replace("&&", "&");

  if (queryKey!=OWN_QUERY_KEY){
    switch(returnType){
      case EmacsClient::Error:
        emit queryError(queryKey, queryResult);
        break;
      case EmacsClient::String:
      case EmacsClient::Number:
        emit queryFinished(queryKey, queryResult);
    }
  }

  return queryResult;
}

QString EmacsClient::doQuery(const QString &queryKey, const QString &queryString){
  QString escapedQuery=queryString;

  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();
  escapedQuery.replace("&", "&&");
  escapedQuery.replace(" ", "&_");
  escapedQuery.replace("\n", "&n");
  //escapedQuery.replace("-", "&-");

#ifndef __ANDROID_API__
  int socketType=settings->value("core/socketType", 0).toInt();

  if (socketType==0){
    QString queryTemplate="-current-frame -eval %1\n";
    QString socketPath=settings->value("localSocket/socket", "").toString();
    QLocalSocket socket;

    socket.connectToServer(socketPath);

    if (!socket.isOpen()){
      qCritical()<< "Problem opening socket " << socketPath;
      emit queryError(queryKey, "Problem opening Emacs server connection");
      return "";
    }

    return(query(queryKey,
                 queryTemplate.arg(escapedQuery),
                 socket));
  } else {
#endif
    QString queryTemplate="-auth %1 -current-frame -eval %2\n";
    QTcpSocket socket;

    // TODO: there should be error handling with missing and invalid secrets
    QString networkSecret=settings->value("networkSocket/secret").toString();
    socket.connectToHost(
      settings->value("networkSocket/address", "127.0.0.1").toString(),
      settings->value("networkSocket/port", 60325).toInt());

    if(!socket.waitForConnected(5000)){
      qCritical()<< "Error: " << socket.errorString();
      emit queryError(queryKey, "Timeout waiting for Emacs server connection");
      return "";
    }

    return(query(queryKey,
                 (queryTemplate
                  .arg(networkSecret)
                  .arg(escapedQuery).toLocal8Bit()),
                 socket));
#ifndef __ANDROID_API__
  }
#endif
}

void EmacsClient::insertKeySequence(const QString &sequence){
  QString queryTemplate="(setq unread-command-events (listify-key-sequence \"%1\"))";
  doQuery(OWN_QUERY_KEY, queryTemplate.arg(sequence));
}

void EmacsClient::resetTimer(const bool success){
  if (success==true){
    m_emacsLastSeen=QDateTime::currentDateTime();
    emit emacsLastSeenChanged(m_emacsLastSeen);
  }
}

void EmacsClient::timeoutHandler(){

}

bool EmacsClient::queryAgent(const QString &queryKey, const QString &queryString){
  qDebug()<< queryKey << ">" << queryString;
  emit queryStarted(queryKey, queryString);
  return true;
}
