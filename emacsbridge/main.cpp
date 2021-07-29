/**
 * @file main.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

/** @mainpage
 * @tableofcontents
 * @section working_with_core Working with the core
 * @subsection qt_requirementsQt requirements
 * The following table contains the Qt versions suitable for compiling
 *  emacsbridge:
 *
 * | OS      | Version | State                                       |
 * |---------|---------|---------------------------------------------|
 * | Android |  5.15.1 | recommended                                 |
 * | Android |  5.15.0 | minimum                                     |
 * | PC      |  5.14.x | minimum                                     |
 * | PC      |  5.15.x | recommended                                 |
 * | PC      |  5.13.0 | compiles, works mostly, but not recommended |
 *
 *
 * For building on PC compiling Qt is straightforward, and your distribution
 * might already have a correct version. The following packages should pull in
 * all the other required dependencies for building:
 *
 * - Qt5Quick
 * - Qt5QuickControls2
 * - Qt5WebSockets
 * - Qt5RemoteObjects
 * - Qt5Network
 * - Qt5Sensors
 * - libQt5Core-private-headers-devel
 *
 * The package names are for OpenSuSE, other distributions should have similar
 * naming.
 *
 * To build for Android an Android development environment needs to be set up.
 * It should work with Android Studio or similar, below is a summary on how to
 * setup a non-GUI build environment.
 *
 * @subsection preparing_sources Preparing the sources
 * Emacsbridge requires qthttpserver, which is included as a submodule.
 * qthttpserver also has submodules. The following snippet clones the repository
 *  and initializes all submodules:
 *
 * @code{.sh}
 * $ git clone https://github.com/aardsoft/emacsbridge.git
 * $ cd emacsbridge
 * $ git submodule update --init --recursive qthttpserver
 * @endcode
 *
 * Now the code can be configured for qmake - for Android the qmake from inside
 * the Android Qt tree needs to be called.
 *
 * @subsection buildhelper Build helper
 * The source repository contains a build helper (<code>build.sh</code>). Outside
 * of simple Linux builds using that might be easiest.
 *
 * Variables controlling the build include:
 * - ANDROID_ABIS: ABIs to build, default <code>arm64-v8a x86_64 armeabi-v7a</code>
 * - ANDROID_SDK_VERSION: Android SDK version to use, default @c android-29.
 *   When targetting Play store the version from the script generally is the
 *   one which has to be used.
 * - QT_VERSION: The Qt version used for trying to find directories, default @c 5.15.1
 * - QT_ANDROID_BIN: The bin directory inside of the Qt Android installation,
 *   defaulting to @c $HOME/qt/qt${QT_VERSION}-${ANDROID_SDK_VERSION}/bin
 * - QT_WINDOWS_BIN: The bin directory inside of the Windows Qt installation,
 *   defaulting to @c $HOME/qt/qt${QT_VERSION}-mingw64/bin
 * - WIN32_OBJDUMP: The name of the objdump binary for Windows, default
 *   @c x86_64-w64-mingw32-objdump
 * - WIN32_SYSROOT: Path to the mingw32 sysroot, default @c /usr/x86_64-w64-mingw32/sys-root/mingw/bin
 * - WIN32_PLUGINS: List of win32 plugins to use, default @c platforms
 * - BUILD_DIR: Name of the build directory, default @c build
 * - QMAKE_CXX: The C++ compiler to use for PC, default @c clang++
 * - QMAKE_LINK: The linker to use for PC, default @c clang++
 *
 * @subsection debugging Debugging
 * To see what goes over the local socket, start socat and adjust socket address:
 *
 * @code{.sh}
 * while true; do echo "Restart."; socat -v GOPEN:/tmp/Emacs1000/server UNIX-LISTEN:/tmp/es; done
 * @endcode
 *
 * Building for Android and Windows requires a proper cross-compiled toolchain,
 *
 * @section architecture Architecture
 * The application is separated into a client and a server part, communicating
 * via <a href="https://doc.qt.io/qt-5/qtremoteobjects-index.html">Qt Remote Objects</a>
 *
 * @subsection connection_setup Connection setup
 *
 * Emacsbridge starts up with some open settings endpoints to make initial
 * configuration easier. To set the connection up,
 *
 * - Emacs provides information about its server connection
 * - Emacsbridge uses that to connect to Emacs. If that connection was
 *   successful it will disable the open endpoints, and make the RPC interface
 *   available.
 *
 * @subsection urls URLs
 * @subsubsection icons /icons
 * Icons bundled with the application. Served without modification, directory
 * index is enabled. No authentication required.
 *
 * @subsubsection lisp /lisp
 * Lisp files bundled with the application. Served without modification,
 * directory index is enabled. No authentication required.
 *
 * @subsubsection rpc /rpc
 * Expects JSON data in a POST request, with a valid auth token. The method for
 * which JSON data is sent must be specified in the @c method header. Only
 * available after initial setup has been completed. The following methods
 * are available:
 *
 * @paragraph addComponent addComponent
 * | name      | required | default    |
 * |-----------|----------|------------|
 * | in-drawer | no       | false      |
 * | qmlData   | yes      |            |
 * | qmlFile   | yes      |            |
 * | title     | no       | @c qmlFile |
 * @paragraph setData setData
 * @paragraph notification notification
 * | name  | required | default                      |
 * |-------|----------|------------------------------|
 * | title | no       | "Missing notification title" |
 * | text  | no       | "Missing notification text"  |
 * @paragraph intent intent
 * | name      | required | default    |
 * |-----------|----------|------------|
 * | action    | no       |            |
 * | data      | no       |            |
 * | package   | no       |            |
 * | class     | no       |            |
 * | extra     | no       |            |
 * | startType | no       | "activity" |
 *
 * @code{.lisp}
 * (emacsbridge-post-json "intent" (json-encode `((:package . "com.termux")
 *                                                (:startType . "service")
 *                                                (:class . "com.termux.app.RunCommandService")
 *                                                (:extra . (((:type . "string")
 *                                                            (:key . "com.termux.RUN_COMMAND_PATH")
 *                                                            (:value . "/data/data/com.termux/files/usr/bin/ps"))
 *                                                           ((:type . "stringarray")
 *                                                            (:key . "com.termux.RUN_COMMAND_ARGUMENTS")
 *                                                            (:value . (((:value . "a"))
 *                                                                       ((:value . "x")))))))
 *                                                (:action . "com.termux.RUN_COMMAND"))))
 * @endcode
 *
 * @paragraph sensor sensor
 * @paragraph removeComponent
 * @subsubsection /scripts
 * Shell scripts bundled with the application. Served with simple template
 * expansion, directory index is enabled.
 * @subsubsection settings /settings
 * @subsubsection test_connection /test_connection
 * Returns a page reporting the setup status of the connection to Emacs in human
 * friendly text. No authentication required.
 *
 * @section preparing_build_env Preparing the build environment
 * @subsection android_builds Android builds
 *
 * First the directory structure gets prepared, and some variables need to be
 * set. The SDK directory used here is @c ~/.android/sdk:
 *
 * @code{.sh}
 * export ANDROID_SDK_ROOT=$HOME/.android/sdk
 * export ANDROID_HOME=$HOME/.android/sdk
 * mkdir -p $ANDROID_HOME
 * @endcode
 *
 * Next the command line tools need to be downloaded from
 * <a href="https://developer.android.com/studio#downloads">the Android SDK download page</a>
 * in the 'Command line tools only' section, and extracted:
 *
 * @code{.sh}
 * unzip -d $ANDROID_HOME ~/Downloads/commandlinetools-linux-6858069_latest.zip
 * export PATH=$PATH:$ANDROID_HOME/cmdline-tools/tools/bin
 * @endcode
 *
 * Now sdkmanager can be used to initialize the SDK and download required
 * packages:
 *
 * @code{.sh}
 * sdkmanager --verbose --licenses
 * sdkmanager --update
 * sdkmanager "platforms;android-29" "ndk-bundle" "build-tools;30.0.2" "platform-tools" "tools"
 * @endcode
 *
 * Next Qt needs to be Downloaded
 * (<a href="http://download.qt.io/official_releases/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz.mirrorlist">5.15.1 mirror list</a>),
 * extracted and built:
 *
 * @code{.sh}
 * tar xf qt-everywhere-src-5.15.1.tar.xz
 * cd qt-everywhere-src-5.15.1
 * ./configure -opensource -confirm-license -android-abis armeabi-v7a,arm64-v8a,x86_64 -xplatform android-clang --disable-rpath -nomake tests -nomake examples -no-warnings-are-errors -android-ndk $HOME/.android/sdk/ndk-bundle -android-sdk $HOME/.android/sdk/platforms -android-ndk-platform android-29 -prefix $HOME/qt/qt5.15.1-android-29
 * make -j$(nproc)
 * make -j$(nproc) install
 * @endcode
 *
 * @subsection windows_builds Windows builds
 *
 * The Windows version is highly experimental, seems to segfault due to issues
 * with Qt remote objects, and probably won't see any work unless I'm very bored
 * or somebody is interested in actually using it on Windows. Therefore the
 * following just contains a few pointers for a chance to have a build
 * environment.
 *
 * A Visual Studio build might have the best chances to actually work, but
 * probably will require code changes. MinGW compiles without code changes.
 * On OpenSuSE probably the following packages are required:
 *
 * - mingw64-cross-pkgconf
 * - mingw64-libicu-devel
 * - mingw64-angleproject-devel
 * - mingw64-cross-binutils
 * - mingw64-cross-gcc
 * - mingw64-cross-gcc-c++
 * - mingw64-cross-pkg-config
 * - mingw64-dbus-1-devel
 * - mingw64-filesystem
 * - mingw64-libicu-devel
 * - mingw64-libjpeg-devel
 * - mingw64-libopenssl-devel
 * - mingw64-libpng-devel
 * - mingw64-libtiff-devel
 * - mingw64-mysql-connector-c-devel
 * - mingw64-pcre-devel
 * - mingw64-sqlite-devel
 * - mingw64-zlib-devel
 * - mingw64-libharfbuzz-devel
 * - mingw64-glib2-devel
 * - mingw64-libintl-devel
 *
 * With those installed this should provide a more or less working build of Qt
 * for Windows:
 *
 * @code{.sh}
 * rm -Rf qtactiveqt
 * ./configure -opensource -confirm-license -xplatform win32-g++ -device-option CROSS_COMPILE=x86_64-w64-mingw32- -nomake examples -release -make tools -prefix $HOME/qt/qt5.15.0-mingw64 -opengl desktop -skip qtlocation -skip qtactiveqt
 * @endcode
 *
 * @section resources Resources
 * @subsection android Android
 * - <a href="https://developer.android.com/guide/topics/providers/contacts-provider">Android contacts provider</a>
 * - <a href="https://developers.google.com/maps/documentation/urls/guide">Google Maps URL schemes</a>
 * - <a href="https://developer.android.com/training/sync-adapters">Android sync adapters</a>
 * - <a href="https://developer.android.com/guide/topics/providers/content-providers">Android content providers</a>
 * @subsection qt Qt
 * - <a href="https://www.qt.io/blog/2019/01/25/introducing-qt-http-server">Qt Http Server announcement</a>
 * - <a href="https://bugreports.qt.io/browse/QTBUG-60105">QTBUG-60105 Lightweight HTTP server</a>
 * - <a href="https://doc.qt.io/qt-5/qtremoteobjects-index.html">Qt Remote objects</a>
 * - <a href="https://doc.qt.io/qt-5/android-services.html">Android services with Qt</a>
 * - <a href="https://doc.qt.io/qt-5/qtandroidextras-index.html">Android extras</a>
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
