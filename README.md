![Build](https://github.com/shchuko/OvmfDarwinPkg/workflows/Build/badge.svg?branch=master)
# OvmfDarwinPkg

[OVMF](https://github.com/tianocore/edk2/tree/master/OvmfPkg) clone that provides macOS guests support 
with no third-party bootloaders

## Build notes

Check EDK II documentation to prepare the environment (iasl, nasm required). All the OVMF manuals are 
compatible with this package. Build tested on `edk2-stable202008` version. 

Run this script to perform everything in one command:

```bash
# Build defaults - DEBUG,X64,XCODE5(macOS)/GCC5(Linux)
$ ./builduefi.sh 
```

More examples:

```bash
# Show help
$ ./builduefi.sh -help

# Switch target to RELEASE
$ ./builduefi.sh -buildRelease

# How to pass other build arguments 
$ ./builduefi.sh -D DEBUG_ON_SERIAL_PORT
```

Binaries will be placed in `Build/` directory. The script is a wrapper for EDK `build`: 

```bash
# Simplified './builduefi.sh'
# PWD - ../../OvmfDarwinPkg

# Setup EDK II
git clone https://github.com/tianocore/edk2.git edk2
cd edk2
git checkout edk2-stable202008
git submodule update --init
make -C BaseTools
cd -

# Create symbolic links to packages
ln -s $PWD edk2/OvmfDarwinPkg
ln -s $PWD/LegacyPackages/IntelFrameworkPkg edk2/IntelFrameworkPkg
ln -s $PWD/LegacyPackages/IntelFrameworkModulePkg edk2/IntelFrameworkModulePkg

# Build
cd edk2
source edksetup.sh

# macOS
build -a X64 -t XCODE5 -p OvmfDarwinPkg/OvmfDarwinPkgX64.dsc -b DEBUG

# Linux
build -a X64 -t GCC5 -p OvmfDarwinPkg/OvmfDarwinPkgX64.dsc -b DEBUG
```

## Added components

- [OpenHfsPlus](OpenHfsPlus), based on https://github.com/gsomlo/edk2/tree/gls-hfsplus/OvmfPkg/FswHfsPlus

- [AppleSupportLib](Library/AppleSupportLib), based on https://github.com/fishman/edk2/tree/apple63/OvmfPkg/Library/AppleSupportLib

- [IntelFrameworkPkg](LegacyPackages/IntelFrameworkPkg), from [UDK2018](https://github.com/tianocore/edk2/tree/UDK2018/IntelFrameworkPkg)

- [IntelFrameworkModulePkg](LegacyPackages/IntelFrameworkModulePkg), from [UDK2018](https://github.com/tianocore/edk2/tree/UDK2018/IntelFrameworkModulePkg)

- [UefiBootManagerLib](Library/UefiBootManagerLib) from [edk2-stable202008 MdeModulePkg](https://github.com/tianocore/edk2/tree/edk2-stable202008/MdeModulePkg/Library/UefiBootManagerLib)
  
- Required [Protocols](Include/Protocol/Darwin) and [GUIDes](Include/Guid/Darwin)