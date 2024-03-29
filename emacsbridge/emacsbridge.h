/**
 * @file emacsbridge.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef EMACSBRIDGE_H
#define EMACSBRIDGE_H

#ifndef __ANDROID_API__
#include <QSystemTrayIcon>
#endif

#include <QObject>
#include <QProcess>
#include <QQmlApplicationEngine>

#include "emacsbridgetypes.h"
#include "rep_emacsbridgeremote_replica.h"

/**
 * This is the main class for the GUI process. Per default it sets up
 * a connection to the service using Qt remote objects, and exposes select
 * service APIs to QML. It is also possible to use the dummy constructor to
 * test loading of QML components without providing the full backend logic.
 */
class EmacsBridge: public QObject{
    Q_OBJECT
    Q_PROPERTY(bool configured READ isConfigured NOTIFY configuredStatusChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedStatusChanged)
    Q_PROPERTY(QDateTime emacsLastSeen READ emacsLastSeen NOTIFY emacsLastSeenChanged)
    Q_PROPERTY(bool mobile READ mobile CONSTANT)
    Q_PROPERTY(quint16 activeServerListenPort READ activeServerListenPort NOTIFY activeServerListenPortChanged)
    Q_PROPERTY(QString serverListenAddress READ serverListenAddress WRITE setServerListenAddress NOTIFY serverListenAddressChanged)
    Q_PROPERTY(quint16 serverListenPort READ serverListenPort WRITE setServerListenPort NOTIFY serverListenPortChanged)
    Q_PROPERTY(QDateTime serviceStartupTime READ serviceStartupTime CONSTANT)
    Q_PROPERTY(QDateTime startupTime READ startupTime CONSTANT)
    Q_PROPERTY(QString defaultPage READ defaultPage WRITE setDefaultPage NOTIFY defaultPageChanged)
  public:
    explicit EmacsBridge(QObject *parent=0);
    explicit EmacsBridge(const QString &dummy, QObject *parent=0){(void)dummy; (void)parent; m_isDummy=true;};
    ~EmacsBridge();

    /**
     * Copies @a text to the systems clipboard
     */
    Q_INVOKABLE void copyToClipboard(const QString &text);
    Q_INVOKABLE void runQuery(const QString &queryKey, const QString &query);
    Q_INVOKABLE void initDrawer();
    Q_INVOKABLE QVariant serverProperty(const QString &key);
    Q_INVOKABLE void copyServerProperty(const QString &key);
#ifdef __ANDROID_API__
    /**
     * Open the Android application settings
     */
    Q_INVOKABLE void openAppSettings();
    Q_INVOKABLE void callIntent(const QString &iAction, const QString &iData="",
                                const QString &iPackage="", const QString &iClass="");
    Q_INVOKABLE void requestAndroidPermission(const QString &permissionName);
    Q_INVOKABLE void setupTermux();
#endif
    bool isConnected() const;
    bool isConfigured() const;
    QDateTime emacsLastSeen() const;
    QDateTime serviceStartupTime() const;
    QDateTime startupTime() const;
    QString defaultPage() const;
    void setDefaultPage(const QString &defaultPage);
    quint16 activeServerListenPort() const;
    QString serverListenAddress() const;
    void setServerListenAddress(const QString &serverAddress) const;
    quint16 serverListenPort() const;
    void setServerListenPort(const quint16 serverPort) const;
    /**
     * Check if the application is running on a mobile device
     * @return true on Android
     * @return false on any other platform
     */
    bool mobile() const{
#ifdef __ANDROID_API__
      return true;
#else
      return false;
#endif
    };

  signals:
    void emacsLastSeenChanged(const QDateTime &lastSeen);
    void queryFinished(const QString &queryKey, const QString &queryResult);
    void queryError(const QString &queryKey, const QString &errorMessage);
    void componentAdded(const QmlFileContainer &qmlFile);
    void componentRemoved(const QString &qmlFile);
    void dataSet(const QString &requesterId, const QJSValue &jsonData);
    void defaultPageChanged();
    void activeServerListenPortChanged(quint16 serverPort);
    void serverListenAddressChanged(QString &serverAddress);
    void serverListenPortChanged(quint16 serverPort);
    void connectedStatusChanged(const bool status);
    void configuredStatusChanged(const bool status);

  private slots:
#ifndef __ANDROID_API__
    void updateNotification(const QString &title, const QString &message);
#endif
    void updateQueryResult(const QString &queryKey, const QString &result);
    void addComponent(const QmlFileContainer &qmlFile);
    void removeComponent(const QString &qmlFile);
    void setData(const JsonDataContainer &jsonContainer);

  private:
    /// The startup time of the UI part
    QDateTime m_startupTime;
#ifndef __ANDROID_API__
    QSystemTrayIcon *m_trayIcon;
#endif
    QRemoteObjectNode m_repNode;
    QSharedPointer<EmacsBridgeRemoteReplica> m_rep;
    QQmlApplicationEngine *m_engine;
    bool m_isDummy=false;
    QStringList m_drawerComponents;
};

#endif
