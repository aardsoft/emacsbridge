/**
 * @file emacsbridgemorse.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#include "emacsclient.h"
#include "emacsbridgemorse.h"

EmacsBridgeMorse::EmacsBridgeMorse(): QObject(){
  m_dict.insert(".-", "a");
  m_dict.insert("-...", "b");
  m_dict.insert("-.-.", "c");
  m_dict.insert("-..", "d");
  m_dict.insert(".", "e");
  m_dict.insert("..-.", "f");
  m_dict.insert("--.", "g");
  m_dict.insert("....", "h");
  m_dict.insert("..", "i");
  m_dict.insert(".---", "j");
  m_dict.insert("-.-", "k");
  m_dict.insert(".-..", "l");
  m_dict.insert("--", "m");
  m_dict.insert("-.", "n");
  m_dict.insert("---", "o");
  m_dict.insert(".--.", "p");
  m_dict.insert("--.-", "q");
  m_dict.insert(".-.", "r");
  m_dict.insert("...", "s");
  m_dict.insert("-", "t");
  m_dict.insert("..-", "u");
  m_dict.insert("...-", "v");
  m_dict.insert(".--", "w");
  m_dict.insert("-..-", "x");
  m_dict.insert("-.--", "y");
  m_dict.insert("--..", "z");
  m_dict.insert(".----", "1");
  m_dict.insert("..---", "2");
  m_dict.insert("...--", "3");
  m_dict.insert("....-", "4");
  m_dict.insert(".....", "5");
  m_dict.insert("-....", "6");
  m_dict.insert("--...", "7");
  m_dict.insert("---..", "8");
  m_dict.insert("----.", "9");
  m_dict.insert("-----", "0");
  m_dict.insert(".-.-.-", ".");
  m_dict.insert("--..--", ",");
  m_dict.insert("..--..", "?");
  m_dict.insert(".----.", "'");
  m_dict.insert("-.-.--", "!");
  m_dict.insert("-..-.", "/");
  m_dict.insert("-.--.", "(");
  m_dict.insert("-.--.-", ")");
  m_dict.insert(".-...", "&");
  m_dict.insert("---...", ":");
  m_dict.insert("-.-.-.", ";");
  m_dict.insert("-...-", "=");
  m_dict.insert(".-.-.", "+");
  m_dict.insert("-....-", "-");
  m_dict.insert("..--.-", "_");
  m_dict.insert(".-..-.", "\"");
  m_dict.insert("..._.._", "$");
  m_dict.insert(".--.-.", "@");

  m_endTimer=new QTimer();
  m_endTimer->setSingleShot(true);

  connect(m_endTimer, SIGNAL(timeout()),
          this, SLOT(timeoutHandler()));
}

EmacsBridgeMorse::~EmacsBridgeMorse(){
}

void EmacsBridgeMorse::setLowState(int state){
  m_lowState=state;
}

void EmacsBridgeMorse::setHighState(int state){
  m_highState=state;
}

void EmacsBridgeMorse::stateChange(int state){

  // nothing changed, so ignore
  if (state==m_oldState)
    return;

  /**
   * @TODO we can also check here if the timer is active, for valid inputs it
   *       should be active except for the very first state change
   */
  //bool timerWasActive=m_endTimer->isActive();
  m_endTimer->stop();

  if (state>m_lowState){
    // high
    if (m_oldState==m_lowState){
      qint64 elapsedTime=m_stateTimer.restart();
      if (elapsedTime<=m_shortMax){
        qDebug()<<"Transitioned from low to high in"
                <<elapsedTime
                <<"registering .";
        currentInput+=".";
      } else {
        qDebug()<<"Transitioned from low to high in"
                <<elapsedTime
                <<"registering -";
        currentInput+="-";
      }
    } else {
      qDebug()<<"Transition to high from invalid state"
              <<m_stateTimer.restart();
    }
    m_oldState=m_highState;
  } else {
    // low
    // rules out invalid -1 states
    if (m_oldState>m_lowState){
      qDebug()<<"Transitioned from high to low in"
              <<m_stateTimer.restart();
    } else {
      qDebug()<<"Transition to low from invalid state";
      m_stateTimer.restart();
    }
    m_oldState=m_lowState;
  }
  m_endTimer->start(m_longMax);
}

void EmacsBridgeMorse::timeoutHandler(){
  qDebug()<<"Timeout timer";
  if (m_oldState==m_highState){
    if (m_dict.contains(currentInput)){
      EmacsClient *client=EmacsClient::instance();
      qInfo()<<"Identified"<<m_dict[currentInput];
      client->insertKeySequence(m_dict[currentInput]);
    } else {
      qInfo()<<currentInput<<"not found in dict";
    }
  } else {
    qInfo()<<"Aborting invalid attempt";
  }
  currentInput="";
}
