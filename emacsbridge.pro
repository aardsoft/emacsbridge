android {
!versionAtLeast(QT_VERSION, 5.14): error("Qt 5.14 or newer is required")
!versionAtLeast(QT_VERSION, 5.15): message("Qt 5.15 or newer is recommended")
} else {
!versionAtLeast(QT_VERSION, 5.13): error("Qt 5.13 or newer is required")
!versionAtLeast(QT_VERSION, 5.14): message("Qt 5.14 or newer is recommended")
}

include(build_info.pri)

TEMPLATE = subdirs
SUBDIRS = qthttpserver emacsbridge

emacsbridge.depends = qthttpserver
