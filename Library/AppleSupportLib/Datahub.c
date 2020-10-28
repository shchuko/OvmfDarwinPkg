/** @file
*
*  Copyright (c) 2011, Reza Jelveh. All rights reserved.
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

#include "Datahub.h"

EFI_GUID gAppleSystemInfoProducerNameGuid = {0x64517CC8, 0x6561, 0x4051, {0xB0, 0x3C, 0x59, 0x64, 0xB6, 0x0F, 0x4C, 0x7A}};
EFI_GUID gAppleFsbFrequencyPropertyGuid = {0xD1A04D55, 0x75B9, 0x41A3, {0x90, 0x36, 0x8F, 0x4A, 0x26, 0x1C, 0xBB, 0xA2}};
EFI_GUID gAppleDevicePathsSupportedGuid = {0x5BB91CF7, 0xD816, 0x404B, {0x86, 0x72, 0x68, 0xF2, 0x7F, 0x78, 0x31, 0xDC}};

typedef struct {
  UINT32              DataNameSize;
  UINT32              DataSize;
} EFI_PROPERTY_SUBCLASS_RECORD;

typedef struct {
  EFI_SUBCLASS_TYPE1_HEADER   Header;
  EFI_PROPERTY_SUBCLASS_RECORD  Record;
} EFI_PROPERTY_SUBCLASS_DATA;


EFI_STATUS
SetEfiPlatformProperty (
  IN EFI_DATA_HUB_PROTOCOL *DataHub,
  IN CONST EFI_STRING Name,
  EFI_GUID EfiPropertyGuid,
  VOID *Data,
  UINT32 DataSize
  )
{
  EFI_STATUS Status;
  UINT32 DataNameSize;
  EFI_PROPERTY_SUBCLASS_DATA *DataRecord;

  DataNameSize = (UINT32)StrSize(Name);

  DataRecord = AllocateZeroPool (sizeof (EFI_PROPERTY_SUBCLASS_DATA) + DataNameSize + DataSize);
  ASSERT (DataRecord != NULL);

  DataRecord->Header.Version = EFI_DATA_RECORD_HEADER_VERSION;
  DataRecord->Header.HeaderSize = sizeof(EFI_SUBCLASS_TYPE1_HEADER);
  DataRecord->Header.Instance = 0xFFFF;
  DataRecord->Header.SubInstance = 0xFFFF;
  DataRecord->Header.RecordType = 0xFFFFFFFF;
  DataRecord->Record.DataNameSize = DataNameSize;
  DataRecord->Record.DataSize = DataSize;


  CopyMem((UINT8 *)DataRecord + sizeof(EFI_PROPERTY_SUBCLASS_DATA), Name, DataNameSize);
  CopyMem((UINT8 *)DataRecord + sizeof(EFI_PROPERTY_SUBCLASS_DATA) + DataNameSize, Data, DataSize);

  Status = DataHub->LogData(DataHub, &EfiPropertyGuid, &gAppleSystemInfoProducerNameGuid,
                             EFI_DATA_RECORD_CLASS_DATA,
                             DataRecord,
                             sizeof(EFI_PROPERTY_SUBCLASS_DATA) + DataNameSize + DataSize);

  if (DataRecord) {
    gBS->FreePool(DataRecord);
  }

  return Status;
}

/**
  Initialize the DataHub protocols data for the xnu kernel to
  detect an EFI boot.

  @param  ImageHandle   of the loaded driver

  @retval EFI_SUCCESS Source was updated to support Handler.
  @retval EFI_DEVICE_ERROR  Hardware could not be programmed.

**/
EFI_STATUS
EFIAPI
InitializeDatahub (
  IN EFI_HANDLE         ImageHandle
  )
{
  EFI_STATUS                         Status;
  EFI_DATA_HUB_PROTOCOL              *DataHub;

  Status = gBS->LocateProtocol(&gEfiDataHubProtocolGuid, NULL, (VOID **)&DataHub);
  ASSERT_EFI_ERROR(Status);

  UINT64 FsbFrequency = 667000000;
  UINT32 DevicePathsSupported = 1;

  SetEfiPlatformProperty(DataHub, L"FSBFrequency", gAppleFsbFrequencyPropertyGuid, &FsbFrequency, sizeof(UINT64));
  SetEfiPlatformProperty(DataHub, L"DevicePathsSupported", gAppleDevicePathsSupportedGuid, &DevicePathsSupported, sizeof(UINT32));
  ASSERT_EFI_ERROR(Status);

  return Status;
}
