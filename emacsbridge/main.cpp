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

void init(){
  QCoreApplication::setOrganizationName("AardSoft");
  QCoreApplication::setOrganizationDomain("aardsoft.fi");
  QCoreApplication::setApplicationName("emacsbridge");
  QCoreApplication::setApplicationVersion("0.1");
}

int main(int argc, char **argv){

  if (argc<=1){
#ifdef __ANDROID_API__
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif

    init();

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QQuickStyle::setStyle("Material");

    QQmlApplicationEngine engine;

    qmlRegisterType<EmacsBridge>("fi.aardsoft.emacsbridge",1,0,"EmacsBridge");
    engine.rootContext()->setContextProperty("emacsBridge", new EmacsBridge);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty())
      return -1;

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
