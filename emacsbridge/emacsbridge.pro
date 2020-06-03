TARGET = emacsbridge
QT += quick quickcontrols2 websockets remoteobjects
CONFIG += qmltypes
QML_IMPORT_NAME = fi.aardsoft.emacsbridge
QML_IMPORT_MAJOR_VERSION = 1

REPC_REPLICA += emacsbridgeremote.rep
REPC_SOURCE += emacsbridgeremote.rep

# part of the libraries, should probably be autodetected on build
android{
  ANDROID_ARCH=_arm64-v8a
}

# INCLUDEPATH insists on changing this to a relative path if the directory
# exists. Unfortunately the relative path is wrong, so builds fail. Explicitely
# set compiler flags as workaround.
QMAKE_CXXFLAGS *= -I$$OUT_PWD/../qthttpserver/include -I$$OUT_PWD/../qthttpserver/include/QtHttpServer -DQT_HTTPSERVER_LIB
QMAKE_LFLAGS *= -L$$OUT_PWD/../qthttpserver/lib -lQt5HttpServer$$ANDROID_ARCH

qtConfig(ssl){
  QMAKE_LFLAGS *= -lQt5SslServer
}

android {
  QT += androidextras
  ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../android
  OTHER_FILES += \
    ../android/src/fi/aardsoft/emacsbridge/EmacsBridgeNotification.java \
    ../android/src/fi/aardsoft/emacsbridge/EmacsBridgeActivity.java \
    ../android/src/fi/aardsoft/emacsbridge/EmacsBridgeService.java \
    ../android/src/fi/aardsoft/emacsbridge/EmacsBridgeBroadcastReceiver.java \
    ../android/AndroidManifest.xml
  ANDROID_EXTRA_LIBS *= $$OUT_PWD/../qthttpserver/lib/libQt5HttpServer$${ANDROID_ARCH}.so
}


unix:!android {
  QT += widgets
}


SOURCES += \
    main.cpp \
    emacsbridge.cpp \
    emacsbridgeremote.cpp \
    emacsbridgesettings.cpp \
    emacsclient.cpp \
    emacsserver.cpp \
    emacsservice.cpp

RESOURCES += \
    ../main.qrc

HEADERS += \
    emacsbridge.h \
    emacsbridgesettings.h \
    emacsclient.h \
    emacsserver.h \
    emacsservice.h \
    emacsbridgeremote.h
