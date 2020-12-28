/**
 * @file emacsbridgelog.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2020
 */

#include <syslog.h>
#include "emacsbridgelog.h"

EmacsBridgeLog *EmacsBridgeLog::emacsBridgeLog=0;

EmacsBridgeLog *EmacsBridgeLog::instance(){
  if (!emacsBridgeLog){
    emacsBridgeLog=new EmacsBridgeLog();
    //emacsBridgeLog->m_logLevel = LOG_INFO;
    emacsBridgeLog->m_logLevel = LOG_DEBUG;
    emacsBridgeLog->m_consoleLogging = 2;
    emacsBridgeLog->m_ringBufferLogging = 2;
#ifdef QT_DEBUG
    emacsBridgeLog->m_fileListIsWhitelist = false;
    emacsBridgeLog->m_functionListIsWhitelist = false;
#endif
  }

  return emacsBridgeLog;
}

EmacsBridgeLog::EmacsBridgeLog(){
}

int EmacsBridgeLog::bufferLogging(){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  return _instance->m_ringBufferLogging;
}

int EmacsBridgeLog::bufferSize(){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  return _instance->m_logBuffer.capacity();
}

int EmacsBridgeLog::consoleLogging(){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  return _instance->m_consoleLogging;
}

int EmacsBridgeLog::priority(){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  return _instance->m_logLevel;
}

void EmacsBridgeLog::setBufferLogging(const int value){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  _instance->m_ringBufferLogging=value;
}

void EmacsBridgeLog::setBufferSize(int bufferSize){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  _instance->m_logBuffer.setCapacity(bufferSize);
}

void EmacsBridgeLog::setConsoleLogging(const int value){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  _instance->m_consoleLogging=value;
}

void EmacsBridgeLog::setPriority(const int priority){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  _instance->m_logLevel=priority;
}

void EmacsBridgeLog::messageHandler(QtMsgType type,
                                    const QMessageLogContext &context,
                                    const QString &msg){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();

#ifdef QT_DEBUG
  const char *file=context.file?context.file:"";
  const char *function=context.function?context.function:"";

  // filter based on function white/blacklisting
  if (_instance->m_functionList.contains(file) &&
      _instance->m_functionListIsWhitelist==false)
    return;
  if (!_instance->m_functionList.contains(file) &&
      _instance->m_functionListIsWhitelist==true)
    return;

  // filter based on file white/blacklisting
  if (_instance->m_fileList.contains(file) &&
      _instance->m_fileListIsWhitelist==false)
    return;
  if (!_instance->m_fileList.contains(file) &&
      _instance->m_fileListIsWhitelist==true)
    return;

  QString msgWithFile=QString("%1 (%2:%3)")
    .arg(msg)
    .arg(file)
    .arg(function);
  QByteArray localMsg=msgWithFile.toLocal8Bit();
#else
  QByteArray localMsg=msg.toLocal8Bit();
#endif

  // not all of those message types may have the best possible
  // syslog type. Unused: ALERT ERR
  switch(type){
    case QtDebugMsg:
      if (_instance->m_logLevel>=LOG_DEBUG){
#ifdef HAS_SYSTEMD
        sd_journal_print(LOG_DEBUG, "%s", localMsg.constData());
        if (_instance->m_consoleLogging>=2)
#endif
          fprintf(stdout, "[DEBUG] %s\n", localMsg.constData());
      }
      break;
    case QtWarningMsg:
      if (_instance->m_logLevel>=LOG_WARNING){
#ifdef HAS_SYSTEMD
        sd_journal_print(LOG_WARNING, "%s", localMsg.constData());
        if (_instance->m_consoleLogging>=1)
#endif
          fprintf(stdout, "[WARN] %s\n", localMsg.constData());
      }
      break;
    case QtCriticalMsg:
      // QtSystemMsg is defined as QtCritical
      if (_instance->m_logLevel>=LOG_CRIT){
#ifdef HAS_SYSTEMD
        sd_journal_print(LOG_CRIT, "%s", localMsg.constData());
        if (_instance->m_consoleLogging>=0)
#endif
          fprintf(stderr, "[CRIT] %s\n", localMsg.constData());
      }
      break;
    case QtFatalMsg:
      if (_instance->m_logLevel>=LOG_EMERG){
#ifdef HAS_SYSTEMD
        sd_journal_print(LOG_EMERG, "%s", localMsg.constData());
        if (_instance->m_consoleLogging>=0)
#endif
          fprintf(stderr, "[FATAL] %s\n", localMsg.constData());
      }
      break;
    case QtInfoMsg:
      if (_instance->m_logLevel>=LOG_INFO){
#ifdef HAS_SYSTEMD
        sd_journal_print(LOG_INFO, "%s", localMsg.constData());
        if (_instance->m_consoleLogging>=1)
#endif
          fprintf(stdout, "[INFO] %s\n", localMsg.constData());
      }
      break;
  }
}
