/**
 * @file emacslogger.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EMACSLOGGER_H
#define _EMACSLOGGER_H

#include <QObject>
#include <QCoreApplication>

#include "rep_emacsbridgeremote_replica.h"

class EmacsLogger: public QObject{
    Q_OBJECT

  public:
    EmacsLogger();
    ~EmacsLogger();
    void setArgumentList(const QStringList &argumentList);

  public slots:
    void run();

  private slots:
    void addLog(const QString &message);

  private:
    void help();
    QRemoteObjectNode m_repNode;
    QSharedPointer<EmacsBridgeRemoteReplica> m_rep;
    QStringList m_argumentList;

  signals:
    void done();
};

#endif
