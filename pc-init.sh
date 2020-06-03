#!/bin/sh
# pc-init.sh
# Script to quickly configure emacsbridge on PC
# (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

SERVER_SOCKET=`emacsclient -e '(concat server-socket-dir "/server")'|sed 's/"//g'`
curl -v -d $SERVER_SOCKET --header "setting: server-socket" localhost:1616/settings
