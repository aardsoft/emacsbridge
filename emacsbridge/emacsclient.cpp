/**
 * @file emacsclient.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

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
EmacsClient::EmacsClient(): QThread(), m_exitThread(false), m_queryActive(false){
  connect(this, SIGNAL(queryStarted(QString)), this, SLOT(doQuery(QString)), Qt::DirectConnection);

  start();
}

EmacsClient::~EmacsClient(){
  qDebug()<< "Destroying EmacsClient instance...";
  m_exitThread=true;
  quit();
  wait();
  qDebug()<< "...done";
}

bool EmacsClient::isSetup(){
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();
  QString queryTemplate="(customize-save-variable 'emacsbridge-auth-token \"%1\")";
  QString authToken=settings->value("core/auth-token").toString();

  if (authToken=="")
    return false;

  QString stringResult=doQuery(queryTemplate.arg(authToken));

  if (stringResult==authToken){
    qDebug()<< authToken << "==" << stringResult;
    return true;
  } else {
    qDebug()<< stringResult;
    return false;
  }
}

bool EmacsClient::isConnected(){
  quint32 a=QRandomGenerator::global()->bounded(256);
  quint32 b=QRandomGenerator::global()->bounded(256);
  quint32 r=a+b;
  QString queryTemplate="(+ %1 %2)";

  QString stringResult=doQuery(queryTemplate.arg(a).arg(b), true);

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

QString EmacsClient::doQuery(const QString &query, bool ownQuery){
  QString escapedQuery=query;
  qDebug()<< "Executing query: " << query;
  m_queryActive=false;

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
      qDebug()<< "Problem opening socket " << socketPath;
      return "";
    }

    socket.write(queryTemplate.arg(escapedQuery).toLocal8Bit());
    if (!socket.waitForBytesWritten(100)){
      qDebug()<< "Issue writing";
      return "";
    }

    while (socket.waitForReadyRead()){
      QByteArray byteArray=socket.readLine();
      QString message=QString::fromUtf8(byteArray.constData());
      qDebug()<< message;
      if (message.startsWith("-print \"")){
        // String
        QRegExp rx("^-print \"(.*)\"\\s*$");
        rx.indexIn(message);
        m_queryResult=rx.capturedTexts().at(1);
      } else if (message.startsWith("-print ")){
        // Number or lisp object
        // TODO: make it available as number as well
        QRegExp rx("^-print ([^ \\n]*)\\s*$");
        rx.indexIn(message);
        m_queryResult=rx.capturedTexts().at(1);
      }
    }
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
      qDebug()<< "Error: " << socket.errorString();
      return "";
    }

    qDebug() << "Query: " << queryTemplate
      .arg(networkSecret)
      .arg(escapedQuery).toLocal8Bit();
    socket.write(queryTemplate
                 .arg(networkSecret)
                 .arg(escapedQuery).toLocal8Bit());

    if (!socket.waitForBytesWritten(100)){
      qDebug()<< "Issue writing";
      return "";
    }

    while (socket.waitForReadyRead()){
      QByteArray byteArray=socket.readLine();
      QString message=QString::fromUtf8(byteArray.constData());
      qDebug()<< message;
      if (message.startsWith("-print \"")){
        // String
        QRegExp rx("^-print \"(.*)\"\\s*$");
        rx.indexIn(message);
        m_queryResult=rx.capturedTexts().at(1);
      } else if (message.startsWith("-print ")){
        // Number or lisp object
        // TODO: make it available as number as well
        QRegExp rx("^-print ([^ \\n]*)\\s*$");
        rx.indexIn(message);
        m_queryResult=rx.capturedTexts().at(1);
      }
    }

#ifndef __ANDROID_API__
  }
#endif

  m_queryResult.replace("&_", " ");
  m_queryResult.replace("&n", "\n");
  m_queryResult.replace("&-", "-");
  m_queryResult.replace("&&", "&");

  if (!ownQuery)
    emit queryFinished("", m_queryResult);

  return m_queryResult;
}

bool EmacsClient::queryAgent(const QString &query){
  // we only allow one query at a time.
  // That wasn't an issue with the original design, but might be now.
  if (m_queryActive)
    return false;

  m_queryString=query;
  m_queryActive=true;
  emit queryStarted(m_queryString);
  return true;
}

QString EmacsClient::queryResult() const{
  return m_queryResult;
}
