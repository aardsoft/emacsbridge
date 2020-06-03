/**
 * @file emacsbridgesettings.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EMACSBRIDGESETTINGS_H
#define _EMACSBRIDGESETTINGS_H

#include <QSettings>
#include <QObject>

class EmacsBridgeSettings: public QSettings{
    Q_OBJECT

  public:
    static EmacsBridgeSettings *instance();
    void setValue(const QString &key, const QVariant &value);

  signals:
    void settingChanged(const QString &key);

  private:
    EmacsBridgeSettings();
    EmacsBridgeSettings(const EmacsBridgeSettings&);
    static EmacsBridgeSettings *emacsBridgeSettings;
    bool m_initialized=false;
};
#endif
