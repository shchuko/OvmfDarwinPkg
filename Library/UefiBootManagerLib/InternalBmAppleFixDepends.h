/** @file
 * 
 * Functions were made library-visible to be reused outside *.c file where declared
 *
 * Copyright (c) 2021, Vladislav Yaroshchuk <yaroshchuk2000@gmail.com>. All rights reserved.
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 * Copyright (c) 2011 - 2020, Intel Corporation. All rights reserved.<BR>
 * (C) Copyright 2015-2016 Hewlett Packard Enterprise Development LP<BR>
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 *
**/

#ifndef _INTERNAL_BM_APPLE_FIX_DEPENDS_H_
#define _INTERNAL_BM_APPLE_FIX_DEPENDS_H_


/**
  Expand File-path device path node to be full device path in platform.

  @param FilePath      The device path pointing to a load option.
                       It could be a short-form device path.
  @param FullPath      The full path returned by the routine in last call.
                       Set to NULL in first call.

  @return The next possible full path pointing to the load option.
          Caller is responsible to free the memory.
**/
EFI_DEVICE_PATH_PROTOCOL *
BmExpandFileDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL    *FilePath,
  IN  EFI_DEVICE_PATH_PROTOCOL    *FullPath
  );

/**
  Expand the media device path which points to a BlockIo or SimpleFileSystem instance
  by appending Apple bootloader path (firstly), or appending EFI_REMOVABLE_MEDIA_FILE_NAME
  if apple bootloader was not found.

  @param DevicePath  The media device path pointing to a BlockIo or SimpleFileSystem instance.
  @param FullPath    The full path returned by the routine in last call.
                     Set to NULL in first call.

  @return The next possible full path pointing to the load option.
          Caller is responsible to free the memory.
**/
EFI_DEVICE_PATH_PROTOCOL *
BmExpandMediaDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL        *DevicePath,
  IN  EFI_DEVICE_PATH_PROTOCOL        *FullPath
  );

#endif //_INTERNAL_BM_APPLE_FIX_DEPENDS_H_
