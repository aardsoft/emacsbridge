#!/bin/sh
# emacs-init.sh
# (c) 2021 Bernd Wachter <bwachter@lart.info>

SERVER_PORT={{networkSocket/port}}
SERVER_ADDRESS={{networkSocket/address}}
SERVER_USE_TCP={{emacs/serverUseTCP}}
DEFAULT_INIT_DIRECTORY={{emacs/lispDirectory}}

_emacs_running=0

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

if check_emacs_connection; then
    yellow "emacs seems to be there"
    _emacs_initfile=`emacsclient -e 'user-init-file' 2>/dev/null|sed 's/"//g'`
    _emacs_running=1
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
    green "no init file found, using $DEFAULT_INIT_DIRECTORY/init.el"
    _emacs_initfile="$DEFAULT_INIT_DIRECTORY/init.el"
fi

_emacs_initfile_dir=`dirname $_emacs_initfile`
_emacs_initfile_name=`basename $_emacs_initfile`
_lisp_dir="$_emacs_initfile_dir/emacsbridge"

green "init file is $_emacs_initfile"

if [ -d "$_emacs_initfile_dir/.git" ]; then
    green "emacs configuration is git managed, not adding to that"
    green "using $HOME/emacsbridge as alternative lisp directory"
    _lisp_dir="$HOME/emacsbridge"
fi

mkdir -p $_lisp_dir ||
    err_exit "Unable to create lisp directory $_lisp_dir"

# at this stage we just need to manage to get emacs running. once emacs is up we
# can do everything - including changing the server to TCP - from inside of it
emacsclient -e '(switch-to-buffer (url-retrieve-synchronously "http://{{EMACSBRIDGE_HOST}}:{{EMACSBRIDGE_PORT}}/scripts/emacs-init.el"))(delete-region (point-min) url-http-end-of-headers)(eval-buffer)(kill-buffer (current-buffer))' ||
    err_exit "Unable to execute initial lisp code"

# TODO:
# - if initdir == homedir, create a sub directory
# - create initdir
# - check sensible lisp path
# - download the rpc stuff
# - add endpoint to check rpc file version, and use that for rpc auto-update
# - emacsclient to load the rpc file
# - add init code to the rpc file, and then just execute that with emacs client
# - add support to load overrides from termux home dir or emacs dir
