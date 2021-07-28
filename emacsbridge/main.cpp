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
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <jni.h>
#else
#include <QApplication>
#endif

// emacsbridge is the UI component, service the service. As both are started
// from the same binary/so both are required here
#include "emacsbridge.h"
#include "emacsservice.h"
#include "emacsbridgelog.h"
#include "emacslogger.h"


#ifdef __ANDROID_API__
/**
 * critical binding for android
 */
static void jCritical(JNIEnv *env, jobject obj, jstring msg){
  qCritical()<<"[j]"<<env->GetStringUTFChars(msg, 0);
}

static void jDebug(JNIEnv *env, jobject obj, jstring msg){
  qDebug()<<"[j]"<<env->GetStringUTFChars(msg, 0);
}

static void jInfo(JNIEnv *env, jobject obj, jstring msg){
  qInfo()<<"[j]"<<env->GetStringUTFChars(msg, 0);
}

static void jWarning(JNIEnv *env, jobject obj, jstring msg){
  qWarning()<<"[j]"<<env->GetStringUTFChars(msg, 0);
}
#endif

void initAppData(){
  QCoreApplication::setOrganizationName("Aardsoft");
  QCoreApplication::setOrganizationDomain("aardsoft.fi");
  QCoreApplication::setApplicationName("emacsbridge");
  QCoreApplication::setApplicationVersion(VERSION);
}

const QString init(){
  qInstallMessageHandler(EmacsBridgeLog::messageHandler);

  initAppData();

#ifdef __ANDROID_API__
  static JNINativeMethod methods[]={
    {"jCritical", "(Ljava/lang/String;)V", (void *)jCritical},
    {"jDebug", "(Ljava/lang/String;)V", (void *)jDebug},
    {"jInfo", "(Ljava/lang/String;)V", (void *)jInfo},
    {"jWarning", "(Ljava/lang/String;)V", (void *)jWarning},
  };

  QAndroidJniObject jClass("fi/aardsoft/emacsbridge/Log");
  QAndroidJniEnvironment env;
  jclass objectClass = env->GetObjectClass(jClass.object<jobject>());
  env->RegisterNatives(objectClass,
                       methods,
                       sizeof(methods) / sizeof(methods[0]));
  env->DeleteLocalRef(objectClass);
#endif

  QString dataPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir;
  if (!dir.mkpath(dataPath+"/qml"))
    qCritical()<< "Creating storage in "
            << dataPath
            << " failed. Some things will not work.";
  if (!dir.mkpath(dataPath+"/qml-staging"))
    qCritical()<< "Creating storage in "
            << dataPath
            << " failed. Some things will not work.";

  qInfo()<< "Version" << QCoreApplication::applicationVersion() << "initializing";
  return dataPath;
}

static EmacsBridge* bridgeInstance;

// TODO: If/when Qt >= 5.15 is a requirement, use qmlRegisterSingletonInstance instead.
static QJSValue bridgeSingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine){
  Q_UNUSED(engine);
  return scriptEngine->newQObject(bridgeInstance);
}

/**
 * Blabla foo main entry
 */
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
      qInfo()<<"First loading failed, making sure there's a good main.qml";
      if (!QFile::remove(mainQML)){
        qWarning()<<"Unable to remove old main.qml";
      }
      engine.clearComponentCache();

      QFile::copy(":/qml/main.qml", mainQML);
      QFile::setPermissions(mainQML, QFile::ReadOwner|QFile::WriteOwner|QFile::ReadGroup|QFile::WriteGroup);
      engine.load(mainQML);
    }

    if (engine.rootObjects().isEmpty()){
      qInfo()<<"Falling back to internal qml";
      engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
      if (engine.rootObjects().isEmpty())
        return -1;
    }

    return app.exec();
  } else if (argc>1 && strcmp(argv[1], "-service")==0){
#ifdef __ANDROID_API__
    qInfo()<< "Service starting with from the same .so file";
    QAndroidService app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif
    init();
    EmacsService service;

    return app.exec();
  } else if (argc>1 && strcmp(argv[1], "-log")==0){
#ifdef __ANDROID_API__
    //TODO
    QCoreApplication app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif
    initAppData();

    QStringList argumentList;

    if (argc>2)
      for (int i=2;i<argc;i++){
        argumentList.append(argv[i]);
      }
    EmacsLogger log;
    log.setArgumentList(argumentList);
    QTimer::singleShot(0, &log, SLOT(run()));

    return app.exec();
  } else {
    qWarning() << "Unrecognized command line argument(s)";
    return -1;
  }
}
