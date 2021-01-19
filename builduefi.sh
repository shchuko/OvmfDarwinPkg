#!/bin/bash
##
# edk2 build command wrapper
#
# Copyright (c) 2020, Vladislav Yaroshchuk <yaroshchuk2000@gmail.com> 
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

setEnv() {   
  PKG_PATH="$PWD"
  
  # EDK2 stable branch is used
  EDK_REMOTE="https://github.com/tianocore/edk2.git"
  EDK_BRANCH="edk2-stable202008"
  EDK_DIR="$PKG_PATH/edk2"

  PKG_LINK_PATH="$EDK_DIR/OvmfDarwinPkg"
}

loadBuildDefaults() {
  ACTIVE_PLATFORM="OvmfDarwinPkg/OvmfDarwinPkgX64.dsc"
  TARGET_ARCH="X64"
  TARGET="DEBUG"

  case "$(uname)" in
    [Dd]arwin) TOOLCHAIN="XCODE5" ;;
    [Ll]inux) TOOLCHAIN="GCC5" ;;
    *) TOOLCHAIN="unknown"; return 1 ;;
  esac;
}

printHelp() {
  echo -e "edk2 build command wrapper"
  echo -e "usage: ./builduefi.sh [options] [build-args]" 
  echo
  echo -e "options:"
  echo -e "\t-help\t\t\tPrint this help"
  echo -e "\t-buildRelease\t\tShortcut for '-b RELEASE'"
  echo -e "\t-debugSerial\t\tShortcut for '-D DEBUG_ON_SERIAL_PORT;"
  echo -e "\t-a TARGETARCH\t\tCustom target arch"
  echo -e "\t-b TARGET\t\tCustom build target"
  echo -e "\t-p PLATFORMFILE\t\tCustom platform file (*.dsc)"
  echo -e "\t-t TOOLCHAIN\t\tCustom toolchain"
  echo
  echo -e "build-args: will be used as build command args" 
  echo

  loadBuildDefaults
  ret=$?
  
  echo -e "defaults:"
  echo -e "\tACTIVE_PLATFORM=$ACTIVE_PLATFORM"
  echo -e "\tBUILDTARGET=$TARGET"
  echo -e "\tTOOLCHAIN=$TOOLCHAIN"
  echo -e "\tTARGET_ARCH=$TARGET_ARCH"

  return $ret
}

setupEdk() {
  git clone --filter=blob:none --single-branch --branch "$EDK_BRANCH" "$EDK_REMOTE" "$EDK_DIR"   
 
  cd "$EDK_DIR"

  git submodule update --init
  make -C BaseTools
  
  cd "$PKG_PATH"  
}

createLinks() {
  [[ -d $PKG_LINK_PATH ]] || ln -s "$PKG_PATH" "$PKG_LINK_PATH"

  cd "$EDK_DIR"

  [[ -d "$PKG_PATH/LegacyPackages" ]] && \
  for LEGACY_PKG_PATH in $(find "$PKG_PATH/LegacyPackages" -maxdepth 1 -type d); do
    LEGACY_PKG_LINK="$(basename $LEGACY_PKG_PATH)"
    [[ -d $LEGACY_PKG_LINK ]] || ln -s "$LEGACY_PKG_PATH" "$LEGACY_PKG_LINK" 
  done

  cd "$PKG_PATH"
  
  # Copy 'Build' directory easier to work with
  [[ -d Build ]] || ln -s "$EDK_DIR/Build" Build
  
}

runBuild() {
  cd "$EDK_DIR"

  [[ -n $CUSTOM_TARGET_ARCH ]] && TARGET_ARCH="$CUSTOM_TARGET_ARCH" 
  [[ -n $CUSTOM_TARGET ]] && TARGET="$CUSTOM_TARGET" 
  [[ -n $CUSTOM_ACTIVE_PLATFORM ]] && ACTIVE_PLATFORM="$CUSTOM_ACTIVE_PLATFORM" 
  [[ -n $CUSTOM_TOOLCHAIN ]] && TOOLCHAIN="$CUSTOM_TOOLCHAIN" 

  CMD="build -a $TARGET_ARCH -t $TOOLCHAIN -p $ACTIVE_PLATFORM -b $TARGET $BUILD_COMMAND_ARGS"
  echo "exec: $CMD"
  
  source edksetup.sh  
  eval "$CMD"
  EXIT_STATUS=$?

  cd "$PKG_PATH"

  return $EXIT_STATUS
}

readArgs() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      -a) CUSTOM_TARGET_ARCH="$2"; shift
        ;;

      -b) CUSTOM_TARGET="$2"; shift
        ;;

      -p) CUSTOM_ACTIVE_PLATFORM="$2"; shift
        ;;

      -t) CUSTOM_TOOLCHAIN="$2"; shift
        ;;

      -buildRelease) CUSTOM_TARGET="RELEASE"
        ;;

      -debugSerial) BUILD_COMMAND_ARGS="$BUILD_COMMAND_ARGS -DDEBUG_ON_SERIAL_PORT" 
        ;;

      -help) printHelp; exit $?
        ;;

      *) BUILD_COMMAND_ARGS="$BUILD_COMMAND_ARGS $@"; break
        ;;
    esac
    shift
  done
}

readArgs $@

setEnv
loadBuildDefaults

if [[ $? -ne 0 ]]; then
  echo "System type unknown, couldn't set TOOLCHAIN"
fi

if [[ ! -d "$EDK_DIR" ]]; then
  setupEdk
fi

createLinks
runBuild
exit $?
