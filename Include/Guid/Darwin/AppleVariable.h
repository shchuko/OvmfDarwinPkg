/** @file
  Copyright (c) 2020, Vladislav Yaroshchuk <yaroshchuk2000@gmail.com>. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _APPLE_VARIABLE_GUID_H_
#define _APPLE_VARIABLE_GUID_H_

#define APPLE_NVRAM_VARIABLE_GUID \
  {0x7C436110, 0xAB2A, 0x4BBB, {0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82}}

extern EFI_GUID gAppleNVRAMVariableGuid;

#define APPLE_FIRMWARE_VARIABLE_GUID \
  {0x4D1EDE05, 0x38C7, 0x4A6A, {0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14}}

extern EFI_GUID gAppleFirmwareVariableGuid;


#endif //_APPLE_VARIABLE_GUID_H_
