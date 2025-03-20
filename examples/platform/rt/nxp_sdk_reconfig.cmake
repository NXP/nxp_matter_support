# Copyright 2024-2025 NXP
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

if(NOT DEFINED NXP_MATTER_SUPPORT_DIR)
    get_filename_component(NXP_MATTER_SUPPORT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../.. REALPATH)
endif()

# ========================================================================================
# 1. General Configurations
# ========================================================================================

# Common macros
mcux_add_macro(
    SDK_DEBUGCONSOLE_UART
    PRINTF_ADVANCED_ENABLE=1
    __STARTUP_CLEAR_BSS
    __STARTUP_INITIALIZE_NONCACHEDATA
    __STARTUP_INITIALIZE_RAMFUNCTION
    XIP_EXTERNAL_FLASH=1
    XIP_BOOT_HEADER_ENABLE=1
    FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1
    FSL_DRIVER_TRANSFER_DOUBLE_WEAK_IRQ=0
    HAL_UART_ADAPTER_FIFO=1
    __USE_CMSIS
)

mcux_add_macro(
    MBEDTLS_USER_CONFIG_FILE=\\\"nxp_matter_mbedtls_config.h\\\"
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

# RT/RW platform-specific macros
if(CONFIG_CHIP_NXP_PLATFORM_RW61X)
    mcux_add_macro(
        IMU_TASK_STACK_SIZE=1024
        gPlatformDisableBleLowPower_d=1
        MBEDTLS_NIST_KW_C
    )

    if(CONFIG_BT)
        mcux_add_macro(
            BT_PLATFORM
        )
    endif()

    if(CONFIG_CHIP_LIB_SHELL)
        mcux_add_macro(

            # If matter CLI is enabled, move debug console to UART0
            DEBUG_CONSOLE_UART_INDEX=0
        )
    endif()

    mcux_add_configuration(
        CC "-march=armv8-m.main"
        CX "-march=armv8-m.main"
    )
endif()

if(CONFIG_CHIP_NXP_PLATFORM_RT1170)
    mcux_add_macro(

        # Required for BT over UART support on IW transceivers
        HAL_UART_DMA_ENABLE=1

        # Disabling BT transceiver initialization as it would be done by the fwk plat coex component
        CONTROLLER_INIT_ESCAPE=1
    )

    if(CONFIG_CHIP_LIB_SHELL)
        mcux_add_macro(

            # If matter CLI is enabled, move debug console to UART2
            DEBUG_CONSOLE_UART_INDEX=2
        )
    endif()

    mcux_add_configuration(
        CC "-march=armv7e-m"
        CX "-march=armv7e-m"
    )
endif()

if(CONFIG_CHIP_NXP_PLATFORM_RT1060)
    mcux_add_macro(

        # Required for BT over UART support on IW transceivers
        HAL_UART_DMA_ENABLE=1

        # Disabling BT transceiver initialization as it would be done by the fwk plat coex component
        CONTROLLER_INIT_ESCAPE=1
    )

    if(CONFIG_CHIP_LIB_SHELL)
        if(CONFIG_MCUX_HW_BOARD_evkcmimxrt1060)
            mcux_add_macro(

                # When the CLI is enabled the debug console should be moved in UART8 for RT1060_EVKC
                DEBUG_CONSOLE_UART_INDEX=8
            )
        else()
            mcux_add_macro(

                # When the CLI is enabled the debug console should be moved in UART2 for RT1060_EVKB
                DEBUG_CONSOLE_UART_INDEX=2
            )
        endif()
    endif()
endif()

if(CONFIG_CHIP_NXP_PLATFORM_RT1060 OR CONFIG_CHIP_NXP_PLATFORM_RT1170)
    mcux_add_macro(
        # Needed as MCU boot uses remapping, image trailers
        # are expected at a different offset than when it uses swap mode.
        gPlatformMcuBootUseRemap_d=1
    )
endif()

if(CONFIG_MCUX_COMPONENT_component.wifi_bt_module.IW61X AND CONFIG_NET_L2_OPENTHREAD AND NOT CONFIG_CHIP_WIFI)
    # Required to use wifi_nxp fw dwnld in MoT
    # edgefast is forcing the usage of a dedicated wifi_config.h when wifi is disabled, the one from matter cannot be used.
    # Therefore flags coming from components/wifi_bt_module/incl/wifi_bt_module_config.h are not defined and need to manually set.
    
    mcux_add_macro(
        SD9177
    )
    if(CONFIG_CHIP_NXP_PLATFORM_RT1170)
        mcux_add_macro(
            WIFI_BT_USE_USD_INTERFACE
        )
    endif()
    if(CONFIG_CHIP_NXP_PLATFORM_RT1060)
        mcux_add_macro(
            WIFI_BT_USE_M2_INTERFACE
        )
    endif()
endif()

if(CONFIG_MCUX_COMPONENT_component.wifi_bt_module.K32W061_transceiver)
    mcux_add_macro(
        K32W061_TRANSCEIVER
        BOARD_OTW_K32W0_PIN_INIT

        # SDK define required for OTW support
        HAL_UART_DMA_ENABLE=1

        # Required to use wifi_nxp fw dwnld in MoT
        SD8987

        # Required to override hci_uart_state in hci uart wrapper
        EM_HAVE_STATIC_DECL=0
    )

    if(CONFIG_CHIP_NXP_PLATFORM_RT1060 AND CONFIG_MCUX_HW_BOARD_evkbmimxrt1060)
        mcux_add_macro(

            # Framework defines to support hdlc over UART on UART3 and a reset pin
            SPINEL_UART_INSTANCE=3
            SPINEL_ENABLE_RX_RTS=1
            SPINEL_ENABLE_TX_RTS=1

            # OTW configurations
            OTW_DIO5_PIN_PORT=1
            OTW_DIO5_PIN_NUM=26
            OTW_RESET_PIN_PORT=1
            OTW_RESET_PIN_NUM=27
        )
    endif()
endif()

# replacing GNU99 to GNU11
mcux_remove_configuration(
    CC "-std=gnu99"
)

mcux_add_configuration(
    CC " -Wno-maybe-uninitialized -Wno-unused-variable -Wno-address -std=gnu11 -save-temps"
    CX " -std=gnu++17 -Wno-maybe-uninitialized -Wno-unused-variable -Wno-address -save-temps"
)

# ========================================================================================
# 2. Include Paths and Source Files
# ========================================================================================
if(CONFIG_BT)
    mcux_add_source(
        BASE_PATH ${CMAKE_BINARY_DIR}
        PREINCLUDE TRUE
        SOURCES edgefast_bluetooth_config.h
        edgefast_bluetooth_audio_config.h
    )
endif()

# Add board files
file(GLOB BOARD_FILES
    "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/${board}/*.c"
    "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/${board}/*.h"
    "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/*.c"
    "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/*.h"
)

# Remove peripherals files if not LittleFS FileSytem
if(NOT CONFIG_CHIP_NVM_COMPONENT_LITTLEFS)
    list(REMOVE_ITEM BOARD_FILES 
        "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/peripherals.c"
        "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/peripherals.h" 
    )
endif()

mcux_add_source(
    BASE_PATH /
    SOURCES
    ${BOARD_FILES}
    ${NXP_MATTER_SUPPORT_DIR}/gn_build/rt_sdk/transceiver/wifi_config.h
)

mcux_add_include(
    BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
    INCLUDES
    examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/${board}
    examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board
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
elseif(CONFIG_CHIP_ETHERNET)
    mcux_add_include(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        INCLUDES gn_build/rt_sdk/lwip/ethernet
    )
    mcux_add_macro(
    TOOLCHAINS iar armgcc
    CC "-DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE"
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
    -u qspiflash_config \
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

if(CONFIG_MCUX_COMPONENT_component.wifi_bt_module.K32W061_transceiver)
    # Allows to wrap hci_uart functions to another implementation.
    # The goal is to have hci and spinel encapsulated in hdlc frames.
    # To view the content of new implementations, the code of spinel_hci_hdlc.cpp should be checked
    mcux_add_configuration(
        LD "\
        -Wl,--defsym,hci_uart_write_data=__wrap_hci_uart_write_data \
        -Wl,--wrap=hci_uart_bt_init\
        -Wl,--wrap=hci_uart_bt_shutdown \
        -Wl,--wrap=hci_uart_init \
    ")
endif()

if(CONFIG_BOOTLOADER_MCUBOOT)
    # We need to reserve enough space for the bootloader (MCUBoot).
    # CONFIG_CHIP_MCUBOOT_SIZE defines the required size in Bytes
    # to be reserved at the base of the flash for each platform.
    # Consequently, some sections will need to be shifted.
    mcux_add_configuration(
        LD " -Wl,--defsym=__m_mcuboot_size__=${CONFIG_CHIP_MCUBOOT_SIZE} "
    )
endif()

if(CONFIG_MCUX_HW_BOARD_evkcmimxrt1060)
    mcux_add_configuration(
        LD " -Wl,--defsym=gFlashSize_d=0x1000000 "
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
    BASE_PATH /
    LINKER ${CONFIG_MATTER_DEFAULT_LINKER_FILE_PATH}
)

# ========================================================================================
# Notes
# ========================================================================================
# The current file is an example of SDK reconfiguration for Matter applications.
# For further customization, users can extend this structure and adapt it to suit their
# specific needs.
