#ifndef _APPLE_DATA_HUB_GUID_H_
#define _APPLE_DATA_HUB_GUID_H_

// APPLE_SYSTEM_INFO_PRODUCER_NAME_GUID
#define APPLE_SYSTEM_INFO_PRODUCER_NAME_GUID \
  { 0x64517CC8, 0x6561, 0x4051,              \
    { 0xB0, 0x3C, 0x59, 0x64, 0xB6, 0x0F, 0x4C, 0x7A } }

// APPLE_FSB_FREQUENCY_PROPERTY_GUID
#define APPLE_FSB_FREQUENCY_PROPERTY_GUID \
  { 0xD1A04D55, 0x75B9, 0x41A3,           \
    { 0x90, 0x36, 0x8F, 0x4A, 0x26, 0x1C, 0xBB, 0xA2 } }

// APPLE_DEVICE_PATHS_SUPPORTED_GUID
#define APPLE_DEVICE_PATHS_SUPPORTED_GUID \
  { 0x5BB91CF7, 0xD816, 0x404B,           \
    { 0x86, 0x72, 0x68, 0xF2, 0x7F, 0x78, 0x31, 0xDC } };

// gAppleSystemInfoProducerNameGuid
extern EFI_GUID gAppleSystemInfoProducerNameGuid;

// gAppleFsbFrequencyPropertyGuid
extern EFI_GUID gAppleFsbFrequencyPropertyGuid;

// gAppleDevicePathsSupportedGuid
extern EFI_GUID gAppleDevicePathsSupportedGuid;

#endif //_APPLE_DATA_HUB_GUID_H_
