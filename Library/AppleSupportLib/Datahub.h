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

#ifndef _APPLESUPPORT_DATAHUB_H_INCLUDED_
#define _APPLESUPPORT_DATAHUB_H_INCLUDED_

#include "Common.h"

#include <FrameworkDxe.h>

#include <Guid/DataHubRecords.h>

#include <Protocol/DataHub.h>

EFI_STATUS
EFIAPI
InitializeDatahub (
  IN EFI_HANDLE         ImageHandle
  );

#endif
