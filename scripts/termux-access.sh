#!/bin/sh
# termux-access.sh
# Script for making sure termux app access is setup
# (c) 2021 Bernd Wachter <bwachter@aardsoft.fi>

green(){
    echo -e "\e[32m$1\e[0m"
}

green_bold(){
    echo -e "\e[32m\e[1m$1\e[0m"
}

P="$HOME/.termux/termux.properties"

if ! [ -d "$HOME/.termux" ]; then
    green "~/.termux missing, creating."
    mkdir -p $HOME/.termux
fi

if ! [ -f "$P" ]; then
    green "No termux properties found, creating new one"
    echo "allow-external-apps = true" > $P
else
    grep -q ^allow-external-apps $P
    if [ $? -eq 1 ]; then
        green "Termux properties without allow-external-apps found, adding"
        # setting not present, so just append
        echo "allow-external-apps = true" >> $P
    else
        green "Termux properties with allow-external-apps found, adjusting if necessary"
        # setting present - we don't about the current value, just
        # make sure it's true now
        sed -i 's/^allow-external-apps.*/allow-external-apps = true/' $P
    fi
fi
green "Reloading termux settings"
termux-reload-settings
green_bold "You should now be able to continue setup from the Emacs bridge UI"
