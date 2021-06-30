#!/bin/bash
##
# invokes './builduefi.sh' inside docker
#
# All passed arguments are redirected to 'builduefi.sh'
#
# Copyright (c) 2021, Vladislav Yaroshchuk <yaroshchuk2000@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

DOCKER_IMAGE="shchuko/edk2-env:edk2-stable202105"
DOCKER_UEFI_SOURCES_PATH="/home/dummyuser/SomeEdkPkg"
BUILDSCRIPT="$DOCKER_UEFI_SOURCES_PATH/builduefi.sh"

docker run --rm -it \
  -v "$(pwd):$DOCKER_UEFI_SOURCES_PATH" \
  "$DOCKER_IMAGE" \
  "$BUILDSCRIPT" "$@"
