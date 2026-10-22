cmake_minimum_required(VERSION 3.23)
project(vglite)

if(NOT DEFINED CACHE{CMAKE_INSTALL_LIBDIR})
    set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "Object code libraries (lib)")
endif()

include(GNUInstallDirs)

# Build flavour selection
# STANDALONE : build against a provided FreeRTOS kernel path (FREERTOS_KERNEL_PATH)
# MCUXSDK    : build against an MCUX SDK path (MCU_SDK_DIR / SdkTopDir)
set(VGLITE_BUILD_FLAVOUR "STANDALONE" CACHE STRING
    "Build flavour: STANDALONE (provided FreeRTOS kernel path) or MCUXSDK (MCUX SDK path)")
set_property(CACHE VGLITE_BUILD_FLAVOUR PROPERTY STRINGS STANDALONE MCUXSDK)

if (VGLITE_BUILD_FLAVOUR STREQUAL "STANDALONE")
    if(NOT DEFINED FREERTOS_KERNEL_PATH)
        if(DEFINED ENV{FREERTOS_KERNEL_PATH})
            set(FREERTOS_KERNEL_PATH "$ENV{FREERTOS_KERNEL_PATH}")
        else()
            set(FREERTOS_KERNEL_PATH "${CMAKE_SOURCE_DIR}/../../../../../FreeRTOS-Kernel")
        endif()
    endif()
    message(STATUS "VGLITE build flavour: STANDALONE (FREERTOS_KERNEL_PATH=${FREERTOS_KERNEL_PATH})")
elseif (VGLITE_BUILD_FLAVOUR STREQUAL "MCUXSDK")
    if (NOT DEFINED MCU_SDK_DIR)
        if (DEFINED ENV{MCU_SDK_DIR})
            set(MCU_SDK_DIR "$ENV{MCU_SDK_DIR}")
        else()
            message(FATAL_ERROR "MCUXSDK flavour requires an MCUX SDK path (set -DMCU_SDK_DIR=... or set the MCU_SDK_DIR environment variable).")
        endif()
    endif()
    set(SdkTopDir ${MCU_SDK_DIR})
    message(STATUS "VGLITE build flavour: MCUXSDK (SdkTopDir=${SdkTopDir})")
else()
    message(FATAL_ERROR "Invalid VGLITE_BUILD_FLAVOUR '${VGLITE_BUILD_FLAVOUR}' (expected STANDALONE or MCUXSDK).")
endif()

set(CMAKE_STATIC_LIBRARY_PREFIX lib)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

include(flags)

# libvglite
###############################################################################
add_library(vglite STATIC)

if (VGLITE_BUILD_FLAVOUR STREQUAL "MCUXSDK")
    target_compile_definitions(vglite PRIVATE TN_MCUXSDK_BUILD=1)

    list(APPEND CMAKE_MODULE_PATH ${SdkTopDir}/rtos/freertos/freertos-kernel/)

    target_include_directories(vglite PRIVATE
        ${SdkTopDir}/devices/MIMXRT2663
        ${SdkTopDir}/devices/MIMXRT2663/periph
        ${SdkTopDir}/CMSIS/Core/Include
        ${SdkTopDir}/rtos/freertos/freertos-kernel/include
        ${SdkTopDir}/rtos/freertos/freertos-kernel-upstream/include/
        ${SdkTopDir}/rtos/freertos/freertos-kernel-upstream/portable/GCC/ARM_CM85_NTZ/non_secure
    )

    target_include_directories(vglite PUBLIC
        $<BUILD_INTERFACE:${SdkTopDir}/CMSIS/Core/Include>
        $<BUILD_INTERFACE:${SdkTopDir}/rtos/freertos/freertos-kernel/include>
        $<BUILD_INTERFACE:${SdkTopDir}/rtos/freertos/freertos-kernel/portable/GCC/ARM_CM85_NTZ/non_secure>
    )
endif()

target_sources(vglite PRIVATE
    #common
    ${CMAKE_SOURCE_DIR}/common/tn_dynarr.c
    ${CMAKE_SOURCE_DIR}/common/tn_gbuf.c
    ${CMAKE_SOURCE_DIR}/common/tn_color.c
    ${CMAKE_SOURCE_DIR}/common/tn_mem.c
    ${CMAKE_SOURCE_DIR}/common/tn_sort.c
    ${CMAKE_SOURCE_DIR}/common/tn_putchar.c
    ${CMAKE_SOURCE_DIR}/common/tn_aver.c
    ${CMAKE_SOURCE_DIR}/common/tn_trace.c
    ${CMAKE_SOURCE_DIR}/common/trace/hooks.c
    ${CMAKE_SOURCE_DIR}/common/tn_io.c
    ${CMAKE_SOURCE_DIR}/common/tn_cmdq_ccdma.c
    ${CMAKE_SOURCE_DIR}/common/tn_cmd_buf_ccdma.c
    ${CMAKE_SOURCE_DIR}/common/tn_fmt.c
    ${CMAKE_SOURCE_DIR}/common/tn_mbuf_alloc.c
    ${CMAKE_SOURCE_DIR}/common/tn_mspace.c
    ${CMAKE_SOURCE_DIR}/common/tn_mspace_allocator_sysheap.c
    ${CMAKE_SOURCE_DIR}/common/tn_mspace_allocator_tlsf.c
    ${CMAKE_SOURCE_DIR}/common/tn_mspace_mapper_identity.c
    ${CMAKE_SOURCE_DIR}/common/tn_mspace_mapper_offset.c
    ${CMAKE_SOURCE_DIR}/common/tn_mspace_pool_ldscript.c
    ${CMAKE_SOURCE_DIR}/common/tn_ctx_img.c
    ${CMAKE_SOURCE_DIR}/common/tn_ctx_api.c
    ${CMAKE_SOURCE_DIR}/common/tn_ctx.c
    ${CMAKE_SOURCE_DIR}/common/tn_m3f32.c
    ${CMAKE_SOURCE_DIR}/common/tn_regs_image.c
    ${CMAKE_SOURCE_DIR}/common/vglite_tin_utils.c
    ${CMAKE_SOURCE_DIR}/common/tn_sync.c
    ${CMAKE_SOURCE_DIR}/common/tn_init.c
    ${CMAKE_SOURCE_DIR}/common/tlsf/tlsf.c
    ${CMAKE_SOURCE_DIR}/common/tn_blt.c
    ${CMAKE_SOURCE_DIR}/common/tn_sysheap.c
    ${CMAKE_SOURCE_DIR}/common/tn_cyc_cnt.c

    #VGLite
    ${CMAKE_SOURCE_DIR}/VGLite/vg_lite_ext.c
    ${CMAKE_SOURCE_DIR}/VGLite/vg_lite_not_supported.c
    ${CMAKE_SOURCE_DIR}/VGLite/vg_lite_image.c

    #HAL
    ${CMAKE_SOURCE_DIR}/HAL/tn_hw.c

    #OSAL
    ${CMAKE_SOURCE_DIR}/OSAL/FreeRTOS/tn_os.c
)

target_include_directories(vglite PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common/tlsf>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/VGLite>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/HAL>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/OSAL>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/OSAL/FreeRTOS>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/misc/FreeRTOS>
)

target_include_directories(vglite INTERFACE
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/HAL>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/common>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/VGLite>"
)

# The library is compiled with -ffunction-sections -fdata-sections; propagate the
# matching garbage collection request so consumers actually drop unused sections.
target_link_options(vglite INTERFACE
    "-Wl,--gc-sections"
)

target_sources(vglite INTERFACE
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
    FILES
    ${CMAKE_SOURCE_DIR}/common/tn_config.h
    ${CMAKE_SOURCE_DIR}/common/tn_types.h
    ${CMAKE_SOURCE_DIR}/common/tn_result.h
    ${CMAKE_SOURCE_DIR}/common/tn_range.h
    ${CMAKE_SOURCE_DIR}/common/tn_gbuf.h
    ${CMAKE_SOURCE_DIR}/HAL/tn_hw.h
    ${CMAKE_SOURCE_DIR}/HAL/tn_gpu_regs.h
    ${CMAKE_SOURCE_DIR}/HAL/tn_gpu_regs_wrap.h
    ${CMAKE_SOURCE_DIR}/VGLite/vg_lite.h
    ${CMAKE_SOURCE_DIR}/VGLite/vg_lite_ext.h
)

if (VGLITE_BUILD_FLAVOUR STREQUAL "STANDALONE")
    #FreeRTOS
    ###########################################################################
    set(FREERTOS_PORT "GCC_ARM_CM85_NTZ_NONSECURE" CACHE STRING "" FORCE)
    set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)

    add_library(freertos_config INTERFACE)
    target_include_directories(freertos_config SYSTEM
        INTERFACE
        .
        ${CMAKE_SOURCE_DIR}/misc/FreeRTOS
    )

    add_subdirectory(${FREERTOS_KERNEL_PATH} FreeRTOS-Kernel)

    target_include_directories(vglite PUBLIC
        $<BUILD_INTERFACE:${FREERTOS_KERNEL_PATH}/include>
    )

    target_link_libraries(vglite PRIVATE
        freertos_kernel
    )

    install(
        TARGETS freertos_kernel freertos_kernel_include freertos_kernel_port_headers freertos_kernel_port
        EXPORT vglite
        ARCHIVE DESTINATION "lib"
    )
endif()

install(
    TARGETS vglite
    EXPORT vglite
    ARCHIVE DESTINATION "lib"
    FILE_SET public_headers
    DESTINATION "include"
)

install(
    EXPORT vglite
    FILE VGLiteTargets.cmake
    DESTINATION "lib/cmake/nxp"
)
