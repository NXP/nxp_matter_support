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

# replacing GNU99 to GNU11
mcux_remove_configuration(
    CC "-std=gnu99"
)

mcux_add_configuration(
    CC " -std=gnu11"
    CX " -std=gnu++17"
)

if(CONFIG_NXP_GENERATE_PREPROCESS_FILES)
    mcux_add_configuration(
        CC "-save-temps"
        CX "-save-temps"
    )
endif()

# Components configuration
mcux_add_macro(
    gAppButtonCnt_c=2
)

mcux_add_configuration(
    LD "\
    -Wl,--defsym=__heap_size__=0 \
    -Wl,--defsym=__stack_size__=0x480 \
    -Wl,--no-warn-rwx-segments \
    -Wl,--wrap=malloc \
    -Wl,--wrap=free \
    -Wl,--wrap=realloc \
    -Wl,--wrap=calloc \
    -Wl,--wrap=MemoryAlloc \
    -Wl,--wrap=_malloc_r \
    -Wl,--wrap=_realloc_r \
    -Wl,--wrap=_free_r \
    -Wl,--wrap=_calloc_r \
    -Wl,--defsym=gUseNVMLink_d=1 \
")

mcux_add_configuration(
    CX "\
    -Wno-register \
")

# TODO check if all are needed
mcux_add_macro(
    SDK_COMPONENT_INTEGRATION=1
    gSerialManagerMaxInterfaces_c=1
    gAppHighSystemClockFrequency_d=1
    gAppLedCnt_c=2
    USE_NBU=1
    gAspCapability_d=1
    gNvStorageIncluded_d=1
    gUnmirroredFeatureSet_d=1
    gNvFragmentation_Enabled_d=1
    gAppLowpowerEnabled_d=1
    MULTICORE_APP=1
    K32W_LOG_ENABLED
)

# TODO core defines. Check if all are needed
mcux_add_macro(
    __STARTUP_CLEAR_BSS
    SERIAL_USE_CONFIGURE_STRUCTURE=1
    SDK_OS_FREE_RTOS
    HAL_RPMSG_SELECT_ROLE=0
    TM_ENABLE_TIME_STAMP=1
    USE_RTOS=1
    FSL_RTOS_FREE_RTOS=1
    gMemManagerLightExtendHeapAreaUsage=0
    ENABLE_RAM_VECTOR_TABLE=1
    NO_SYSCORECLK_UPD=0
    DEBUG_SERIAL_INTERFACE_INSTANCE=0
    BOARD_DEBUG_UART_CLK_FREQ=6000000U
    gLoggingActive_d=0
    gLogRingPlacementOffset_c=0xF000
)

if(CONFIG_CHIP_NXP_PLATFORM_MCXW71)
    mcux_add_macro(
        # Temporary workaround, allocate more heap
        MinimalHeapSize_c=0x9200
)
endif()

if(CONFIG_CHIP_NXP_PLATFORM_MCXW72)
    # TODO: The SSSAPI component's dependencies should be relocated into
    # the component's makefiles.
    mcux_add_macro(
        MBEDTLS_NXP_SSSAPI
        MBEDTLS_THREADING_C
        MBEDTLS_THREADING_ALT
        # Temporary workaround, allocate more heap
        MinimalHeapSize_c=0xC800
        DEFAULT_APP_UART=1
        gDebugConsoleEnable_d=1
        gUartDebugConsole_d=1
        DebugConsole_c=1
        gMainThreadPriority_c=5
        gMainThreadStackSize_c=3096
    )

    # TODO: The SSSAPI component's dependencies should be relocated into
    # the component's makefiles.
    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}
        INCLUDES
        middleware/mbedtls/include
        middleware/mbedtls/port/sssapi
    )

endif()

# BLE configuration
if(CONFIG_CHIP_SDK_DEPENDENCIES_BLE_HOST)
    mcux_add_macro(
        gAppMaxConnections_c=1
        gUseHciTransportDownward_d=1
        gL2caMaxLeCbChannels_c=2
        gGapSimultaneousEAChainedReports_c=0
        gAppUseBonding_d=0
        gAppUsePairing_d=0
        gAppUsePrivacy_d=0
        gGattUseUpdateDatabaseCopyProc_c=0
        gBleBondIdentityHeaderSize_c=56
        gPasskeyValue_c=999999
        gHost_TaskStackSize_c=2400
        gBleSetMacAddrFromVendorCommand_d=1
        mAdvertisingDefaultTxPower_c=0 # default advertising TX power
        mConnectionDefaultTxPower_c=0 # default connection TX power
        BLE_HIGH_TX_POWER=0 # when enabled overwrite default tx power with following values gAdvertisingPowerLeveldBm_c and gConnectPowerLeveldBm_c
        gAdvertisingPowerLeveldBm_c=0
        gConnectPowerLeveldBm_c=0
        gTmrStackTimers_c=7 # 3 + gAppMaxConnections_c * 2 + gL2caMaxLeCbChannels_c + gGapSimultaneousEAChainedReports_c
    )
endif()

# SSS config
mcux_add_macro(
    SSS_CONFIG_FILE=\\\"fsl_sss_config_elemu.h\\\"
    SSCP_CONFIG_FILE=\\\"fsl_sscp_config_elemu.h\\\"
)

# ========================================================================================
# 2. Include Paths and Source Files
# ========================================================================================

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}/examples/_common/project_segments/wireless/wireless_mcu
    SOURCES
    board.c
    app_common/app_services_init.c
    app_common/hardware_init.c
    components/board_comp.c
    dcdc/board_dcdc.c
    ext_flash/board_extflash.c
    low_power/board_lp.c
)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}/examples/_boards/${board}/wireless_examples
    SOURCES
    clock_config.c
    pin_mux.c
)

mcux_add_source(
    BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
    SOURCES
    examples/platform/common/ble/ble_function_mux.c
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES
    examples/_boards/${board}/wireless_examples
    examples/_common/project_segments/wireless/wireless_mcu
    examples/_common/project_segments/wireless/wireless_mcu/app_common
    examples/_common/project_segments/wireless/wireless_mcu/components
    examples/_common/project_segments/wireless/wireless_mcu/dcdc
    examples/_common/project_segments/wireless/wireless_mcu/ext_flash
    examples/_common/project_segments/wireless/wireless_mcu/low_power
)

mcux_add_include(
    BASE_PATH ${CHIP_ROOT}
    INCLUDES

    # Temporary path for mbedtls config file location
    .

    # Temporary path for gatt_uuid128.h file
    third_party/nxp/nxp_matter_support/examples/platform/common/ble

    # Temporary path for FreeRTOS config file
    third_party/nxp/nxp_matter_support/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/app/project_include/freeRTOS
)

# ========================================================================================
# 3. Linker Configurations
# ========================================================================================

# Here it is required to remove the default linker script added by the SDK Next build system
# and include the application linker script
mcux_remove_armgcc_linker_script(
    TARGETS debug release flash_debug flash_release
    BASE_PATH ${SdkRootDirPath}
    LINKER devices/${soc_portfolio}/${soc_series}/${device}/gcc/${CONFIG_MCUX_TOOLCHAIN_LINKER_DEVICE_PREFIX}_flash.ld
)

mcux_remove_armgcc_linker_script(
    TARGETS debug release
    BASE_PATH ${SdkRootDirPath}
    LINKER devices/${soc_portfolio}/${soc_series}/${device}/gcc/${CONFIG_MCUX_TOOLCHAIN_LINKER_DEVICE_PREFIX}_ram.ld
)

#Extract file name and directory path to be usable by mcux cmake function
get_filename_component(MATTER_DEFAULT_LINKER_FILE_PATH "${CONFIG_MATTER_DEFAULT_LINKER_FILE_PATH}" DIRECTORY)
get_filename_component(MATTER_DEFAULT_LINKER_FILE_NAME "${CONFIG_MATTER_DEFAULT_LINKER_FILE_PATH}" NAME)

mcux_add_armgcc_linker_script(
    TARGETS debug release flash_debug flash_release
    BASE_PATH ${MATTER_DEFAULT_LINKER_FILE_PATH}
    LINKER ${MATTER_DEFAULT_LINKER_FILE_NAME}
)

# ========================================================================================
# Notes
# ========================================================================================
# The current file is an example of SDK reconfiguration for Matter applications.
# For further customization, users can extend this structure and adapt it to suit their
# specific needs.
