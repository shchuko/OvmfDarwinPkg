/** @file
*
*  Copyright (c) 2020, Vladislav Yaroshchuk <yaroshchuk2000@gmail.com>
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

#ifndef _APPLESUPPORT_LIB_INCLUDED_
#define _APPLESUPPORT_LIB_INCLUDED_


#include <Guid/Darwin/AppleVariable.h>

EFI_STATUS
EFIAPI
InitializeAppleSupport (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
BdsBootApple ();

#endif
