/** @file
  Library functions with Apple bootloader device path fixes.

  Copyright (c) 2021, Vladislav Yaroshchuk <yaroshchuk2000@gmail.com>. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#include "InternalBmAppleFix.h"

/**
 * Try to locate the Path and split it into locatable prefix and remaining suffix
 *
 * @param Path                  Path to analyze
 * @param LocatablePathPrefix   Path prefix can be located by the LocateDevicePath().
 *                              Should be freed manually by the caller
 * @param NotLocatableSuffix    Remaining part that leaves after LocatablePathPrefix.
 *
 */
STATIC
VOID
BmTryLocateDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Path,
  OUT EFI_DEVICE_PATH_PROTOCOL  **LocatablePathPrefix,
  OUT EFI_DEVICE_PATH_PROTOCOL  **NotLocatableSuffix
  )
{
  ASSERT (Path != NULL);
  ASSERT (IsDevicePathValid (Path, 0));

  EFI_DEVICE_PATH_PROTOCOL        *NodeIterator;
  EFI_HANDLE                      Handle;

  *NotLocatableSuffix = Path;
  gBS->LocateDevicePath (
      &gEfiDevicePathProtocolGuid,
      NotLocatableSuffix,
      &Handle
    );

  // A shortcut for case when all the path can be located
  if (IsDevicePathEnd (*NotLocatableSuffix)) {
    *LocatablePathPrefix = DuplicateDevicePath (Path);
    return;
  }

  // Find the first not-locatable node
  *LocatablePathPrefix = NULL;
  NodeIterator = Path;
  while (!IsDevicePathEnd (NodeIterator) && NodeIterator != *NotLocatableSuffix) {
    *LocatablePathPrefix = AppendDevicePathNode (*LocatablePathPrefix, NodeIterator);
    NodeIterator = NextDevicePathNode (NodeIterator);
  }
}

/**
 * Probe to read a file located by FullPath
 *
 * @param FullPath Full path of file
 * @return TRUE if probe successful, otherwise false
 */
STATIC
BOOLEAN
BmProbeFullDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL *FullPath
  )
{
  ASSERT (FullPath != NULL);
  ASSERT (IsDevicePathValid (FullPath, 0));

  VOID                                  *FileBuffer;
  UINTN                                 LocalFileSize;
  UINT32                                AuthenticationStatus;
  LocalFileSize = 0;
  FileBuffer  = NULL;

  FileBuffer = GetFileBufferByFilePath (TRUE, FullPath, &LocalFileSize, &AuthenticationStatus);
  if (FileBuffer == NULL) {
    return FALSE;
  }

  FreePool (FileBuffer);
  return TRUE;
}

/**
 * Get the first DevicePath node of specific type and sub-type.
 *
 * @param DevicePath Device path to search into.
 * @param NodeType Node type to be found.
 * @param NodeSubType Node sub-type to be found.
 * @return Pointer to found DevicePath node if found, otherwise NULL.
 */
STATIC
EFI_DEVICE_PATH_PROTOCOL *
BmDevicePathGetFirstNodeOfType (
  IN EFI_DEVICE_PATH_PROTOCOL   *DevicePath,
  IN UINT8                      NodeType,
  IN UINT8                      NodeSubType
  )
{
  ASSERT (DevicePath != NULL);
  ASSERT (IsDevicePathValid (DevicePath, 0));

  EFI_DEVICE_PATH_PROTOCOL *NodeIterator;

  NodeIterator = DevicePath;
  while (!IsDevicePathEnd (NodeIterator)) {
    if (DevicePathType (NodeIterator) == NodeType && DevicePathSubType (NodeIterator) == NodeSubType) {
      return NodeIterator;
    }
    NodeIterator = NextDevicePathNode (NodeIterator);
  }

  return NULL;
}

/**
 * Get last node of DevicePath. The result points directly to input device path.
 *
 * @param DevicePath The path from which to pull the last node.
 * @return Pointer to the last node of DevicePath.
 */
STATIC
EFI_DEVICE_PATH_PROTOCOL *
BmGetLastDevicePathNode (
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath
  )
{
  ASSERT (DevicePath != NULL);
  ASSERT (IsDevicePathValid (DevicePath, 0));

  EFI_DEVICE_PATH_PROTOCOL *NodeIterator;
  EFI_DEVICE_PATH_PROTOCOL *LastNode;

  NodeIterator = DevicePath;
  LastNode = DevicePath;

  while (!IsDevicePathEnd (NodeIterator)) {
    LastNode = NodeIterator;
    NodeIterator = NextDevicePathNode (NodeIterator);
  }

  return LastNode;
}

/**
 * Checks is DevicePath matches specified type and sub-type.
 *
 * @param  DevicePath   Device path to check.
 * @param  NodeType     Node type.
 * @param  NodeSubType  Node sub-type.
 * @return TRUE if matches, otherwise false.
 */
STATIC
BOOLEAN
BmIsDevicePathMatchesType (
  IN EFI_DEVICE_PATH_PROTOCOL   *DevicePath,
  IN UINT8                      NodeType,
  IN UINT8                      NodeSubType
  )
{
  ASSERT (DevicePath != NULL);
  ASSERT (IsDevicePathValid (DevicePath, 0));

  return DevicePathType (DevicePath) == NodeType && DevicePathSubType (DevicePath) == NodeSubType;
}

/**
 * Compares EFI_DEVICE_PATH_PROTOCOL nodes.
 *
 * @param First     First node.
 * @param Second    Second node.
 * @return TRUE if nodes are equal, FALSE if not
 */
STATIC
BOOLEAN
BmDevicePathNodesEqual (
  IN EFI_DEVICE_PATH_PROTOCOL *First,
  IN EFI_DEVICE_PATH_PROTOCOL *Second
  )
{
  ASSERT (First != NULL);
  ASSERT (Second != NULL);
  ASSERT (IsDevicePathValid (First, 0));
  ASSERT (IsDevicePathValid (Second, 0));

  return DevicePathType (First) == DevicePathType (Second) &&
          DevicePathSubType (First) == DevicePathSubType (Second) &&
          CompareMem (First, Second, DevicePathNodeLength (First)) == 0;
}

/**
 * Checks is DevicePath starts from Prefix.
 *
 * @param DevicePath    Device path to check the prefix of.
 * @param Prefix        Expected prefix.
 * @return TRUE if DevicePath starts from Prefix, FALSE if not.
 */
STATIC
BOOLEAN
BmIsDeviceStartsFrom (
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath,
  IN EFI_DEVICE_PATH_PROTOCOL *Prefix
)
{
  ASSERT (DevicePath != NULL);
  ASSERT (Prefix != NULL);
  ASSERT (IsDevicePathValid (DevicePath, 0));
  ASSERT (IsDevicePathValid (Prefix, 0));

  return CompareMem (DevicePath, Prefix, GetDevicePathSize (Prefix) - END_DEVICE_PATH_LENGTH) == 0;
}

/**
 * Validate FullFilePath for meeting the rules:
 * - starts from SourcePathPrefix;
 * - file can be read by the FullFilePath;
 * - contains special nodes (if specified).
 *
 * @param FullFilePath Full path to file to validate.
 * @param Prefix to match path with.
 * @param ... Special EFI_DEVICE_PATH_PROTOCOL nodes must present in path. Args must be terminated with NULL.
 * @return TRUE if the path matches all the rules, FALSE if not.
 */
STATIC
BOOLEAN
BmValidateFilePath (
  IN EFI_DEVICE_PATH_PROTOCOL *FullFilePath,
  IN EFI_DEVICE_PATH_PROTOCOL *Prefix,
  ...
  )
{
  ASSERT (FullFilePath != NULL);
  ASSERT (Prefix != NULL);
  ASSERT (IsDevicePathValid (FullFilePath, 0));
  ASSERT (IsDevicePathValid (Prefix, 0));

  EFI_DEVICE_PATH_PROTOCOL        *RequiredNode;
  EFI_DEVICE_PATH_PROTOCOL        *FoundNode;
  VA_LIST                         VaArgsList;

  DEBUG ((DEBUG_INFO, "[Bds AppleFix] Validate path: \n"));
  BmPrintDp(FullFilePath);
  DEBUG ((DEBUG_INFO, "\n"));

  // Prefix match check
  if (!BmIsDeviceStartsFrom (FullFilePath, Prefix)) {
    DEBUG ((DEBUG_INFO, "[Bds AppleFix] Validate path fail: prefix mismatch\n"));
    return FALSE;
  }

  // Required path nodes must present
  VA_START (VaArgsList, Prefix);
  for (RequiredNode = VA_ARG (VaArgsList, EFI_DEVICE_PATH_PROTOCOL *);
        RequiredNode != NULL;
        RequiredNode = VA_ARG (VaArgsList, EFI_DEVICE_PATH_PROTOCOL *)
        ) {

    ASSERT (IsDevicePathValid (RequiredNode, 0));

    FoundNode = BmDevicePathGetFirstNodeOfType (
      FullFilePath,
      DevicePathType (RequiredNode),
      DevicePathSubType (RequiredNode)
      );

    if (FoundNode == NULL || !BmDevicePathNodesEqual (RequiredNode, FoundNode)) {
      DEBUG ((DEBUG_INFO, "[Bds AppleFix] Validate path fail: required node not found\n"));
      VA_END (VaArgsList);
      return FALSE;
    }
  }
  VA_END (VaArgsList);

  // File can be loaded
  if (!BmProbeFullDevicePath (FullFilePath)) {
    DEBUG ((DEBUG_INFO, "[Bds AppleFix] Validate path fail: could not open file\n"));
    return FALSE;
  }

  DEBUG ((DEBUG_INFO, "[Bds AppleFix] Validate path success!\n"));
  return TRUE;
}

/**
 * Try to recreate Apple bootloader path from non-correct existing ('source') path's content:
 * - existing path trailing 'MEDIA_VENDOR_DP' node, ex. \<GUID>\System\CoreServices\boot.efi
 * - existing path specific VenMedia() node (represents APFS Container partition)
 *
 * This required because Apple may add/remove some nodes when storing bootloader DP into NVRAM,
 * especially when using VirtIO Block Device
 *
 * @param FullPath  Path to use as source. On success will be replaced with recreated correct one.
 *                  Must have MEDIA_FILEPATH_DP as trailing node and contain MEDIA_VENDOR_DP node.
 * @return EFI_SUCCESS on success, otherwise error code.
 */
STATIC
EFI_STATUS
BmTryRecreateAppleDevicePathByTrailingFilepath (
  IN OUT EFI_DEVICE_PATH_PROTOCOL  **FullPath
  )
{
  ASSERT (FullPath != NULL);
  ASSERT (*FullPath != NULL);
  ASSERT (IsDevicePathValid (*FullPath, 0));

  EFI_DEVICE_PATH_PROTOCOL      *Prefix;
  EFI_DEVICE_PATH_PROTOCOL      *Suffix;

  EFI_DEVICE_PATH_PROTOCOL      *LastFullPathNode;
  EFI_DEVICE_PATH_PROTOCOL      *VenMediaDevPathNode;
  EFI_DEVICE_PATH_PROTOCOL      *ExpandedDevicePath;
  EFI_DEVICE_PATH_PROTOCOL      *PrevExpandedDevicePath;

  EFI_STATUS                    Status;

  DEBUG ((DEBUG_INFO, "[Bds AppleFix] Recreate Apple bootloader path: %a\n", __FUNCTION__));
  VenMediaDevPathNode = BmDevicePathGetFirstNodeOfType (*FullPath, MEDIA_DEVICE_PATH, MEDIA_VENDOR_DP);
  if (VenMediaDevPathNode == NULL) {
    DEBUG ((DEBUG_INFO, "[Bds AppleFix] VenMedia(..) node not found\n"));
    return EFI_NOT_FOUND;
  }

  BmTryLocateDevicePath (*FullPath, &Prefix, &Suffix);

  LastFullPathNode = BmGetLastDevicePathNode (*FullPath);
  ASSERT (BmIsDevicePathMatchesType (LastFullPathNode, MEDIA_DEVICE_PATH, MEDIA_FILEPATH_DP));

  ExpandedDevicePath = NULL;
  PrevExpandedDevicePath = NULL;
  Status = EFI_NOT_FOUND;

  for (;;) {
    ExpandedDevicePath = BmExpandFileDevicePath (LastFullPathNode, PrevExpandedDevicePath);

    if (PrevExpandedDevicePath != NULL) {
      FreePool (PrevExpandedDevicePath);
    }
    PrevExpandedDevicePath = ExpandedDevicePath;

    if (ExpandedDevicePath == NULL) {
      break;
    }

    if (BmValidateFilePath (ExpandedDevicePath, Prefix, VenMediaDevPathNode, NULL)) {
      FreePool (*FullPath);
      *FullPath = ExpandedDevicePath;
      Status = EFI_SUCCESS;
      break;
    }
  }

  FreePool (Prefix);
  return Status;
}

/**
 * Try to expand DevicePath to full Apple bootloader path using AppleBless.
 *
 *
 * @param DevicePath Path to expand.
 * @return Expanded path if successful, NULL on fail.
 */
STATIC
EFI_DEVICE_PATH_PROTOCOL *
BmExpandDevicePathWithAppleBless (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  ASSERT (DevicePath != NULL);
  ASSERT (IsDevicePathValid (DevicePath, 0));

  EFI_APPLE_BOOT_PATH_PROTOCOL  *AppleBootPathProtocol;
  EFI_HANDLE                    Handle;
  EFI_STATUS                    Status;

  Status = gBS->LocateProtocol (
    &gEfiAppleBootPathProtocolGuid,
    NULL,
    (VOID *) &AppleBootPathProtocol
  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "[Bds AppleFix] EFI_APPLE_BOOT_PATH_PROTOCOL could not be located\n"));
    return NULL;
  }

  Status = gBS->LocateDevicePath (&gEfiBlockIoProtocolGuid, &DevicePath, &Handle);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "[Bds AppleFix] DevicePath could not be located:\n"));
    BmPrintDp (DevicePath);
    DEBUG ((DEBUG_INFO, "\n"));
    return NULL;
  }

  gBS->ConnectController (Handle, NULL, NULL, TRUE);
  return AppleBootPathProtocol->GetBootLoaderPath (
    AppleBootPathProtocol,
    Handle
  );
}

/**
 * Try to recreate Apple bootloader path expanding prefix of known non-correct path.
 * Input path must contain MEDIA_HARDDRIVE_DP node.
 *
 * This required because Apple may add/remove some nodes when storing bootloader DP into NVRAM,
 * especially when using VirtIO Block Device.
 *
 * @param FullPath Path to use as source. On success will be replaced with recreated correct one.
 * @return EFI_SUCCESS on success, otherwise error code.
 */
STATIC
EFI_STATUS
BmTryRecreateAppleDevicePathByPrefix (
  IN OUT EFI_DEVICE_PATH_PROTOCOL  **DevicePath
  )
{
  ASSERT (DevicePath != NULL);
  ASSERT (*DevicePath != NULL);
  ASSERT (IsDevicePathValid (*DevicePath, 0));

  EFI_DEVICE_PATH_PROTOCOL      *Prefix;
  EFI_DEVICE_PATH_PROTOCOL      *Suffix;
  EFI_DEVICE_PATH_PROTOCOL      *HdDevPathNode;
  EFI_DEVICE_PATH_PROTOCOL      *BlessExpandedPath;
  EFI_STATUS                    Status;

  Status = EFI_NOT_FOUND;
  DEBUG ((DEBUG_INFO, "[Bds AppleFix] Recreate Apple bootloader path: %a\n", __FUNCTION__));

  // Find DevicePath's locatable prefix
  BmTryLocateDevicePath (*DevicePath, &Prefix, &Suffix);

  // Locate the MEDIA_HARDDRIVE_DP node: HD(...)
  HdDevPathNode = BmDevicePathGetFirstNodeOfType (*DevicePath, MEDIA_DEVICE_PATH, MEDIA_HARDDRIVE_DP);
  ASSERT (HdDevPathNode != NULL);

  // Append MEDIA_HARDDRIVE_DP node to prefix and expand the path using AppleBless
  BlessExpandedPath  = BmExpandDevicePathWithAppleBless (AppendDevicePathNode (Prefix, HdDevPathNode));
  if (BlessExpandedPath != NULL && BmValidateFilePath (BlessExpandedPath, Prefix, HdDevPathNode, NULL)) {
    FreePool (*DevicePath);
    *DevicePath = BlessExpandedPath;
    Status = EFI_SUCCESS;
  }

  FreePool (Prefix);
  return Status;
}

/**
 * Try to recreate Apple bootloader path from known non-locatable DevicePath
 *
 * @param   DevicePath Path to use as source. On success will be replaced with recreated correct one.
 * @return  EFI_SUCCESS on success, otherwise error code.
 */
STATIC
EFI_STATUS
BmTryRecreateAppleDevicePath (
  IN OUT EFI_DEVICE_PATH_PROTOCOL  **DevicePath
  )
{
  ASSERT (DevicePath != NULL);
  ASSERT (*DevicePath != NULL);
  ASSERT (IsDevicePathValid (*DevicePath, 0));

  EFI_DEVICE_PATH_PROTOCOL      *LastDevicePathNode;

  LastDevicePathNode = BmGetLastDevicePathNode (*DevicePath);

  if (BmIsDevicePathMatchesType (LastDevicePathNode, MEDIA_DEVICE_PATH, MEDIA_FILEPATH_DP)) {
    DEBUG ((DEBUG_INFO, "[Bds AppleFix] %a: probable attempt to boot from APFS volume\n", __FUNCTION__));
    return BmTryRecreateAppleDevicePathByTrailingFilepath (DevicePath);
  }

  if (BmIsDevicePathMatchesType (LastDevicePathNode, MEDIA_DEVICE_PATH, MEDIA_HARDDRIVE_DP)) {
    DEBUG ((DEBUG_INFO, "[Bds AppleFix] %a: probable attempt to boot from HFS+ volume\n", __FUNCTION__));
    return BmTryRecreateAppleDevicePathByPrefix (DevicePath);
  }

  return EFI_NOT_FOUND;
}

/**
 * Fix Apple DevicePath nodes in accordance with UEFI Specs
 *
 * @param DevicePath DevicePath to fix
 */
STATIC
VOID
BmFixAppleDevicePathNodes (
  IN EFI_DEVICE_PATH_PROTOCOL  **DevicePath
  )
{
  ASSERT (DevicePath != NULL);
  ASSERT (*DevicePath != NULL);

  EFI_DEVICE_PATH_PROTOCOL *SubPath = *DevicePath;
  EFI_DEV_PATH_PTR         Node;

  while (!IsDevicePathEnd (SubPath)) {
    Node.DevPath = SubPath;

    if (BmIsDevicePathMatchesType (SubPath, MESSAGING_DEVICE_PATH, MSG_SATA_DP)) {
      if (Node.Sata->PortMultiplierPortNumber != 0xFFFF) {
        // Must be set to 0xFFFF if the device is directly connected to the host bus adapter.
        // This rule has been established by UEFI Specs and has not been followed by Apple.
        Node.Sata->PortMultiplierPortNumber = 0xFFFF;
      }
    }

    SubPath = NextDevicePathNode (SubPath);
  }
}

/**
  Wrap BmGetNextLoadOptionBuffer to fix UEFI-unsupported Apple BootOption DevicePath.
  Applies series of fixes to Apple's DevicePath.

  @param Type      The load option type
  @param FilePath  The device path pointing to a load option.
  @param FullPath  Return the next full device path after expanding. Caller is responsible to free it.
  @param FileSize  Return the load option size.
  @return          The load option buffer. Caller is responsible to free the memory.
 */
VOID *
BmGetNextLoadOptionBufferWithAppleFix (
  IN  EFI_BOOT_MANAGER_LOAD_OPTION_TYPE Type,
  IN  EFI_DEVICE_PATH_PROTOCOL          *FilePath,
  OUT EFI_DEVICE_PATH_PROTOCOL          **FullPath,
  OUT UINTN                             *FileSize
  )
{
  ASSERT (FilePath != NULL);
  ASSERT (IsDevicePathValid (FilePath, 0));

  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *FixedFilePath;
  VOID                      *FileBuffer;
  FileBuffer = BmGetNextLoadOptionBuffer (Type, FilePath, FullPath, FileSize);
  if (FileBuffer != NULL) {
    return FileBuffer;
  }

  DEBUG ((DEBUG_INFO, "[Bds AppleFix] Apple Device Path: Attempting to fix DP Nodes\n"));
  FixedFilePath = DuplicateDevicePath (FilePath);
  ASSERT (FixedFilePath != NULL);

  BmFixAppleDevicePathNodes (&FixedFilePath);
  FileBuffer = BmGetNextLoadOptionBuffer (Type, FixedFilePath, FullPath, FileSize);
  if (FileBuffer != NULL) {
    goto done_success;
  }

  Status = BmTryRecreateAppleDevicePath (&FixedFilePath);
  if (EFI_ERROR (Status)) {
    goto done_fail;
  }

  FileBuffer = BmGetNextLoadOptionBuffer (Type, FixedFilePath, FullPath, FileSize);
  if (FileBuffer == NULL) {
    goto done_fail;
  }

done_success:
  FreePool (FixedFilePath);
  DEBUG ((DEBUG_INFO, "[Bds AppleFix] Apple DevicePath fixes apply successful\n"));
  return FileBuffer;

done_fail:
  FreePool (FixedFilePath);
  DEBUG ((DEBUG_INFO, "[Bds AppleFix] Apple DevicePath fixes apply fail\n"));
  return NULL;
}
