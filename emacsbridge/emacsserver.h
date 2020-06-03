/**
 * @file emacsserver.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EMACSSERVER_H
#define _EMACSSERVER_H

#include <QtCore>
#include <QHttpServer>

class EmacsServer: public QThread{
    Q_OBJECT

  public:
    EmacsServer();
    ~EmacsServer();

  public slots:

  private:
    QHttpServer m_server;
    QDateTime m_startupTime;

  private slots:

  signals:

};

#endif
