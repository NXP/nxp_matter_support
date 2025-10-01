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

if((CMAKE_BUILD_TYPE STREQUAL "debug") AND (CONFIG_CHIP_NXP_PLATFORM_MCXW71))
    # MCXW71 doesn't have enough memory to afford "-O0" optimization level
    # in debug mode therefore we have to switch it to something which has
    # a lower memory footprint, like "-Og"
    mcux_remove_configuration(
        CC "-O0"
        CX "-O0"
    )
    mcux_add_configuration(
        CC "-Og"
        CX "-Og"
    )
endif()

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
    -Wl,--defsym=lp_ram_lower_limit=0x04000000 \
    -Wl,--defsym=lp_ram_upper_limit=0x2001C000 \
")

# Note: <lp_ram_lower_limit> and <lp_ram_upper_limit> are used by the
#       connectivity framework in order to calculate which RAM banks
#       are required to be retained in low power mode and which banks
#       can be switched off in order to save power.

mcux_add_configuration(
    CX "\
    -Wno-register \
")

# TODO check if all are needed
mcux_add_macro(
    SDK_COMPONENT_INTEGRATION=1
    gSerialManagerMaxInterfaces_c=1
    gAppHighSystemClockFrequency_d=1
    USE_NBU=1
    gAspCapability_d=1
    gNvStorageIncluded_d=1
    gUnmirroredFeatureSet_d=1
    gNvFragmentation_Enabled_d=1
    MULTICORE_APP=1
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
    DEFAULT_APP_UART=1
    DEBUG_SERIAL_INTERFACE_INSTANCE=0
    BOARD_DEBUG_UART_CLK_FREQ=96000000U
    gLoggingActive_d=0
    gLogRingPlacementOffset_c=0xF000
)

# MbedTLS configuration
mcux_add_macro(
    MBEDTLS_USER_CONFIG_FILE=\\\"nxp_matter_mbedtls_config.h\\\"
)
mcux_add_include(
    BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
    INCLUDES
    gn_build/mbedtls/config
)
# MCXW71 and MCXW72 platforms do not support full crypto acceleration via mbedtls
mcux_remove_macro(
    MBEDTLS_NXP_ELE200
)
# MCXW72 does support, however, accelerating the AES CCM
if(CONFIG_CHIP_NXP_PLATFORM_MCXW72)
if(CONFIG_CHIP_MBEDTLS_2X)
    mcux_add_macro(
        MBEDTLS_CCM_ALT
    )
elseif(CONFIG_CHIP_MBEDTLS_3X)
    mcux_add_macro(
        # Needed inside secure-subsystem\port\kw45_k4w1\sss_init.c
        # to enable CRYPTO_ConfigureThreadingMcux at init. If the
        # threading support is not enabled the Matter init will fail
        MBEDTLS_NXP_SSSAPI
    )
endif()
endif()

if(CONFIG_CHIP_LIB_SHELL)
    mcux_add_macro(
        -DgAppUseSerialManager_c=1
    )
else()
    mcux_add_macro(
        BOARD_DEBUG_UART_INSTANCE=1
    )
endif()

if(CONFIG_CHIP_NXP_PLATFORM_MCXW71)
    mcux_add_macro(
        # Temporary workaround, allocate more heap
        MinimalHeapSize_c=0x9200
    )
endif()

if(CONFIG_CHIP_NXP_PLATFORM_MCXW72)
    mcux_add_macro(
        # Temporary workaround, allocate more heap
        MinimalHeapSize_c=0xC800
        gMainThreadPriority_c=5
        gMainThreadStackSize_c=3096
    )
endif()

if(CONFIG_NXP_USE_LOW_POWER)
    mcux_add_macro(
        nxp_use_low_power=1
        K32W_LOG_ENABLED=0
        gUartDebugConsole_d=0
        cPWR_UsePowerDownMode=1
        gAppLowpowerEnabled_d=1
    )
else()
    mcux_add_macro(
        gAppLedCnt_c=2
        K32W_LOG_ENABLED
    )
    if(CONFIG_CHIP_NXP_PLATFORM_MCXW72)
        mcux_add_macro(
            gDebugConsoleEnable_d=1
            gUartDebugConsole_d=1
            DebugConsole_c=1
        )
    endif()
endif()

# BLE configuration
if(CONFIG_CHIP_SDK_DEPENDENCIES_BLE_HOST)
    if (CONFIG_CHIP_NXP_MULTIPLE_BLE_CONNECTIONS)
        mcux_add_macro(
            gAppMaxConnections_c=3
            MAX_PLATFORM_SUPPORTED_CONNECTIONS=3
        )
    else()
        mcux_add_macro(
            gAppMaxConnections_c=1
            MAX_PLATFORM_SUPPORTED_CONNECTIONS=1
        )
    endif()

    if (CONFIG_CHIP_SDK_DEPENDENCIES_BLE_HOST_CS)
        mcux_add_macro(
            gAppRasDataTransfer_d=1
            gAppRunAlgo_d=1
            gcGapMaximumActiveConnections_c=gAppMaxConnections_c
            gGattCaching_d=0
            gBLE_ChannelSounding_d=1
            gAppIsPeripheral_d=1
            gRasRREQ_d=1
            gAppUseRADEAlgorithm_d=0
            gAppUseCDEAlgorithm_d=1
            gBLE42_d=1
            gBLE50_d=1
            gBLE51_d=1
            gBLE52_d=1
        )
    endif()

    if (CONFIG_CHIP_NXP_BLE_PAIRING)
        mcux_add_macro(
            gAppUsePairing_d=1
            gCentralInitiatedPairing_d=1
        )
    else()
        mcux_add_macro(
            gAppUsePairing_d=0
        )
    endif()

    if (CONFIG_CHIP_NXP_BLE_BONDING)
        mcux_add_macro(
            gAppUseBonding_d=1
        )
    else()
        mcux_add_macro(
            gAppUseBonding_d=0
        )
    endif()

    mcux_add_macro(
        gUseHciTransportDownward_d=1
        gL2caMaxLeCbChannels_c=2
        gGapSimultaneousEAChainedReports_c=0
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
# The MCXW72 platform is using the threading implementation from SSS
if(CONFIG_CHIP_NXP_PLATFORM_MCXW72)
    mcux_add_macro(
        MBEDTLS_THREADING_C
        MBEDTLS_THREADING_ALT
    )
endif()

if (CONFIG_CHIP_FACTORY_DATA)
    mcux_add_configuration(
        LD "-Wl,--defsym=gUseFactoryData_d=1"
    )
    mcux_add_macro(
        gHwParamsAppFactoryDataExtension_d=1
    )
    if(CONFIG_CHIP_NXP_PLATFORM_MCXW71)
        mcux_add_macro(
            gHwParamsProdDataPlacement_c=gHwParamsProdDataMainFlashMode_c
        )
    endif()
endif()

# ========================================================================================
# 2. Include Paths and Source Files
# ========================================================================================
# Add board files
include(${NXP_MATTER_SUPPORT_DIR}/examples/platform/project_segments/mcxw7x/prjseg.cmake)

mcux_add_source(
    BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
    SOURCES
    examples/platform/common/ble/ble_function_mux.c
)

# Include app_preinclude_common.h to get BleBond size macros
if (CONFIG_CHIP_NXP_MULTIPLE_BLE_CONNECTIONS)
    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/middleware/wireless/bluetooth
        INCLUDES
        boards/${board}
    )
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}
        SOURCES middleware/wireless/bluetooth/boards/${board}/app_preinclude_common.h
    )
    mcux_add_source(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        SOURCES
        examples/platform/common/ble/ble_nvm_bonding.c
    )
endif()

mcux_add_include(
    BASE_PATH ${CHIP_ROOT}
    INCLUDES

    # Temporary path for gatt_uuid128.h file
    third_party/nxp/nxp_matter_support/examples/platform/common/ble

    # Temporary path for FreeRTOS config file
    third_party/nxp/nxp_matter_support/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/app/project_include/freeRTOS
)

if (CONFIG_CHIP_SDK_DEPENDENCIES_BLE_HOST_CS)
    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/middleware/wireless/bluetooth_cs
        INCLUDES
        application/common/lcl
        profiles/ranging
        host/interface
        localization/lcl/pde/algo/interface
    )
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/middleware/wireless/bluetooth_cs
        SOURCES
        application/common/lcl/app_localization.c
        application/common/lcl/app_localization_algo.c
        application/common/lcl/ranging_client.c
        profiles/ranging/ranging_service.c

    )
    mcux_add_library(
        BASE_PATH ${SdkRootDirPath}/middleware/wireless/bluetooth_cs
        LIBS host/lib/lib_channel_sounding_gcc.a
        DSP DSP
        FPU SP_FPU
        TOOLCHAINS mcux armgcc
    )
    mcux_add_library(
        BASE_PATH ${SdkRootDirPath}/middleware/wireless/bluetooth_cs
        LIBS localization/lib/lib_lcl_algo_cm33_gcc.a
        TOOLCHAINS mcux armgcc
    )
endif()

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
