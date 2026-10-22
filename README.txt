HOW TO BUILD VGLITE LIBRARY
===========================

Prerequisites
-------------
- ARM GNU toolchain (arm-none-eabi)
- FreeRTOS-Kernel repository OR MCUX SDK archive
- CMake >= 3.23
- make

Build steps
-----------
1. Set the path to the ARM GNU toolchain:

       export ARMGCC_DIR=/path/to/arm-gnu-toolchain

   Example:
       export ARMGCC_DIR=/opt/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi

2. For standalone builds referencing a FreeRTOS-Kernel:
2a. Set the path to the FreeRTOS-Kernel repository:

       export FREERTOS_KERNEL_PATH=/path/to/FreeRTOS-Kernel

2b. Configure with CMake (optionally also pass VGLITE_BUILD_FLAVOUR which
defaults to STANDALONE):

       cmake -D CMAKE_TOOLCHAIN_FILE='cmake/armgcc.cmake' -D VGLITE_BUILD_FLAVOUR=STANDALONE .

3. For MCUX SDK builds:
3a. Set the path to an MCUX SDK archive directory:

	export MCU_SDK_DIR=/path/to/mcux/archive

3b. Configure with CMake, setting VGLITE_BUILD_FLAVOUR=MCUXSDK:

	cmake -D CMAKE_TOOLCHAIN_FILE="${MCU_SDK_DIR}/tools/cmake_toolchain_files/armgcc.cmake" -D VGLITE_BUILD_FLAVOUR=MCUXSDK .

4. Build:

       make
