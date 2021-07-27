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

/**
 * This class handles logging of messages from the Qt log functions to
 * various useful outputs. Depending on the system this may be one or
 * more of the following:
 * - stdout/stderr
 * - systemd journal
 * - Android log, readable via logcat
 * - internal ringbuffer, readable via emacsbridge -log
 *
 * This class is a singleton, and most methods are static for ease of
 * use. Additionally it is possible to inject raw log entries for parsing,
 * which generally is usable to forward messages from multiple processes
 * to a single logging instance.
 */
class EmacsBridgeLog: public QObject{
    Q_OBJECT

  public:
    /**
     * A handle for the class instance. Generally just using static methods
     * should be sufficient.
     */
    static EmacsBridgeLog *instance();
    /**
     * The Qt message handler to redirect the output of Qts log functions
     * to this class. This typically needs to be called once early on for
     * registration: `qInstallMessageHandler(EmacsBridgeLog::messageHandler);`
     */
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    /**
     * Set the size of the internal ringbuffer to @a bufferSize. The value
     * refers to the number of lines it can hold.
     *
     * Per default it is initialized at 50 lines to be able to capture early
     * log messages before it is possible to re-initialize it with a larger
     * number.
     */
    static void setBufferSize(int bufferSize);
    /**
     * @return the current size of the internal ringbuffer
     */
    static int bufferSize();
    /**
     * Inject a raw log entry. No testing if messages of the entries
     * priority should be logged is performed - the class directly
     * jumps into formatting and logging the entry.
     */
    static void injectLogEntry(const QHash<QString, QVariant> entry);
    /**
     * @return the complete internal ringbuffer as a string list
     */
    static QStringList logBuffer();
    /**
     * Set the lowest priority for which log messages should still get
     * printed. The @a priority matches syslog priorities, and defaults
     * to LOG_INFO.
     */
    static void setPriority(const int priority);
    /**
     * Configure which messages are logged to stdout/stderr. @a value
     * can take the following values:
     * |    |                                  |
     * |----|----------------------------------|
     * | -1 | no console logging at all        |
     * |  0 | only critical/fatal to stderr    |
     * |  1 | info and warnings to stdout      |
     * |  2 | debug messages to stdout as well |
     */
    static void setConsoleLogging(const int value);
    /**
     * Configure which messages are logged to the internal ringbuffer.
     * @a value can take the values as documented for @ref setConsoleLogging
     */
    static void setBufferLogging(const int value);
    /**
     * Enable or disable logging
     */
    static void setLogging(bool state);
    /**
     * Set a prefix to be logged in front of all log messages
     */
    static void setLogPrefix(const QString &prefix);
    /**
     * @return the current lowest priority for logging messages
     */
    static int priority();
    /**
     * @return the current setting for logging messages to stdout/stderr
     */
    static int consoleLogging();
    /**
     * @return the current setting for logging messages to ringbuffer
     */
    static int bufferLogging();

  private:
    EmacsBridgeLog();
    EmacsBridgeLog(const EmacsBridgeLog&);
    static EmacsBridgeLog *emacsBridgeLog;
    /**
     * Format and log an entry to various facilities. Log injection just
     * calls this method with @a injected set to `true`.
     */
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
    /// logging state. true: log false: just emit a row log entry signal
    bool m_logState;
    /// the internal ringbuffer
    QContiguousCache<QString> m_logBuffer;

  signals:
    /**
     * A string with a formatted log message is emitted when it has been
     * logged to the usual facilities
     */
    void logAdded(const QString &message);
    /**
     * An unformatted log entry is emitted if logging to regular facilities
     * is disabled. Typically a processes main class would catch that and
     * make sure a different process injects the log message into its log
     * class
     */
    void rawLogAdded(const QHash<QString, QVariant> entry);
};

#endif
