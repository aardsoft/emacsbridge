#!/bin/sh
# run.sh
# (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

LD_LIBRARY_PATH=`pwd`/build/pc/qthttpserver/lib ./build/pc/emacsbridge/emacsbridge
