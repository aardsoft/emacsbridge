#!/bin/bash
# build.sh
# (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

GIT_SOURCE_DIR=`git rev-parse -q --absolute-git-dir 2>/dev/null`
IS_GITDIR=$?

if [ $IS_GITDIR -eq 0 ]; then
    if [ -f $GIT_SOURCE_DIR/../.local ]; then
        . $GIT_SOURCE_DIR/../.local
    fi
    GIT_NEWEST_TAG=`git describe --abbrev=0 --tags`
    GIT_COMMIT_COUNT=`git rev-list ${GIT_NEWEST_TAG}.. --count`
    GIT_TOTAL_COMMIT_COUNT=`git rev-list HEAD --count`
    # 0 if dirty, 1 if clean
    IS_GITDIRTY=`git describe --abbrev=0 --tags --dirty="/dirty"|grep -q dirty; echo $?`
    SOURCE_DIR=$GIT_SOURCE_DIR/../
else
    SOURCE_DIR=`pwd`
    if [ -f .local ]; then
        .local
    fi
fi

ANDROID_ABIS=${ANDROID_ABIS:-arm64-v8a x86_64}
ANDROID_SDK_VERSION=${ANDROID_SDK_VERSION:-android-29}
QT_VERSION=${QT_VERSION:-5.15.1}
QT_ANDROID_BIN=${QT_ANDROID_BIN:-$HOME/qt/qt${QT_VERSION}-${ANDROID_SDK_VERSION}/bin}
QT_WINDOWS_BIN=${QT_WINDOWS_BIN:-$HOME/qt/qt${QT_VERSION}-mingw64/bin}
WIN32_OBJDUMP=${WIN32_OBJDUMP:-x86_64-w64-mingw32-objdump}
WIN32_SYSROOT=${WIN32_SYSROOT:-/usr/x86_64-w64-mingw32/sys-root/mingw/bin}
WIN32_PLUGINS=${WIN32_PLUGINS:-platforms}
BUILD_DIR=${BUILD_DIR:-build}
QMAKE_CXX=${QMAKE_CXX:-clang++}
QMAKE_LINK=${QMAKE_LINK:-clang++}
declare -A ANDROID_ICONS=( [hdpi]=72 [mdpi]=48 [ldpi]=36 [xhdpi]=96 [xxhdpi]=142 [xxxhdpi]=192 )
declare -A PC_ICONS=( [mini]=48 [regular]=256 [large]=512 )
export ANDROID_SDK_ROOT

dump_DLL(){
    if [ -n "$1" ]; then
        $WIN32_OBJDUMP -p $1 |grep 'DLL Name:'|sed 's/^.*DLL Name: //'
    fi
}

deploy_DLLs(){
    for _dll in `dump_DLL $1`; do
        if [ -f "${QT_WINDOWS_BIN}/${_dll}" ]; then
            cp "${QT_WINDOWS_BIN}/${_dll}" ${2}/
        elif [ -f "${WIN32_SYSROOT}/${_dll}" ]; then
            cp "${WIN32_SYSROOT}/${_dll}" ${2}/
        else
            echo "Skipping ${_dll}"
        fi
    done
}

build_android(){
    if [ -z "$ANDROID_SDK_ROOT" ]; then
        echo "ANDROID_SDK_ROOT not set. It can be configured in the .local file."
        exit 1
    fi
    mkdir -p ${BUILD_DIR}/android
    cd ${BUILD_DIR}/android
    $QT_ANDROID_BIN/qmake CONFIG+=release ANDROID_ABIS="$ANDROID_ABIS" $SOURCE_DIR
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
    qmake-qt5 QMAKE_CXX=$QMAKE_CXX QMAKE_LINK=$QMAKE_LINK $SOURCE_DIR
    make -j$(nproc)
    cd $SOURCE_DIR
}

build_pc_icons(){
    mkdir -p icons
    for _res_name in "${!PC_ICONS[@]}"; do
        _res=${PC_ICONS[$_res_name]}
        if [ $_res_name = "regular" ]; then
            _icon_name="icon.png"
        else
            _icon_name="icon_${_res_name}.png"
        fi
        echo "magick -density ${_res}x${_res} -background transparent source_icons/merged.svg -define icon:auto-resize -resize ${_res}x${_res} -colors 256 icons/${_icon_name}"
        magick -density ${_res}x${_res} -background transparent source_icons/merged.svg -define icon:auto-resize -resize ${_res}x${_res} -colors 256 icons/${_icon_name}
    done
}

build_dev_rpm(){
    if [ $IS_GITDIR -eq 0 ]; then
        if [ $IS_GITDIRTY -eq 0 ]; then
            _release="dirty"
        else
            _release=1
        fi
        mkdir -p ~/rpmbuild/SPECS ~/rpmbuild/SOURCES
        if [ -d $GIT_SOURCE_DIR/../build ]; then
            rm -Rf $GIT_SOURCE_DIR/../build
        fi
        tar czf ~/rpmbuild/SOURCES/emacsbridge.tar.gz $GIT_SOURCE_DIR/..
        sed -e "s/RELEASE/$_release/" -e "s/VERSION/$GIT_NEWEST_TAG.$GIT_COMMIT_COUNT/" $GIT_SOURCE_DIR/../emacsbridge.spec > ~/rpmbuild/SPECS/emacsbridge.spec
        rpmbuild -ba ~/rpmbuild/SPECS/emacsbridge.spec
    else
        echo "Building dev RPMs is only supported from within a git tree"
    fi
}

build_windows(){
    mkdir -p ${BUILD_DIR}/windows
    cd ${BUILD_DIR}/windows
    $QT_WINDOWS_BIN/qmake CONFIG+=debug -recursive $SOURCE_DIR
    make -j$(nproc)
    cd $SOURCE_DIR
}

copy_android_artifacts(){
    _debug_dir=$BUILD_DIR/android/emacsbridge/android-build/build/outputs/apk/debug
    _release_dir=$BUILD_DIR/android-deploy/build/outputs/apk/release
    _bundle_dir=$BUILD_DIR/android-deploy/build/outputs/bundle
    if [ -n "$ARTIFACT_COPY_DIR" ]; then
        _version=$GIT_NEWEST_TAG.$GIT_COMMIT_COUNT
        if [ $IS_GITDIRTY -eq 0 ]; then
            _version=${_version}-dirty
        fi
        mkdir -p "$ARTIFACT_COPY_DIR"
        if  [ -f $_debug_dir/android-build-debug.apk ]; then
            cp $_debug_dir/android-build-debug.apk $ARTIFACT_COPY_DIR/emacsbridge-${_version}-debug.apk
        fi
        if [ -f $_release_dir/android-deploy-release-signed.apk ]; then
            cp $_release_dir/android-deploy-release-signed.apk $ARTIFACT_COPY_DIR/emacsbridge-${_version}-release-signed.apk
        fi
        if [ -f $_release_dir/android-deploy-release-unsigned.apk ]; then
            cp $_release_dir/android-deploy-release-unsigned.apk $ARTIFACT_COPY_DIR/emacsbridge-${_version}-release-unsigned.apk
        fi
        if [ -f $_bundle_dir/release/android-deploy-release.aab ]; then
            cp $_bundle_dir/release/android-deploy-release.aab $ARTIFACT_COPY_DIR/emacsbridge-${_version}-release.aab
        fi
        if [ -f $_bundle_dir/debug/android-deploy-debug.aab ]; then
            cp $_bundle_dir/debug/android-deploy-debug.aab $ARTIFACT_COPY_DIR/emacsbridge-${_version}-debug.aab
        fi
    fi
}

deploy_android(){
    mkdir -p ${BUILD_DIR}/android-deploy
    export BUILD_TARGET=${BUILD_DIR}/android-deploy
    cd ${BUILD_DIR}/android
    make install INSTALL_ROOT=$BUILD_TARGET
    if  [ -n "$SIGN_URL" ] && [ -n "$SIGN_ALIAS" ]; then
        DEPLOY_SIGN_ARGS="--sign $SIGN_URL $SIGN_ALIAS"
    fi
    $QT_ANDROID_BIN/androiddeployqt --release --output $BUILD_TARGET --gradle --android-platform $ANDROID_SDK_VERSION --input emacsbridge/android-emacsbridge-deployment-settings.json --aab $DEPLOY_SIGN_ARGS
    cd $SOURCE_DIR
}

deploy_windows(){
    mkdir -p ${BUILD_DIR}/windows-deploy
    export BUILD_TARGET=${BUILD_DIR}/windows-deploy
    cd ${BUILD_DIR}/windows
    cp qthttpserver/lib/Qt5HttpServer.dll ${BUILD_TARGET}/
    deploy_DLLs emacsbridge/debug/emacsbridge.exe ${BUILD_TARGET}/
    for _dll in ${BUILD_TARGET}/*.dll; do
        deploy_DLLs $_dll ${BUILD_TARGET}/
    done | grep Skipping|sort|uniq
    mkdir -p ${BUILD_TARGET}/plugins
    for _plugin in ${WIN32_PLUGINS}; do
        cp -R ${QT_WINDOWS_BIN}/../plugins/${_plugin} ${BUILD_TARGET}/plugins/
    done
    cp -R ${QT_WINDOWS_BIN}/../qml ${BUILD_TARGET}/
    cd $SOURCE_DIR
}

release(){
  (( _total_commit_count=$GIT_TOTAL_COMMIT_COUNT+1 ))
  if [ -n "$1" ]; then
      _new_tag=$1
  else
      _newest_tag_minor=${GIT_NEWEST_TAG//*./}
      _newest_tag_major=${GIT_NEWEST_TAG//.*/}
      (( _newest_tag_minor=_newest_tag_minor+1 ))
      _new_tag=$_newest_tag_major.$_newest_tag_minor
  fi
  cat >release_version.pri <<-EOF
NEWEST_TAG = $_new_tag
COMMIT_COUNT = 0
TOTAL_COMMIT_COUNT = $_total_commit_count
EOF
  git add release_version.pri
  git commit -m "Release $_new_tag"
  git tag $_new_tag
}

mkdir -p $BUILD_DIR

case "$1" in
    "build-icons")
        build_android_icons
        build_pc_icons
        ;;
    "deploy-android")
        build_android
        deploy_android
        copy_android_artifacts
        ;;
    "android")
        build_android
        copy_android_artifacts
        ;;
    "dev-rpm")
        build_dev_rpm
        ;;
    "pc")
        build_pc
        ;;
    "release")
        release "$2"
        ;;
    "run")
        if [ -z "$2" ]; then
            LD_LIBRARY_PATH=$BUILD_DIR/pc/qthttpserver/lib $BUILD_DIR/pc/emacsbridge/emacsbridge
        elif [ "$2" = "-service" ]; then
            LD_LIBRARY_PATH=$BUILD_DIR/pc/qthttpserver/lib $BUILD_DIR/pc/emacsbridge/emacsbridge
        fi
        ;;
    "windows")
        build_windows
        deploy_windows
        ;;
    *)
        build_pc
        build_android
        ;;
esac
