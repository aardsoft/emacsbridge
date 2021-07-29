#!/bin/sh
# pc-init.sh
# Script to quickly configure emacsbridge on PC
# (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

EMACSBRIDGE_PORT={{http/bindPort}}
EMACSBRIDGE_HOST={{http/bindAddress}}

red(){
    echo -e "\e[31m$1\e[0m"
}

red_bold(){
    echo -e "\e[31m\e[1m$1\e[0m"
}

green(){
    echo -e "\e[32m$1\e[0m"
}

green_bold(){
    echo -e "\e[32m\e[1m$1\e[0m"
}

yellow(){
    echo -e "\e[33m$1\e[0m"
}

err_exit(){
    red_bold "$1"
    exit 1
}

green_bold "Starting setup for emacs bridge"

SERVER_SOCKET=`emacsclient -e '(concat server-socket-dir "/server")'|sed 's/"//g'`
#curl -v -d $SERVER_SOCKET --header "setting: server-socket" $EMACSBRIDGE_HOST:$EMACSBRIDGE_PORT/settings

echo
echo "Checking if variables should be set"
LISP_PATH=`pwd`/lisp
QML_PATH=`pwd`/qml

#emacsclient -e "(progn(add-to-list 'load-path \"$LISP_PATH\")(message \"Configuring load-path\"))"
#emacsclient -e "(if (and (boundp 'emacsbridge-default-qml-path) emacsbridge-default-qml-path)(message \"emacsbridge-default-qml-path is set, not touching it\")(progn (setq emacsbridge-default-qml-path \"$QML_PATH\")(message \"setting emacsbridge-default-qml-path\")))"
#emacsclient -e "(require 'emacsbridge-rpc)"

curl -o $HOME/emacs-init.sh http://$EMACSBRIDGE_HOST:$EMACSBRIDGE_PORT/scripts/emacs-init.sh ||
    err_exit "Unable to retrieve emacs init script"

. $HOME/emacs-init.sh
rm $HOME/emacs-init.sh
