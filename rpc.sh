#!/bin/sh
# rpc.sh
# Script to test the RPC interface
# (c) 2020 Bernd Wachter <bwachter@lart.info>

_config_file=${_config_file:-${HOME}/.config/AardSoft/emacsbridge.conf}
if [ -f $_config_file ]; then
    _auth_token=`grep ^auth-token= ${_config_file}|sed 's/^auth-token=//'`
    echo $_auth_token
fi

if [ -z "$_auth_token" ]; then
    echo "RPC calls only work with a valid auth token."
    exit 1
fi

if [ -z "$1" ]; then
    echo "First argument should be the method name"
    exit 1
else
    _method=$1
fi

if [ -z "$2" ]; then
    echo "Enter JSON data, and end with CTRL-D"
    _json=$(</dev/stdin)
else
    if [ -f "$2" ]; then
        echo "Reading JSON data from file '$2'"
        _json=$(<$2)
    else
        _json=$2
    fi
fi

curl -v --header "auth-token: $_auth_token" --header "method: $_method" -d "$_json" localhost:1616/rpc
