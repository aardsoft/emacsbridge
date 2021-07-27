#!/data/data/com.termux/files/usr/bin/bash
# termux-init.sh
# Script to quickly configure emacsbridge on Android/termux
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

check_emacs_connection(){
    result=`emacsclient -e '(+ 2 2)' 2>/dev/null`
    if [ "$?" -ne 0 ]; then
        return 1
    fi

    if [ "$result" -eq 4 ]; then
        return 0
    fi

    return 1
}

green_bold "Starting setup for emacs bridge"
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
        err_exit "Unable to install X11 repository"
    green "-> installing emacs package"
    apt -y --no-install-recommends install emacs-x ||
        err_exit "Unable to install emacs-x package"
fi

if check_emacs_connection; then
    yellow "emacs seems to be there"
    _emacs_initfile=`emacsclient -e 'user-init-file' 2>/dev/null|sed 's/"//g'`
else
    green "-> searching for existing init files"
    if [ -f ~/.emacs.el ]; then
        yellow "found ~/.emacs.el"
        _emacs_initfile=$HOME/.emacs.el
    elif [ -f ~/.emacs ]; then
        yellow "found ~/.emacs"
        _emacs_initfile=$HOME/.emacs
    elif [ -f ~/.emacs.d/init.el ]; then
        yellow "found ~/.emacs.d/init.el"
        _emacs_initfile=$HOME/.emacs.d/init.el
    elif [ -f ~/.config/emacs/init.el ]; then
        yellow "found ~/.config/emacs.d/init.el"
        _emacs_initfile=$HOME/.config/emacs/init.el
    fi
fi



if [ -z "$_emacs_initfile" ]; then
    green "no init file found, using ~/.emacs.d/init.el"
    _emacs_initfile=$HOME/.config/emacs/init.el
fi

# TODO:
# - if initdir == homedir, create a sub directory
# - create initdir
# - check sensible lisp path
# - download the rpc stuff
# - add endpoint to check rpc file version, and use that for rpc auto-update
# - emacsclient to load the rpc file
# - add init code to the rpc file, and then just execute that with emacs client
# - add support to load overrides from termux home dir or emacs dir

_emacs_initdir=`dirname $_emacs_initfile`
green "init file is $_emacs_initfile"


green_bold "Press enter to close this window"
read
