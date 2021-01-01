/**
 * @file emacsbridgelog.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EMACSBRIDGELOG_H
#define _EMACSBRIDGELOG_H

#include <QHash>
#include <QVariant>
#include <QObject>
#include <QContiguousCache>

#ifdef HAS_SYSTEMD
#include <systemd/sd-journal.h>
#endif

#include "rep_emacsbridgeremote_replica.h"

class EmacsBridgeLog: public QObject{
    Q_OBJECT

  public:
    static EmacsBridgeLog *instance();
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void setBufferSize(int bufferSize);
    static int bufferSize();
    static void injectLogEntry(const QHash<QString, QVariant> entry);
    static QStringList logBuffer();
    // priority determines if anything gets logged at all
    static void setPriority(const int priority);
    // those additional settings then determine what gets logged to specific
    // log outputs, assuming the message is in the priority to be logged
    static void setConsoleLogging(const int value);
    static void setBufferLogging(const int value);
    static void setLogging(bool state);
    static void setLogPrefix(const QString &prefix);
    static int priority();
    static int consoleLogging();
    static int bufferLogging();

  private:
    EmacsBridgeLog();
    EmacsBridgeLog(const EmacsBridgeLog&);
    static EmacsBridgeLog *emacsBridgeLog;
    void addLogEntry(const QHash<QString, QVariant> entry, bool injected=false);
    bool m_initialized=false;
    QString m_logPrefix="";
    int m_logLevel;
    /*
     * -1: no console logging at all
     *  0: only critical/fatal to stderr
     *  1: info and warnings to stdout
     *  2: debug messages to stdout as well
     */
    int m_consoleLogging;
    int m_ringBufferLogging;
#ifdef QT_DEBUG
    // filenames and lines are only defined for debug builds
    QStringList m_functionList;
    QStringList m_fileList;
    bool m_functionListIsWhitelist, m_fileListIsWhitelist;
#endif
    bool m_logState;
    QContiguousCache<QString> m_logBuffer;

  signals:
    void logAdded(const QString &message);
    void rawLogAdded(const QHash<QString, QVariant> entry);
};

#endif
