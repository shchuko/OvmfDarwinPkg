/** @file
 *
 * Internal functions with Apple bootloader device path fixes.
 *
 * Copyright (c) 2021, Vladislav Yaroshchuk <yaroshchuk2000@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 *
**/

#ifndef _INTERNAL_BM_APPLE_FIX_H_
#define _INTERNAL_BM_APPLE_FIX_H_

#include "InternalBm.h"
#include "InternalBmAppleFixDepends.h"

/**
  Wrap BmGetNextLoadOptionBuffer to fix UEFI-unsupported Apple BootOption DevicePath.
  Applies series of fixes to Apple's DevicePath.

  @param Type      The load option type
  @param FilePath  The device path pointing to a load option. Could be short-form
  @param FullPath  Return the next full device path after expanding. Caller is responsible to free it.
  @param FileSize  Return the load option size.
  @return          The load option buffer. Caller is responsible to free the memory.
 */
VOID *
BmGetNextLoadOptionBufferWithAppleFix (
  IN  EFI_BOOT_MANAGER_LOAD_OPTION_TYPE Type,
  IN  EFI_DEVICE_PATH_PROTOCOL          *FilePath,
  OUT EFI_DEVICE_PATH_PROTOCOL          **FullPath,
  OUT UINTN                             *FileSize
  );

#endif //_INTERNAL_BM_APPLE_FIX_H_
