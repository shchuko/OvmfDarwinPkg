#!/bin/bash
##
# edk2 build command wrapper
#
# Note: set '$EDK_DIR' to specify edk2 sources root
#       if not set, edk2 will be cloned using cloneEdk()
#
# Copyright (c) 2021, Vladislav Yaroshchuk <yaroshchuk2000@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##
set -eo pipefail

cloneEdk() {
  EDK_REMOTE="https://github.com/tianocore/edk2.git"
  EDK_BRANCH="edk2-stable202105"
  EDK_DIR="$PWD/edk2"

  if [[ ! -d "$EDK_DIR" ]]; then
    git clone --filter=blob:none --single-branch --branch "$EDK_BRANCH" "$EDK_REMOTE" "$EDK_DIR"

    cd "$EDK_DIR"
    git submodule update --init
    make -C BaseTools
    cd - 1>/dev/null
  fi
}

setEnv() {
  if [[ -z ${EDK_DIR+x} ]]; then
    cloneEdk
  fi

  PKG_PATH="$PWD"
  PKG_SHORTNAME="OvmfDarwin"
  PKG_NAME="${PKG_SHORTNAME}Pkg"

  FIRMWARE_CODE_BASENAME="OVMF_DARWIN_CODE.fd"
  FIRMWARE_VARS_BASENAME="OVMF_DARWIN_VARS.fd"

  BINARIES_DIR="$PKG_PATH/Binaries"
  CREATE_ZIP="False"
  CREATE_TAR="False"

  PKG_LINK_PATH="$EDK_DIR/$PKG_NAME"
  VERSION=${OVMF_DARWIN_VERSION:-"SNAPSHOT"}
}

cleanup() {
  echo "Doing cleanup..."
  if [[ -d "$BINARIES_DIR" ]]; then
    cd "${BINARIES_DIR:?}"
    rm -rf -- *
    cd -
  fi
}

loadEnvDefaults() {
  ACTIVE_PLATFORM="$PKG_NAME/${PKG_NAME}X64.dsc"
  TARGET_ARCH="X64"
  TARGET="DEBUG"

  case "$(uname)" in
  [Dd]arwin) TOOLCHAIN="XCODE5" ;;
  [Ll]inux) TOOLCHAIN="GCC5" ;;
  *)
    TOOLCHAIN="unknown"
    return 1
    ;;
  esac
}

printHelp() {
  echo -e "edk2 'build' tool wrapper"
  echo -e "usage: ./builduefi.sh [options] [build-args]"
  echo
  echo -e "options:"
  echo -e "\t-help\t\t\tPrint this help"
  echo -e "\t-buildRelease\t\tShortcut for '-b RELEASE'"
  echo -e "\t-debugSerial\t\tShortcut for '-D DEBUG_ON_SERIAL_PORT"
  echo -e "\t-createZip\t\tCreate zip archive with FVs"
  echo -e "\t-createTar\t\tCreate tar archive with FVs"
  echo -e "\t-b TARGET\t\tCustom build target"
  echo -e "\t-p PLATFORMFILE\t\tCustom platform file (*.dsc)"
  echo -e "\t-t TOOLCHAIN\t\tCustom toolchain"
  echo
  echo -e "build-args: will be used as edk2 'build' args"
  echo

  loadEnvDefaults
  ret=$?

  echo -e "defaults:"
  echo -e "\tACTIVE_PLATFORM=$ACTIVE_PLATFORM"
  echo -e "\tBUILDTARGET=$TARGET"
  echo -e "\tTOOLCHAIN=$TOOLCHAIN"
  echo -e "\tTARGET_ARCH=$TARGET_ARCH"

  return $ret
}

createLinks() {
  if [[ ! -d $PKG_LINK_PATH ]]; then
    ln -s "$PKG_PATH" "$PKG_LINK_PATH"
  fi

  cd "$EDK_DIR"

  [[ -d "$PKG_PATH/LegacyPackages" ]] &&
    while read -r LEGACY_PKG_PATH; do
      LEGACY_PKG_LINK="$(basename "$LEGACY_PKG_PATH")"
      [[ -d $LEGACY_PKG_LINK ]] || ln -s "$LEGACY_PKG_PATH" "$LEGACY_PKG_LINK"
    done < <(find "$PKG_PATH/LegacyPackages" -maxdepth 1 -type d)

  cd "$PKG_PATH"

  # Create 'Binaries' dir
  if [[ ! -d "$BINARIES_DIR" ]]; then
    mkdir -p "$BINARIES_DIR"
  fi
}

updEnv() {
  [[ -n $CUSTOM_TARGET_ARCH ]] && TARGET_ARCH="$CUSTOM_TARGET_ARCH"
  [[ -n $CUSTOM_TARGET ]] && TARGET="$CUSTOM_TARGET"
  [[ -n $CUSTOM_ACTIVE_PLATFORM ]] && ACTIVE_PLATFORM="$CUSTOM_ACTIVE_PLATFORM"
  [[ -n $CUSTOM_TOOLCHAIN ]] && TOOLCHAIN="$CUSTOM_TOOLCHAIN"

  FIRMWARE_SRC_DIR="$EDK_DIR/Build/${PKG_SHORTNAME}${TARGET_ARCH}/${TARGET}_${TOOLCHAIN}/FV/"
  FIRMWARE_CODE_SRC="$FIRMWARE_SRC_DIR/$FIRMWARE_CODE_BASENAME"
  FIRMWARE_VARS_SRC="$FIRMWARE_SRC_DIR/$FIRMWARE_VARS_BASENAME"
}

packBinaries() {
  cp "$FIRMWARE_CODE_SRC" "$BINARIES_DIR/$FIRMWARE_CODE_BASENAME"
  cp "$FIRMWARE_VARS_SRC" "$BINARIES_DIR/$FIRMWARE_VARS_BASENAME"

  ARCH_PREFIX="ovmf-darwin-$TARGET_ARCH-$TARGET-$VERSION"

  if [[ "$CREATE_ZIP" == "True" ]]; then
    echo "Creating zip archive..."
    cd "$BINARIES_DIR"
    zip -qr "$BINARIES_DIR/$ARCH_PREFIX.zip" ./*.fd
    cd "$PKG_PATH"
    echo "- Done -"
  fi

  if [[ "$CREATE_TAR" == "True" ]]; then
    echo "Creating tar archive..."
    cd "$BINARIES_DIR"
    tar -czvf "$BINARIES_DIR/$ARCH_PREFIX.tar.gz" ./*.fd
    cd "$PKG_PATH"
    echo "- Done -"
  fi
}

runBuild() {
  cd "$EDK_DIR"

  CMD="build -a $TARGET_ARCH -t $TOOLCHAIN -p $ACTIVE_PLATFORM -b $TARGET $BUILD_COMMAND_ARGS"
  echo "version: $VERSION"
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
    -a)
      CUSTOM_TARGET_ARCH="$2"
      shift
      ;;

    -b)
      CUSTOM_TARGET="$2"
      shift
      ;;

    -p)
      CUSTOM_ACTIVE_PLATFORM="$2"
      shift
      ;;

    -t)
      CUSTOM_TOOLCHAIN="$2"
      shift
      ;;

    -buildRelease)
      CUSTOM_TARGET="RELEASE"
      ;;

    -debugSerial)
      BUILD_COMMAND_ARGS="$BUILD_COMMAND_ARGS -DDEBUG_ON_SERIAL_PORT"
      ;;

    -help)
      printHelp
      exit $?
      ;;

    -createZip)
      CREATE_ZIP="True"
      ;;
    
    -createTar)
      CREATE_TAR="True"
      ;;


    *)
      BUILD_COMMAND_ARGS="$BUILD_COMMAND_ARGS$*"
      break
      ;;
    esac
    shift
  done
}

setEnv
readArgs "$@"
cleanup

if ! loadEnvDefaults && [[ -z "$CUSTOM_TOOLCHAIN" ]]; then
  echo "System type unknown, please provide custom TOOLCHAIN"
  exit 1
fi

updEnv
createLinks

if runBuild; then
  packBinaries
else
  echo "Build errored!"
  exit 1
fi
