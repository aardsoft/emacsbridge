include(../build_info.pri)
# main uses the version number passed in by defines. This is a hack to rebuild
# main every time to make sure the version is correct
REBUILD = $$system(touch main.cpp)

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
QMAKE_CXXFLAGS *= -I$$OUT_PWD/../qthttpserver/include -I$$OUT_PWD/../qthttpserver/include/QtHttpServer -DQT_HTTPSERVER_LIB -std=c++2a
QMAKE_LFLAGS *= -L$$OUT_PWD/../qthttpserver/lib -lQt5HttpServer$$ANDROID_ARCH

qtConfig(ssl){
  QMAKE_LFLAGS *= -lQt5SslServer
}

android {
  QT += androidextras
  ANDROID_VERSION_NAME = $$VERSION
  ANDROID_VERSION_CODE = $$TOTAL_COMMIT_COUNT
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
  target.path = /usr/bin
  user_service.path = /usr/lib/systemd/user
  user_service.files = ../emacsbridge-service.service ../emacsbridge.service
  INSTALLS += target user_service
}

windows {
  QT += widgets
  QMAKE_LIBS_PRIVATE += -lQt5HttpServer
  CONFIG += console
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
    emacsbridgeremote.h \
    emacsbridgetypes.h
