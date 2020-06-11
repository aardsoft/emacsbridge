!versionAtLeast(QT_VERSION, 5.14): error("Qt 5.14 or newer is required")
include(build_info.pri)

TEMPLATE = subdirs
SUBDIRS = qthttpserver emacsbridge

emacsbridge.depends = qthttpserver
