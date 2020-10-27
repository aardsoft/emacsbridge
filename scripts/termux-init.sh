#!/data/data/com.termux/files/usr/bin/bash
# termux-init.sh
# Script to quickly configure emacsbridge on Android/termux
# (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

EMACSBRIDGE_PORT={{http/bindPort}}
EMACSBRIDGE_HOST={{http/bindAddress}}

red(){
    echo -e "\e[31m$1\e[0m"
}

green(){
    echo -e "\e[32m$1\e[0m"
}

green_bold(){
    echo -e "\e[32m$1\e[0m"
}

yellow(){
    echo -e "\e[33m$1\e[0m"
}

green_bold "Starting setup for emacs bridge"
green "-> installing x11-repo"
apt -y --no-install-recommends install x11-repo
green "-> installing emacs package"
apt -y --no-install-recommends install emacs-x
green "Press enter to close this window"
read
