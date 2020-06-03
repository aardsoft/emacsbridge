!versionAtLeast(QT_VERSION, 5.14): error("Qt 5.14 or newer is required")

TEMPLATE = subdirs
SUBDIRS = qthttpserver emacsbridge

emacsbridge.depends = qthttpserver
