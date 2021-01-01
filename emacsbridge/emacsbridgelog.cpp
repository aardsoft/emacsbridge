/**
 * @file emacsbridgelog.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2020
 */

#include <syslog.h>
#include "emacsbridgelog.h"

#ifdef __ANDROID_API__
#include <android/log.h>
const char* appname="emacsbridge";
#endif

EmacsBridgeLog *EmacsBridgeLog::emacsBridgeLog=0;

EmacsBridgeLog *EmacsBridgeLog::instance(){
  if (!emacsBridgeLog){
    emacsBridgeLog=new EmacsBridgeLog();
    // set sensible buffer size to capture early log messages
    emacsBridgeLog->m_logBuffer.setCapacity(50);
    //emacsBridgeLog->m_logLevel=LOG_INFO;
    emacsBridgeLog->m_logLevel=LOG_DEBUG;
    emacsBridgeLog->m_consoleLogging=2;
    emacsBridgeLog->m_ringBufferLogging=2;
    emacsBridgeLog->m_logState=true;
#ifdef QT_DEBUG
    emacsBridgeLog->m_fileListIsWhitelist=false;
    emacsBridgeLog->m_functionListIsWhitelist=false;
#endif
  }

  return emacsBridgeLog;
}

EmacsBridgeLog::EmacsBridgeLog(){
}

void EmacsBridgeLog::addLogEntry(const QHash<QString, QVariant> entry, bool injected){
  if (m_logState==false){
    emit rawLogAdded(entry);
    return;
  }

  QString message=QString("%1%2%3\t%4%5")
    .arg(m_logPrefix)
    .arg(entry["token"].toString())
    .arg(injected?"i":" ")
    .arg(entry["marker"].toString())
    .arg(entry["message"].toString());

  QByteArray localMessage=message.toLocal8Bit();

#ifdef HAS_SYSTEMD
  sd_journal_print(entry["severity"].toInt(), "%s", localMessage.constData());
#endif
#ifdef __ANDROID_API__
  __android_log_write(entry["androidSeverity"].toInt(),
                      appname,
                      localMessage.constData());
#endif
  // TODO: check log level for console/ringbuffer
  //if (m_consoleLogging>=2)
  fprintf(stdout, "%s\n", localMessage.constData());
  m_logBuffer.append(message);

  emit logAdded(message);
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

void EmacsBridgeLog::injectLogEntry(const QHash<QString, QVariant> entry){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  _instance->addLogEntry(entry, true);
}

QStringList EmacsBridgeLog::logBuffer(){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  QStringList ret;

  for (int i=_instance->m_logBuffer.firstIndex();
       i<=_instance->m_logBuffer.lastIndex();
       i++){
    ret.append(_instance->m_logBuffer.at(i));
  }
  return ret;
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

void EmacsBridgeLog::setLogPrefix(const QString &prefix){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  _instance->m_logPrefix=prefix;
}

void EmacsBridgeLog::setPriority(const int priority){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  _instance->m_logLevel=priority;
}

void EmacsBridgeLog::setLogging(bool state){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  _instance->m_logState=state;
}

void EmacsBridgeLog::messageHandler(QtMsgType type,
                                    const QMessageLogContext &context,
                                    const QString &msg){
  EmacsBridgeLog *_instance=EmacsBridgeLog::instance();
  QHash<QString, QVariant> logEntry;

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
  logEntry.insert("message", msgWithFile);
#else
  logEntry.insert("message", msg);
#endif

  // not all of those message types may have the best possible
  // syslog type. Unused: ALERT ERR
  switch(type){
    case QtDebugMsg:
      if (_instance->m_logLevel>=LOG_DEBUG){
        logEntry.insert("token", "[DEBUG]");
        logEntry.insert("severity", LOG_DEBUG);
#ifdef __ANDROID_API__
        logEntry.insert("androidSeverity", ANDROID_LOG_DEBUG);
#endif
        _instance->addLogEntry(logEntry);
      }
      break;
    case QtWarningMsg:
      if (_instance->m_logLevel>=LOG_WARNING){
        logEntry.insert("token", "[WARN ]");
        logEntry.insert("severity", LOG_WARNING);
#ifdef __ANDROID_API__
        logEntry.insert("androidSeverity", ANDROID_LOG_WARN);
#endif
        _instance->addLogEntry(logEntry);
      }
      break;
    case QtCriticalMsg:
      // QtSystemMsg is defined as QtCritical
      if (_instance->m_logLevel>=LOG_CRIT){
        logEntry.insert("token", "[ERROR]");
        logEntry.insert("severity", LOG_CRIT);
#ifdef __ANDROID_API__
        logEntry.insert("androidSeverity", ANDROID_LOG_ERROR);
#endif
        _instance->addLogEntry(logEntry);
      }
      break;
    case QtFatalMsg:
      if (_instance->m_logLevel>=LOG_EMERG){
        logEntry.insert("token", "[EMERG]");
        logEntry.insert("severity", LOG_EMERG);
#ifdef __ANDROID_API__
        logEntry.insert("androidSeverity", ANDROID_LOG_ERROR);
#endif
        _instance->addLogEntry(logEntry);
      }
      break;
    case QtInfoMsg:
      if (_instance->m_logLevel>=LOG_INFO){
        logEntry.insert("token", "[INFO ]");
        logEntry.insert("severity", LOG_INFO);
#ifdef __ANDROID_API__
        logEntry.insert("androidSeverity", ANDROID_LOG_INFO);
#endif
        _instance->addLogEntry(logEntry);
      }
      break;
  }
}
