SET(CMAKE_ASM_FLAGS " \
    ${CMAKE_ASM_FLAGS} \
    -D__STARTUP_CLEAR_BSS \
    -DMCUXPRESSO_SDK \
    -DCPU_MIMXRT2663CVVAA \
    -DMIMXRT2663_SERIES \
    -mthumb \
    -mcpu=cortex-m85 \
    -mfloat-abi=hard -mfpu=fpv5-d16 \
")
SET(CMAKE_ASM_FLAGS_DEBUG " \
    ${CMAKE_ASM_FLAGS_DEBUG} \
    -g \
")
SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
")
SET(CMAKE_C_FLAGS " \
    ${CMAKE_C_FLAGS} \
    -DENABLE_RAM_VECTOR_TABLE=1 \
    -D__STARTUP_CLEAR_BSS \
    -DUSE_PSRAM=1 \
    -DSKIP_SYSCLK_INIT=1 \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -D__STARTUP_INITIALIZE_QADATA \
    -D__STARTUP_INITIALIZE_RAMFUNCTION \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DMCUX_META_BUILD \
    -DMCUXPRESSO_SDK \
    -DXIP_EXTERNAL_FLASH=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DUSE_PSRAM_W958D6=1 \
    -DST7796S_DATA_WITDH=16 \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DBOARD_USE_PCAL6524=1 \
    -DBOARD_USE_PCA9555=1 \
    -DCPU_MIMXRT2663CVVAA \
    -DMIMXRT2663_SERIES \
    -DMCUX_DBI_LEGACY=0 \
    -D_GNU_SOURCE=1 \
    -DCMSDK_CM85=1 \
    -DTN_SDK_DEBUG_CONSOLE=1 \
    -DSDK_OS_FREE_RTOS \
    -Wno-unused-function \
    -fno-if-conversion \
    -fno-if-conversion2 \
    -Wno-error=array-bounds \
    -mcmse \
    -Werror \
    -fstack-usage \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -std=gnu99 \
    -mcpu=cortex-m85 \
    -Wno-error=attributes \
    -fdiagnostics-color=auto \
    -ffreestanding \
    -mabi=aapcs \
    -Winline \
    -mfloat-abi=hard -mfpu=fpv5-d16 \
    -DSDK_DEBUGCONSOLE=1 \
")
SET(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    -DDEBUG \
    -g3 \
    -O0 \
")
SET(CMAKE_C_FLAGS_RELEASE " \
    ${CMAKE_C_FLAGS_RELEASE} \
    -DNDEBUG \
    -O3 \
")
