# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

# ========================================================================================
# Configuration file for SDK customization at the application level.
#
# This file provides an example of how to reconfigure the SDK to suit the specific
# applications requirements, such as compiler options, linker settings, board-specific files,
# and other project configurations needed for Matter to work.
#
# Structure :
# 1. General configurations (e.g. C/C++ flags)
# 2. Include paths and source files
# 3. Linker configurations
#
# ========================================================================================

if(CONFIG_CHIP_SDK_DEPENDENCIES)
    get_filename_component(NXP_MATTER_SUPPORT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../.. REALPATH)

    # ========================================================================================
    # 1. General Configurations
    # ========================================================================================

    # Common macros
    mcux_add_macro(

        # TODO : check if all these flags are still required & whether it could be defined within SDK
        gMemManagerLightExtendHeapAreaUsage=1
        PRINTF_ADVANCED_ENABLE=1
        __STARTUP_CLEAR_BSS
        __STARTUP_INITIALIZE_NONCACHEDATA
        __STARTUP_INITIALIZE_RAMFUNCTION
        gAspCapability_d=1
        XIP_EXTERNAL_FLASH=1
        XIP_BOOT_HEADER_ENABLE=1
        FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1
        SDK_COMPONENT_INTEGRATION=1
        FSL_DRIVER_TRANSFER_DOUBLE_WEAK_IRQ=0
        HAL_UART_ADAPTER_FIFO=1
        SDK_DEBUGCONSOLE_UART=2
        PRINT_FLOAT_ENABLE=0

        # Required for BT over UART support on IW transceivers
        HAL_UART_DMA_ENABLE=1

        # Disabling BT transceiver initialization as it would be done by the fwk plat coex component
        CONTROLLER_INIT_ESCAPE=1
        __USE_CMSIS
        __GCC
    )

    if(CONFIG_CHIP_NVM_COMPONENT_LITTLEFS)
        mcux_add_macro(

            # Littlefs flags required to support littlefs file system
            LFS_THREADSAFE=1
            LFS_NO_DEBUG
            LFS_NO_ERROR
            LFS_NO_WARN
        )
        mcux_remove_configuration(
            CC "-DLFS_NO_INTRINSICS=1"
            CX "-DLFS_NO_INTRINSICS=1"
        )
    endif()

    if(CONFIG_CHIP_LIB_SHELL)
        mcux_add_macro(

            # If matter CLI is enabled, move debug console to UART2
            DEBUG_CONSOLE_UART_INDEX=2
        )
    endif()

    mcux_add_macro(
        MBEDTLS_USER_CONFIG_FILE=\\\"nxp_matter_mbedtls_config.h\\\"
    )

    # replacing GNU99 to GNU11
    # TMP remove nano.specs until SDK is updated to add it also as a cflag
    mcux_remove_configuration(
        CC "-std=gnu99"
    )

    mcux_add_configuration(
        CC " -Wno-maybe-uninitialized -Wno-unused-variable -march=armv7e-m -std=gnu11 -save-temps"
        CX " -std=gnu++17 -Wno-maybe-uninitialized -Wno-unused-variable -march=armv7e-m -save-temps"
    )

    # ========================================================================================
    # 2. Include Paths and Source Files
    # ========================================================================================
    if(CONFIG_BT)
        mcux_add_source(
            BASE_PATH ${CMAKE_BINARY_DIR}
            PREINCLUDE true
            SOURCES edgefast_bluetooth_config.h
            edgefast_bluetooth_audio_config.h
        )
    endif()

    mcux_add_source(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        SOURCES

        # board files
        examples/platform/rt/rt1170/board/board.c
        examples/platform/rt/rt1170/board/clock_config.c
        examples/platform/rt/rt1170/board/hardware_init.c
        examples/platform/rt/rt1170/board/peripherals.c
        examples/platform/rt/rt1170/board/pin_mux.c
        examples/platform/rt/rt1170/board/sdmmc_config.c
        examples/platform/rt/rt1170/board/sdmmc_config.h
        examples/platform/rt/rt1170/board/wifi_bt_config.c
    )

    mcux_add_include(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        INCLUDES
        examples/platform/rt/rt1170/board
    )

    # Include config files for lwip and wifi
    mcux_add_include(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        INCLUDES gn_build/rt_sdk/transceiver
        gn_build/rt_sdk/lwip/common
        gn_build/mbedtls/config
    )

    # Include lwipopts.h
    if(CONFIG_CHIP_WIFI AND CONFIG_NET_L2_OPENTHREAD)
        mcux_add_include(
            BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
            INCLUDES gn_build/rt_sdk/lwip/wifi_openthread
        )
    elseif(CONFIG_CHIP_WIFI AND NOT CONFIG_NET_L2_OPENTHREAD)
        mcux_add_include(
            BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
            INCLUDES gn_build/rt_sdk/lwip/wifi
        )
    elseif(CONFIG_NET_L2_OPENTHREAD AND NOT CONFIG_CHIP_WIFI)
        mcux_add_include(
            BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
            INCLUDES gn_build/rt_sdk/lwip/openthread
        )
    endif()

    # ========================================================================================
    # 3. Linker Configurations
    # ========================================================================================
    mcux_add_configuration(
        LD "\
    -Wl,--defsym=__stack_size__=2048 \
    -ffreestanding \
    -fno-builtin \
    -u flexspi_config \
    -u image_vector_table \
    -u boot_data \
    -u dcd_data \
    -Wl,--wrap=malloc \
    -Wl,--wrap=free \
    -Wl,--wrap=realloc \
    -Wl,--wrap=calloc \
    -Wl,--wrap=MemoryAlloc \
    -Wl,--wrap=exit \
    -Wl,--wrap=printf \
    -Wl,--wrap=_malloc_r \
    -Wl,--wrap=_realloc_r \
    -Wl,--wrap=_free_r \
    -Wl,--wrap=_calloc_r \
")

    if(CONFIG_BOOTLOADER_MCUBOOT)
        # we need to reserve enough space for the bootloader (MCUBoot)
        # MCUBoot requires 0x40000 Bytes to be reserved at the base of the flash
        # Consequently, some sections will need to be shifted
        mcux_add_configuration(
            LD "-Wl,--defsym=__m_mcuboot_size__=0x40000"
        )
    endif()

    # Here it is required to remove the default linker script added by the SDK build system
    # and include the application linker script
    mcux_remove_armgcc_linker_script(
        TARGETS flash_debug flash_release debug release
        BASE_PATH ${SdkRootDirPath}
        LINKER devices/${soc_portfolio}/${soc_series}/${device}/gcc/${CONFIG_MCUX_TOOLCHAIN_LINKER_DEVICE_PREFIX}_flash.ld
    )

    mcux_remove_armgcc_linker_script(
        TARGETS debug release
        BASE_PATH ${SdkRootDirPath}
        LINKER devices/${soc_portfolio}/${soc_series}/${device}/gcc/${CONFIG_MCUX_TOOLCHAIN_LINKER_DEVICE_PREFIX}_ram.ld
    )

    mcux_add_armgcc_linker_script(
        TARGETS debug release flash_debug flash_release
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        LINKER examples/platform/rt/rt1170/app/ldscripts/MIMXRT1176xxxxx_cm7_flexspi_nor.ld
    )
endif() # CONFIG_CHIP_SDK_DEPENDENCIES

# ========================================================================================
# Notes
# ========================================================================================
# The current file is an example of SDK reconfiguration for Matter applications.
# For further customization, users can extend this structure and adapt it to suit their
# specific needs.
