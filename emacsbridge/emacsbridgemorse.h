/**
 * @file emacsbridgemorse.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EMACSBRIDGEMORSE_H
#define _EMACSBRIDGEMORSE_H

#include <QtCore>

class EmacsBridgeMorse: public QObject{
    Q_OBJECT

  public:
    EmacsBridgeMorse();
    ~EmacsBridgeMorse();
    void setLowState(int state);
    void setHighState(int state);

  public slots:
    void stateChange(int state);

  private slots:
    void timeoutHandler();

  private:
    QElapsedTimer m_stateTimer;
    QTimer *m_endTimer;
    QHash<QString, QString> m_dict;
    QString currentInput="";
    int m_highState=3;
    int m_lowState=1;
    int m_oldState;
    // this tends to be <150, though in rare cases system lag seems to bring it
    // to over 200 (with a light sensor). Wait for full implementation and error
    // rate.
    int m_shortMax=200;
    // timeout which is treated as "end of character"
    int m_longMax=700;
    // TODO:
    // - space handling is missing
    // - try to filter out some of the false inputs, especially when it's rather
    //   dark in the room

  signals:
    void keys(QString &key);
};

#endif
