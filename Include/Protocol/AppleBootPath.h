/** @file
*
*  Copyright (c) 2021 Vladislav Yaroshchuk. All rights reserved.<BR>
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
* Module Name:
*
*  AppleBootPath.h
*
* Abstract:
*
*  Abstraction of Apple's boot.efi bootloader path retrieval
**/

#ifndef _APPLE_BOOT_PATH_PROTOCOL_H_
#define _APPLE_BOOT_PATH_PROTOCOL_H_

#define EFI_APPLE_BOOT_PATH_PROTOCOL_GUID \
  {0x85e1deb6, 0xd97a, 0x4bea, {0xa0, 0xa0, 0xe6, 0xa9, 0x43, 0x32, 0x85, 0x09}}


typedef struct _EFI_APPLE_BOOT_PATH_PROTOCOL  EFI_APPLE_BOOT_PATH_PROTOCOL;

/**
 * Attempts to find Apple's boot.efi bootloader expanding media device path.
 * Returns expanded device path if bootloader found, otherwise NULL
 */
typedef
EFI_DEVICE_PATH_PROTOCOL *
(EFIAPI *EFI_APPLE_BOOT_PATH_GET_BOOT_LOADER_PATH) (
  IN  EFI_APPLE_BOOT_PATH_PROTOCOL    *This,
  IN  EFI_HANDLE                      Device
  );


struct _EFI_APPLE_BOOT_PATH_PROTOCOL {
  EFI_APPLE_BOOT_PATH_GET_BOOT_LOADER_PATH  GetBootLoaderPath;
};

extern EFI_GUID gEfiAppleBootPathProtocolGuid;

#endif //_APPLE_BOOT_PATH_PROTOCOL_H_
