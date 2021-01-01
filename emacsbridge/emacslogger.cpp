/**
 * @file emacslogger.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#include <QDebug>
#include "emacslogger.h"

EmacsLogger::EmacsLogger(): QObject(){
  connect(this,SIGNAL(done()),
          QCoreApplication::instance(),
          SLOT(quit()), Qt::DirectConnection);
}

EmacsLogger::~EmacsLogger(){

}

void EmacsLogger::addLog(const QString &message){
  QTextStream cout(stdout);
  cout<<message<<Qt::endl;
}

void EmacsLogger::help(){
  QTextStream cout(stdout);
  cout<<"Usage:"<<Qt::endl;
  cout<<"-d print log buffer. This is the default, but the option is required"<<Qt::endl;
  cout<<"   if other options are present as well"<<Qt::endl;
  cout<<"-h this help text"<<Qt::endl;
  cout<<"-f continuously print new log entries"<<Qt::endl;
  emit done();
}

void EmacsLogger::run(){
  if (m_argumentList.contains("-h")){
    help();
    return;
  }

  if (!m_argumentList.isEmpty() &&
      !m_argumentList.contains("-f") &&
      !m_argumentList.contains("-d")){
    emit done();
    return;
  }

  m_repNode.connectToNode(QUrl(QStringLiteral("local:replica")));
  m_rep=QSharedPointer<EmacsBridgeRemoteReplica>(m_repNode.acquire<EmacsBridgeRemoteReplica>());

  if (!m_rep->isInitialized()){
    qDebug()<<"Trying to connect to replica source...";
    bool res=m_rep->waitForSource();
    if (res==false){
      qWarning()<<"Connecting replica source failed. Is the server running?";
      emit done();
      return;
    }
  }

  QRemoteObjectPendingReply<QStringList> reply;
  reply=m_rep->logBuffer();
  reply.waitForFinished();

  QTextStream cout(stdout);
  cout<<"---"<<Qt::endl;
  cout<<reply.returnValue().join("\n")<<Qt::endl;

  if (m_argumentList.contains("-f")){
    connect(m_rep.data(), SIGNAL(logAdded(QString)),
            this, SLOT(addLog(QString)));
  } else
    emit done();
}

void EmacsLogger::setArgumentList(const QStringList &argumentList){
  m_argumentList=argumentList;
}
