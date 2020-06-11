#!/bin/sh
# run.sh
# (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

if [ -z "$1" ]; then
    LD_LIBRARY_PATH=`pwd`/build/pc/qthttpserver/lib ./build/pc/emacsbridge/emacsbridge
elif [ "$1" = "-service" ]; then
    LD_LIBRARY_PATH=`pwd`/build/pc/qthttpserver/lib ./build/pc/emacsbridge/emacsbridge -service
fi
