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
#include <QJsonObject>

#include <QAmbientLightSensor>
#ifdef _WITH_PROXIMITY
// proximit seems to be less useful than ambient, so make it a compile time flag
// for now
#include <QProximitySensor>
#endif

#include "emacsbridgetypes.h"
#include "emacsbridgemorse.h"

/**
 * This class provides the HTTP server to serve documentation pages,
 * allow initial configuration from Emacs side, and provide the RPC
 * endpoints used from Emacs.
 *
 */
class EmacsServer: public QObject{
    Q_OBJECT

  public:
    EmacsServer();
    ~EmacsServer();

  public slots:
    /**
     * Setup everything required for the server to work properly, and start
     * it. This is a separate function to allow easy server restarts and to
     * make sure it runs in a separate thread.
     */
    void startServer();
    /**
     * Restart the web server after configuration has changed. This just
     * destroys the internal HTTP server object, and then calls startServer()
     */
    void restartServer();
    /**
     * @return the current server listen port, or 0
     *
     * The server is supposed to listen to only one port. If something went
     * went wrong and the server listens to more than one the first listen
     * port is returned. In this case, and if the server is not listening at
     * all a critical log message is logged.
     */
    quint16 activeServerPort();
#ifdef __ANDROID_API__
    /**
     * Call a JSON encoded Android intent through callIntentFromJson() on Java
     * side in EmacsBridgeService.
     *
     * @return OK on success, Java exception string on error
     */
    QString callIntent(const QString &jsonString);
    /**
     * Look up the permissions required for @a intentClass, check if those are
     * granted, and request the permission dialog from the UI if they are not.
     *
     * An Android service is not allowed to request permissions, so all we can
     * do here is check if permissions have been granted already. If the result
     * is "denied" the UI is triggered to request permission - but to avoid
     * blocking this function returns without waiting for the result.
     *
     * The permission may not be granted, or be granted at a later time (and
     * possibly revoked again), so code should handle missing permissions
     * gracefully. If a permission was denied code should continue without
     * those permissions, if possible. If it is not possible a short wait and
     * several retries are an option before aborting.
     *
     * The following classes are currently supported:
     *
     * | Class                            | Permission                        |
     * |----------------------------------|-----------------------------------|
     * | com.termux.app.RunCommandService | com.termux.permission.RUN_COMMAND |
     *
     * Note: Currently the implementation can only request a single permission
     * per class name.
     *
     * @return "" on success, or the denied permission name
     */
    QString checkPermissions(const QString &intentClass);
#endif

  private:
    QHttpServer *m_server;
    QDateTime m_startupTime;
    QString m_htmlTemplate;
    QString m_dataPath;
    EmacsBridgeMorse *m_morseInterpreter;

    /**
     * Check if the provided string is a valid auth token.
     *
     * @return true if valid, false if invalid
     */
    bool validAuthToken(const QString &token);
    /**
     * Start up the actual HTTP server. This creates a new QHttpServer in
     * m_server, and sets up all the mappings. If possible this also does
     * auth token checking - though not for all endpoints it's clear if
     * an auth token is required before evaluating the messag content.
     *
     * For simple endpoints the implementation is in here as well, though
     * some of that should get split out for simplicity.
     */
    void startHttpServer();
    /**
     * List the directory given in @a directory.
     *
     * @return a string containing a HTML page with the directory listing
     */
    QString listDirectory(const QString &directory);
    /**
     * Check which RPC method to call, verify data, and call the method if
     * everything is correct.
     *
     * This got split out from the /rpc mapping of startHttpServer() to
     * simplify the code.
     *
     * This function does some basic validation of the provided JSON data,
     * and calls the endpoint specific method with verified data. It then
     * returns the return value of that method call.
     *
     * @return QHttpServerResponder::StatusCode::BadRequest on error or
     *         the return value of the endpoint method call
     */
    QHttpServerResponse methodCall(const QString &method, const QByteArray &payload);
    /**
     * Method for handling /settings mapping of startHttpServer().
     *
     * Currently the only settings supported are @a server-key and
     * @a server-socket. Those two settings are provided wit the key
     * in the header, and the complete POST containing the data, without
     * any padding.
     *
     * Code for parsing additional key/value pairs fom the POST body
     * is present, but not yet hooked up to the settings backend.
     *
     * @return QHttpServerResponder::StatusCode::BadRequest on error.
     *         Both the error and success messages are designed to be
     *         exposed to the Emacs UI and are translatable.
     */
    QHttpServerResponse settingCall(const QString &setting, const QByteArray &payload);
    /**
     * Verify the data, and add a new QML component to the application if
     * the provided data is valid. Note that overwriting an existing component
     * typically requires a restart of the applications due to shortcomings
     * in the QML garbage collector.
     *
     * @return QHttpServerResponder::StatusCode::BadRequest on error.
     *         Both the error and success messages are designed to be
     *         exposed to the Emacs UI and are translatable.
     */
    QHttpServerResponse addComponent(const QJsonObject &jsonObject);
    /**
     * Display the provided notification via platform specific UI.
     *
     * @return OK. Most notification backends don't provide much error
     *         error handling, so the assumption is always that the
     *         call succeeded.
     */
    QHttpServerResponse addNotification(const QJsonObject &jsonObject);
#ifdef __ANDROID_API__
    /**
     * Do a validity check on the provided data before executing the intent
     * through callIntent()
     *
     * @return QHttpServerResponder::StatusCode::BadRequest on error.
     *         Both the error and success messages are designed to be
     *         exposed to the Emacs UI and are translatable.
     */
    QHttpServerResponse handleIntent(const QJsonObject &jsonObject,
                                     const QString &jsonString);
#endif
    /**
     * Dummy method supposed to handle sensor control in the future
     */
    QHttpServerResponse handleSensorCall(const QJsonObject &jsonObject);
    /**
     * Open a file (from filesystem or resource) and replace all valid keys
     * in template notation ({{key}}) with their values.
     *
     * @return QHttpServerResponder::StatusCode::BadRequest on error or
     *         the templated file on success. Both the error and success
     *         messages are designed to be exposed to the Emacs UI and are
     *         translatable.
     */
    QHttpServerResponse parseFile(const QString &fileName);
    /**
     * Verify the data, and remove a QML component from the application if
     * the provided data is valid. Note that overwriting an existing component
     * typically requires a restart of the applications due to shortcomings
     * in the QML garbage collector.
     *
     * @return QHttpServerResponder::StatusCode::BadRequest on error.
     *         Both the error and success messages are designed to be
     *         exposed to the Emacs UI and are translatable.
     */
    QHttpServerResponse removeComponent(const QJsonObject &jsonObject);
    /**
     * Verify if incoming data as a response to a request triggered through
     * elisp contains a requester ID, and then throw the JSON data up to the
     * UI. No checks if the data itself is useful for the application is
     * possible here.
     *
     * @return QHttpServerResponder::StatusCode::BadRequest on error.
     *         Both the error and success messages are designed to be
     *         exposed to the Emacs UI and are translatable.
     */
    QHttpServerResponse setData(const QJsonObject &jsonObject, const QString &jsonString);
    // sensors
    QAmbientLightSensor *m_ambientLightSensor;
#ifdef _WITH_PROXIMITY
    QProximitySensor *m_proximitySensor;
#endif

  private slots:

  signals:
    void componentAdded(const QmlFileContainer &qmlFile);
    void componentRemoved(const QString &qmlFile);
    void dataSet(const JsonDataContainer &jsonData);
    void notificationAdded(const QString &title, const QString &message);
    void activePortChanged(const quint16 port);
    void androidPermissionDenied(const QString &permission);
    void parseMorse(int level);
};

#endif
