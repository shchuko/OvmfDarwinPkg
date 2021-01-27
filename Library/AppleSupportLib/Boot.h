/** @file
*
*  Copyright (c) 2021, Vladislav Yaroshchuk. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/


#ifndef _APPLESUPPORT_BOOT_H_INCLUDED_
#define _APPLESUPPORT_BOOT_H_INCLUDED_

#include "Common.h"

#include <PiDxe.h>

#include <Library/DevicePathLib.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/Darwin/AppleBootPath.h>

#include <Guid/FileInfo.h>
#include <Guid/Darwin/AppleBless.h>

#ifndef APPLE_BOOTLOADER_DEFAULT_PATH
  #define APPLE_BOOTLOADER_DEFAULT_PATH L"System\\Library\\CoreServices\\boot.efi"
#endif

/**
 * Installs AppleBootPathProtocol to be used by UefiBootManagerLib BDS phase
 * @param ImageHandle Image handle
 * @return EFI_SUCCESS on success, otherwise InstallMultipleProtocolInterfaces error code
 */
EFI_STATUS
InitializeAppleBoot (
  IN EFI_HANDLE ImageHandle
  );

#endif
