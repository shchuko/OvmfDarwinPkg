/** @file
*
*  Copyright (c) 2021, Vladislav Yaroshchuk <yaroshchuk2000@gmail.com>
*  Copyright (c) 2006 - 2009, Intel Corporation All rights reserved.
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include "SmBios.h"

/**
 * Reallocate Smbios table with Memory type EfiReservedMemoryType
 * And placing in E/F Seg
 *
 * @param Table Pointer to the table
 * @return EFI_SUCCESS - Relocate Table successfully, other - failed
 */
STATIC
EFI_STATUS
ReallocSmbiosTable (
    IN OUT SMBIOS_TABLE_ENTRY_POINT **Table
)
{
  SMBIOS_TABLE_ENTRY_POINT *SmbiosTableNew;
  SMBIOS_TABLE_ENTRY_POINT *SmbiosTableOri;
  UINT32                   SmbiosEntryLen;
  UINT32                   BufferLen;
  EFI_PHYSICAL_ADDRESS     BufferPtr;
  EFI_STATUS               Status;

  SmbiosTableNew  = NULL;
  SmbiosTableOri  = *Table;

  //
  // Relocate the Smibos memory
  //
  BufferPtr = EFI_SYSTEM_TABLE_MAX_ADDRESS;
  if (SmbiosTableOri->SmbiosBcdRevision != 0x21) {
    SmbiosEntryLen  = SmbiosTableOri->EntryPointLength;
  } else {
    //
    // According to Smbios Spec 2.4, we should set entry point length as 0x1F if version is 2.1
    //
    SmbiosEntryLen = 0x1F;
  }
  BufferLen = SmbiosEntryLen + SYS_TABLE_PAD(SmbiosEntryLen) + SmbiosTableOri->TableLength;
  Status = gBS->AllocatePages (
      AllocateMaxAddress,
      EfiReservedMemoryType,
      EFI_SIZE_TO_PAGES(BufferLen),
      &BufferPtr
  );
  ASSERT_EFI_ERROR (Status);

  SmbiosTableNew = (SMBIOS_TABLE_ENTRY_POINT *)(UINTN)BufferPtr;
  CopyMem (
      SmbiosTableNew,
      SmbiosTableOri,
      SmbiosEntryLen
  );
  //
  // Get Smbios Structure table address, and make sure the start address is 32-bit align
  //
  BufferPtr += SmbiosEntryLen + SYS_TABLE_PAD(SmbiosEntryLen);
  CopyMem (
      (VOID *)(UINTN)BufferPtr,
      (VOID *)(UINTN)(SmbiosTableOri->TableAddress),
      SmbiosTableOri->TableLength
  );

  SmbiosTableNew->TableAddress = (UINT32)BufferPtr;
  SmbiosTableNew->IntermediateChecksum = 0;
  SmbiosTableNew->IntermediateChecksum =
      CalculateCheckSum8 ((UINT8*)SmbiosTableNew + 0x10, SmbiosEntryLen -0x10);

  //
  // Change the SMBIOS pointer
  //
  *Table = SmbiosTableNew;

  return EFI_SUCCESS;
}

/**
 * Reinstall Smbios table. Makes Smbios can be read by the guest
 *
 * @return EFI_SUCCESS - done successfully, other - failed
 */
EFI_STATUS
InitializeSmBios (
)
{
  EFI_STATUS                  Status;
  SMBIOS_TABLE_ENTRY_POINT    *mOriginalSmbios;

  Status = EfiGetSystemConfigurationTable (
      &gEfiSmbiosTableGuid,
      (VOID **) &mOriginalSmbios
  );
  ASSERT_EFI_ERROR (Status);

  Status = ReallocSmbiosTable (&mOriginalSmbios);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->InstallConfigurationTable (&gEfiSmbiosTableGuid, mOriginalSmbios);
  ASSERT_EFI_ERROR (Status);

  return Status;
}