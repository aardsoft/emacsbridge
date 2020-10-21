#!/bin/sh
# pc-init.sh
# Script to quickly configure emacsbridge on PC
# (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

SERVER_SOCKET=`emacsclient -e '(concat server-socket-dir "/server")'|sed 's/"//g'`
curl -v -d $SERVER_SOCKET --header "setting: server-socket" {{SERVER_HOST}}:{{SERVER_PORT}}/settings

echo
echo "Checking if variables should be set"
LISP_PATH=`pwd`/lisp
QML_PATH=`pwd`/qml

emacsclient -e "(progn(add-to-list 'load-path \"$LISP_PATH\")(message \"Configuring load-path\"))"
emacsclient -e "(if (and (boundp 'emacsbridge-default-qml-path) emacsbridge-default-qml-path)(message \"emacsbridge-default-qml-path is set, not touching it\")(progn (setq emacsbridge-default-qml-path \"$QML_PATH\")(message \"setting emacsbridge-default-qml-path\")))"
emacsclient -e "(require 'emacsbridge-rpc)"
