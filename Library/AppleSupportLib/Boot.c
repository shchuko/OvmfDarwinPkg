/** @file
*
*  Copyright (C) 2021, Vladislav Yaroshchuk
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

#include "Boot.h"

/**
 * Try to find Blessed file on Device and return its path into BlessedFilePath
 * @param Device Device to lookup
 * @param BlessedFilePath Found blessed file path
 * @return EFI_SUCCESS if blessed file found, error code on fail
 */
STATIC
EFI_STATUS
GetBlessedFilePath (
  IN  EFI_HANDLE                      Device,
  OUT EFI_DEVICE_PATH_PROTOCOL        **BlessedFilePath
  )
{
  EFI_STATUS                        Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *SimpleFsProtocol;
  EFI_FILE_PROTOCOL                 *Root;
  UINTN                             Size;

  Status = gBS->HandleProtocol (Device, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&SimpleFsProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SimpleFsProtocol->OpenVolume (SimpleFsProtocol, &Root);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Size = 0;
  Status = Root->GetInfo (Root, &gAppleBlessedSystemFileInfoGuid, &Size, NULL);
  if (Status != EFI_BUFFER_TOO_SMALL && EFI_ERROR (Status)) {
    return Status;
  }

  *BlessedFilePath = AllocatePool (Size);
  if (BlessedFilePath == NULL) {
  }

  Status = Root->GetInfo (Root, &gAppleBlessedSystemFileInfoGuid, &Size, *BlessedFilePath);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!IsDevicePathValid (*BlessedFilePath, Size)) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

/**
 * Check if given path exists
 * @param Device Device to lookup
 * @param Path Path in format '\Dir0\Dir1' to be checked
 * @return TRUE if exists, otherwise FALSE
 */
STATIC
BOOLEAN
IsPathExists(
  IN EFI_HANDLE         Device,
  IN CHAR16             *Path
  )
{
  EFI_STATUS                        Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *FileSystem;
  EFI_FILE_PROTOCOL                 *Root;
  EFI_FILE_PROTOCOL                 *Target;
  BOOLEAN                           Result;

  Result = FALSE;

  Status = gBS->HandleProtocol (Device, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&FileSystem);

  if (EFI_ERROR (Status)) {
    goto done;
  }

  Status = FileSystem->OpenVolume (FileSystem, &Root);
  if (EFI_ERROR (Status)) {
    goto done;
  }

  Status = Root->Open (Root, &Target, Path, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    goto close_root;
  }

  Result = TRUE;
  Target->Close(Target);

close_root:
  Root->Close (Root);

done:
  return Result;
}

/**
 * Get boot.efi bootloader path from predefined APPLE_BOOTLOADER_DEFAULT_PATH
 * @param Device Device to lookup
 * @param BootloaderFilePath Found predefined file path
 * @return EFI_SUCCESS if APPLE_BOOTLOADER_DEFAULT_PATH file exists, error code on fail
 */
STATIC
EFI_STATUS
GetPredefinedFilePath(
  IN  EFI_HANDLE                      Device,
  OUT EFI_DEVICE_PATH_PROTOCOL        **BootloaderFilePath
  )
{
  STATIC CHAR16             *DefaultPath = APPLE_BOOTLOADER_DEFAULT_PATH;

  if (!IsPathExists(Device, DefaultPath)) {
    return EFI_NOT_FOUND;
  }

  *BootloaderFilePath = FileDevicePath (Device, DefaultPath);
  return EFI_SUCCESS;
}

/**
 * Attempts to find Apple's boot.efi bootloader expanding media device path.
 * Returns expanded device path if bootloader found, otherwise NULL
 *
 * We try to expand device path into three steps:
 *  1. Request blessed file (bootloader) from device handle. Use it if found.
 *  2. If blessed file not found, lookup predefined path, check for existence and use.
 *  3. If noting found or an error occurred, return NULL
 */
STATIC
EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
GetBootLoaderPathImpl (
  IN  EFI_APPLE_BOOT_PATH_PROTOCOL    *This,
  IN  EFI_HANDLE                      Device
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  DevicePath = NULL;
  // Blessed file check
  Status = GetBlessedFilePath (Device, &DevicePath);
  if (!EFI_ERROR (Status)) {
    return DevicePath;
  }

  // If blessed file not found attempt to find bootloader from default path
  Status = GetPredefinedFilePath(Device, &DevicePath);
  if (!EFI_ERROR (Status)) {
    return DevicePath;
  }
  return NULL;
}

STATIC EFI_APPLE_BOOT_PATH_PROTOCOL mAppleBootPathProtocolImpl = {
    GetBootLoaderPathImpl,
};

/**
 * Installs AppleBootPathProtocol to be used by UefiBootManagerLib BDS phase
 * @param ImageHandle Image handle
 * @return EFI_SUCCESS on success, otherwise InstallMultipleProtocolInterfaces error code
 */
EFI_STATUS
InitializeAppleBoot (
  IN EFI_HANDLE ImageHandle
  )
{
  EFI_STATUS  Status;
  Status = gBS->InstallMultipleProtocolInterfaces (
      &ImageHandle,
      &gEfiAppleBootPathProtocolGuid,
      &mAppleBootPathProtocolImpl,
      NULL
  );
  return Status;
}