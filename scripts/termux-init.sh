#!/data/data/com.termux/files/usr/bin/bash
# termux-init.sh
# Script to quickly configure emacsbridge on Android/termux
# (c) 2020 Bernd Wachter <bwachter-github@aardsoft.fi>

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
green "-> updating package cache"
apt-get update

green "-> checking existing packages"
dpkg -s emacs  >/dev/null 2>&1
if [ $? -eq 0 ]; then
    green "found emacs without X support"
    _emacs_installed=true
fi
dpkg -s emacs-x 2>&1 >/dev/null
if [ $? -eq 0 ]; then
    green "found emacs with X support"
    _emacs_installed=true
fi

if [ "$_emacs_installed" != "true" ]; then
    green "-> installing x11-repo"
    apt -y --no-install-recommends install x11-repo ||
        red "Unable to install X11 repository. No GUI will be available."
    green "-> installing emacs package"
    apt -y --no-install-recommends install emacs-x
    if [ $? -ne 0 ]; then
        red "Unable to install emacs-x package"
        red "Falling back to non-X emacs"
        apt -y --no-install-recommends install emacs ||
            err_exit "Unable to install emacs package"
    fi
fi

green "-> installing additional termux packages"
apt -y --no-install-recommends install termux-services ||
    err_exit "Unable to install additional packages"

green "-> configuring emacs"
curl -o $HOME/emacs-init.sh http://$EMACSBRIDGE_HOST:$EMACSBRIDGE_PORT/scripts/emacs-init.sh ||
    err_exit "Unable to retrieve emacs init script"

. $HOME/emacs-init.sh
rm $HOME/emacs-init.sh

green_bold "Press enter to close this window"
read
