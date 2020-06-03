#!/bin/sh
# build.sh
# (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

QT_ANDROID_BIN=${QT_ANDROID_BIN:-$HOME/qt/qt5.15.0-android-27/bin}
BUILD_DIR=${BUILD_DIR:-build}
SOURCE_DIR=`pwd`
declare -A ANDROID_ICONS=( [hdpi]=72 [mdpi]=48 [ldpi]=36 [xhdpi]=96 [xxhdpi]=142 [xxxhdpi]=192 )
declare -A PC_ICONS=( [mini]=48 [regular]=256 )

build_android(){
    mkdir -p ${BUILD_DIR}/android
    cd ${BUILD_DIR}/android
    $QT_ANDROID_BIN/qmake $SOURCE_DIR
    make -j$(nproc)
    cd emacsbridge
    make -j$(nproc) apk
    cd $SOURCE_DIR
}

build_android_icons(){
    mkdir -p android/res
    cd android/res

    for _res_name in "${!ANDROID_ICONS[@]}"; do
        _res=${ANDROID_ICONS[$_res_name]}
        mkdir -p drawable-${_res_name}
        echo "magick -density ${_res}x${_res} -background transparent ${SOURCE_DIR}/source_icons/merged.svg -define icon:auto-resize -resize ${_res}x${_res} -colors 256 drawable-${_res_name}/icon.png"
        magick -density ${_res}x${_res} -background transparent ${SOURCE_DIR}/source_icons/merged.svg -define icon:auto-resize -colors 256 -resize ${_res}x${_res} drawable-${_res_name}/icon.png
    done
    cd $SOURCE_DIR
}

build_pc(){
    echo "Building PC build in ${BUILD_DIR}/pc"
    mkdir -p ${BUILD_DIR}/pc
    cd ${BUILD_DIR}/pc
    qmake-qt5 $SOURCE_DIR
    make -j$(nproc)
    cd $SOURCE_DIR
}

build_pc_icons(){
    mkdir -p images
    for _res_name in "${!PC_ICONS[@]}"; do
        _res=${PC_ICONS[$_res_name]}
        if [ $_res_name = "regular" ]; then
            _icon_name="icon.png"
        else
            _icon_name="icon_${_res_name}.png"
        fi
        echo "magick -density ${_res}x${_res} -background transparent source_icons/merged.svg -define icon:auto-resize -resize ${_res}x${_res} -colors 256 images/${_icon_name}"
        magick -density ${_res}x${_res} -background transparent source_icons/merged.svg -define icon:auto-resize -resize ${_res}x${_res} -colors 256 images/${_icon_name}
    done
}

deploy_android(){
    mkdir -p ${BUILD_DIR}/android-deploy
    export BUILD_TARGET=`pwd`/${BUILD_DIR}/android-deploy
    cd ${BUILD_DIR}/android
    make install INSTALL_ROOT=$BUILD_TARGET
    $QT_ANDROID_BIN/androiddeployqt --output $BUILD_TARGET --gradle --android-platform android-27 --input android-emacsbridge-deployment-settings.json
    cd $SOURCE_DIR
}

mkdir -p $BUILD_DIR

case "$1" in
    "build-icons")
        build_android_icons
        build_pc_icons
        break
        ;;
    "deploy-android")
        build_android
        deploy_android
        break
        ;;
    "android")
        build_android
        break
        ;;
    "pc")
        build_pc
        break
        ;;
    *)
        build_pc
        build_android
        ;;
esac
