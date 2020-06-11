/**
 * @file main.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#include <QtGui>
#include <QtQuick>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QIcon>


#ifdef __ANDROID_API__
#include <QAndroidService>
#else
#include <QApplication>
#endif

// emacsbridge is the UI component, service the service. As both are started
// from the same binary/so both are required here
#include "emacsbridge.h"
#include "emacsservice.h"

const QString init(){
  QCoreApplication::setOrganizationName("Aardsoft");
  QCoreApplication::setOrganizationDomain("aardsoft.fi");
  QCoreApplication::setApplicationName("emacsbridge");
  QCoreApplication::setApplicationVersion(VERSION);

  QString dataPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir;
  if (!dir.mkpath(dataPath+"/qml"))
    qDebug()<< "Creating storage in "
            << dataPath
            << " failed. Some things will not work.";
  if (!dir.mkpath(dataPath+"/qml-staging"))
    qDebug()<< "Creating storage in "
            << dataPath
            << " failed. Some things will not work.";

  qDebug()<< "Version" << QCoreApplication::applicationVersion() << "initializing";
  return dataPath;
}

static EmacsBridge* bridgeInstance;

// TODO: If/when Qt >= 5.15 is a requirement, use qmlRegisterSingletonInstance instead.
static QJSValue bridgeSingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine){
  Q_UNUSED(engine);
  return scriptEngine->newQObject(bridgeInstance);
}

int main(int argc, char **argv){
  if (argc<=1){
#ifdef __ANDROID_API__
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif

    QString dataPath=init();

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QQuickStyle::setStyle("Material");
    QIcon::setThemeName("gallery");

    QQmlApplicationEngine engine;

    bridgeInstance = new EmacsBridge;
    qmlRegisterSingletonType("fi.aardsoft.emacsbridge", 1, 0, "EmacsBridge", bridgeSingletonProvider);

    QString mainQML=dataPath+"/qml/main.qml";
    QDir dir;
    if (!dir.exists(mainQML)){
      QFile::copy(":/qml/main.qml", mainQML);
      QFile::setPermissions(mainQML, QFile::ReadOwner|QFile::WriteOwner|QFile::ReadGroup|QFile::WriteGroup);
    }
    engine.load(mainQML);

    // Note: This doesn't allow updating the QML when it changes in qrc
    if (engine.rootObjects().isEmpty()){
      qDebug()<<"First loading failed, making sure there's a good main.qml";
      if (!QFile::remove(mainQML)){
        qDebug()<<"Unable to remove old main.qml";
      }
      engine.clearComponentCache();

      QFile::copy(":/qml/main.qml", mainQML);
      QFile::setPermissions(mainQML, QFile::ReadOwner|QFile::WriteOwner|QFile::ReadGroup|QFile::WriteGroup);
      engine.load(mainQML);
    }

    if (engine.rootObjects().isEmpty()){
      qDebug()<<"Falling back to internal qml";
      engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
      if (engine.rootObjects().isEmpty())
        return -1;
    }

    return app.exec();
  } else if (argc>1 && strcmp(argv[1], "-service")==0){
#ifdef __ANDROID_API__
    qDebug()<< "Service starting with from the same .so file";
    QAndroidService app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    init();
    EmacsService service;

    return app.exec();
  } else {
    qWarning() << "Unrecognized command line argument(s)";
    return -1;
  }
}
