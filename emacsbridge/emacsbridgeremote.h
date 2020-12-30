/**
 * @file emacsbridgeremote.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EMACSBRIDGEREMOTE_H
#define _EMACSBRIDGEREMOTE_H

#include "rep_emacsbridgeremote_source.h"
#include "emacsclient.h"

class EmacsBridgeRemote : public EmacsBridgeRemoteSimpleSource{
    Q_OBJECT
  public:
    EmacsBridgeRemote(QObject *parent=nullptr);
  public slots:
    void setQuery(const QString &queryKey, const QString &query) override;
    void displayNotification(const QString &title, const QString &message);
    void addComponent(const QmlFileContainer &qmlFile);
    void removeComponent(const QString &qmlFile);
    QVariant serverProperty(const QString &key) override;
    void setData(const JsonDataContainer &jsonContainer);
    void setupTermux() override;
    void requestAndroidPermission(const QString &permissionName) override;
  private slots:
    void clientQueryFinished(const QString &queryKey, const QString &queryResult);
    void clientQueryError(const QString &queryKey, const QString &error);
  private:
    EmacsClient *m_client;
};

#endif
