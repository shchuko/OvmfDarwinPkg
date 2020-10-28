/** @file
*
*  Copyright (c) 2014, Reza Jelveh. All rights reserved.
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

#include "Console.h"

EFI_STATUS
EFIAPI
GetModeImpl (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL      *This,
  OUT EFI_CONSOLE_CONTROL_SCREEN_MODE   *Mode,
  OUT BOOLEAN                           *GopUgaExists,  OPTIONAL
  OUT BOOLEAN                           *StdInLocked    OPTIONAL
  )
{
  *Mode = EfiConsoleControlScreenGraphics;

  if (GopUgaExists)
    *GopUgaExists = TRUE;
  if (StdInLocked)
    *StdInLocked = FALSE;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SetModeImpl (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL      *This,
  IN  EFI_CONSOLE_CONTROL_SCREEN_MODE   Mode
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LockStdInImpl (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL      *This,
  IN CHAR16                             *Password
  )
{
  return EFI_SUCCESS;
}



EFI_CONSOLE_CONTROL_PROTOCOL gConsoleController =
{
  GetModeImpl,
  SetModeImpl,
};

/**
  Install ConsoleControl protocol, which is needed for Apple's
  boot.efi

  @param  ImageHandle   of the loaded driver

  @retval EFI_SUCCESS       Successfully installed protocol handler
  @retval EFI_DEVICE_ERROR  ConsoleProtocol could not be installed

**/
EFI_STATUS
EFIAPI
InitializeConsoleControl (
  IN EFI_HANDLE         ImageHandle
  )
{
  EFI_STATUS                         Status;

  Status = gBS->InstallMultipleProtocolInterfaces (
      &ImageHandle,
      &gEfiConsoleControlProtocolGuid,
      &gConsoleController,
      NULL
      );

  return Status;
}
