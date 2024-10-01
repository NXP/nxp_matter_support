# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if(CONFIG_CHIP_NXP_PLATFORM_RW61X)

include(${NXP_MATTER_SUPPORT_DIR}/cmake/rt/nxp_sdk_reconfig.cmake)

# TODO : This can be moved to the build_helpers.cmake as it is a pre-build step
# Monolithic app property
# convert fw binaries into src files for CPU1/CPU2
# this requires -DgPlatformMonolithicApp_d=1 to be defined in flags.cmake
find_package(Python COMPONENTS Interpreter REQUIRED)

get_filename_component(FW_BIN_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin REALPATH)

SET(FW_BIN_ARGS "-t"  "sb" ${FW_BIN_PATH})

add_custom_command(OUTPUT
    ${FW_BIN_PATH}/A2/fw_cpu1.c
    ${FW_BIN_PATH}/A2/fw_cpu2_ble.c
    ${FW_BIN_PATH}/A2/fw_cpu2_combo.c
    COMMAND python ${SdkRootDirPath}/components/conn_fwloader/script/fw_bin2c_conv.py ${FW_BIN_ARGS}
    COMMENT "Generating fw_cpu1.c , fw_cpu2_ble.c , fw_cpu2_combo.c"
)

add_custom_target(convert_fw_bin2c ALL
    DEPENDS
    ${FW_BIN_PATH}/A2/fw_cpu1.c
    ${FW_BIN_PATH}/A2/fw_cpu2_ble.c
    ${FW_BIN_PATH}/A2/fw_cpu2_combo.c
)

add_dependencies(McuxSDK convert_fw_bin2c)

mcux_add_macro(
    IMU_TASK_STACK_SIZE=1024
    gPlatformDisableBleLowPower_d=1 # TODO Add fwk kconfig for this
    APP_FLEXSPI_AMBA_BASE=0x08000000
)

if(CONFIG_BT)
    mcux_add_macro(
        BT_PLATFORM
    )
endif()

mcux_add_configuration(
    LD "\
    -Wl,--defsym=__stack_size__=2048 \
    -ffreestanding \
    -fno-builtin \
    -u flexspi_config \
    -u image_vector_table \
    -u boot_data \
    -u dcd_data \
")

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES 
        # cpu1/cpu2 fw binaries for monolithic app
        components/conn_fwloader/fw_bin/A2/fw_cpu1.c
        components/conn_fwloader/fw_bin/A2/fw_cpu2_ble.c
        components/conn_fwloader/fw_bin/A2/fw_cpu2_combo.c
)

if(CONFIG_MATTER_APP_DEFAULT_BOARD_FILES)
    mcux_add_source(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        SOURCES
        # board files
        examples/platform/rt/rw61x/board/${board}/board.c
        examples/platform/rt/rw61x/board/${board}/clock_config.c
        examples/platform/rt/rw61x/board/hardware_init.c
        examples/platform/rt/rw61x/board/peripherals.c
        examples/platform/rt/rw61x/board/pin_mux.c
    )

    mcux_add_include(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        INCLUDES
        examples/platform/rt/rw61x/board/${board}
        examples/platform/rt/rw61x/board
    )
endif() # CONFIG_MATTER_APP_DEFAULT_BOARD_FILES

if(CONFIG_CHIP_LIB_SHELL)
    mcux_add_macro(
        # If matter CLI is enabled, move debug console to UART0    
        DEBUG_CONSOLE_UART_INDEX=0
    )
endif()

if(CONFIG_BOOTLOADER_MCUBOOT)
    # we need to reserve enough space for the bootloader (MCUBoot)
    # MCUBoot requires 0x20000 Bytes to be reserved at the base of the flash
    # Consequently, some sections will need to be shifted
    mcux_add_configuration(
        LD "-Wl,--defsym=__m_mcuboot_size__=0x20000"
    )
endif()

# Add the platform linker script for matter applications
mcux_add_armgcc_linker_script(
    TARGETS debug release flash_debug flash_release
    BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
    LINKER examples/platform/rt/rw61x/app/ldscripts/RW610_flash.ld
)

endif()