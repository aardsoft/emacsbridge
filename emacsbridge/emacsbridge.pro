include(../build_info.pri)
# main uses the version number passed in by defines. This is a hack to rebuild
# main every time to make sure the version is correct
REBUILD = $$system(touch main.cpp)

QT += quick quickcontrols2 websockets remoteobjects sensors gui
CONFIG += qmltypes
QML_IMPORT_NAME = fi.aardsoft.emacsbridge
QML_IMPORT_MAJOR_VERSION = 1

REPC_REPLICA += emacsbridgeremote.rep
REPC_SOURCE += emacsbridgeremote.rep

# INCLUDEPATH insists on changing this to a relative path if the directory
# exists. Unfortunately the relative path is wrong, so builds fail. Explicitely
# set compiler flags as workaround.
QMAKE_CXXFLAGS *= -I$$OUT_PWD/../qthttpserver/include -I$$OUT_PWD/../qthttpserver/include/QtHttpServer -DQT_HTTPSERVER_LIB -std=c++2a
QMAKE_LFLAGS *= -L$$OUT_PWD/../qthttpserver/lib
android{
QMAKE_LFLAGS *=  -lQt5HttpServer_$$QT_ARCH
}
unix:!android {
QMAKE_LFLAGS *=  -lQt5HttpServer
}

qtConfig(ssl){
  QMAKE_LFLAGS *= -lQt5SslServer
}

android {
  QT += androidextras
  # see android_deployment_settings.prf
  # setting sdk versions in gradle.properties gets overwritten for some qt versions
  ANDROID_MIN_SDK_VERSION = 27
  ANDROID_TARGET_SDK_VERSION = 29
  ANDROID_VERSION_NAME = $$VERSION
  ANDROID_VERSION_CODE = $$TOTAL_COMMIT_COUNT
  ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../android
  OTHER_FILES += \
    ../android/src/fi/aardsoft/emacsbridge/EmacsBridgeNotification.java \
    ../android/src/fi/aardsoft/emacsbridge/EmacsBridgeActivity.java \
    ../android/src/fi/aardsoft/emacsbridge/EmacsBridgeService.java \
    ../android/src/fi/aardsoft/emacsbridge/EmacsBridgeBroadcastReceiver.java \
    ../android/AndroidManifest.xml
  for(abi, ANDROID_ABIS): ANDROID_EXTRA_LIBS *= $$OUT_PWD/../qthttpserver/lib/libQt5HttpServer_$${abi}.so
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
    emacsbridgelog.cpp \
    emacsbridgemorse.cpp \
    emacsbridgeremote.cpp \
    emacsbridgesettings.cpp \
    emacsclient.cpp \
    emacslogger.cpp \
    emacsserver.cpp \
    emacsservice.cpp

RESOURCES += \
    ../main.qrc

HEADERS += \
    emacsbridge.h \
    emacsbridgelog.h \
    emacsbridgemorse.h \
    emacsbridgesettings.h \
    emacsclient.h \
    emacslogger.h \
    emacsserver.h \
    emacsservice.h \
    emacsbridgeremote.h \
    emacsbridgetypes.h
