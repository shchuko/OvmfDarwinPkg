/** @file
*
*  Copyright (c) 2021, Vladislav Yaroshchuk <yaroshchuk2000@gmail.com>
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

#ifndef _APPLESUPPORT_SMBIOS_H_INCLUDED_
#define _APPLESUPPORT_SMBIOS_H_INCLUDED_

#include "Common.h"

#include <Guid/SmBios.h>
#include <IndustryStandard/SmBios.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>

#define EFI_SYSTEM_TABLE_MAX_ADDRESS 0xFFFFFFFF
#define SYS_TABLE_PAD(ptr) (((~ptr) +1) & 0x07 )

EFI_STATUS
InitializeSmBios (
);

#endif //_APPLESUPPORT_SMBIOS_H_INCLUDED_
