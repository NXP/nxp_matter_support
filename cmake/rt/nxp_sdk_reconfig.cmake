# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if(CONFIG_CHIP_NXP_PLATFORM_RW61X OR CONFIG_CHIP_NXP_PLATFORM_RT1170 OR CONFIG_CHIP_NXP_PLATFORM_RT1060)

if(CONFIG_BT)
    # Common macros
    mcux_add_macro(
        # TODO : these macros should be defined within SDK (tickets open)
        BT_BLE_PLATFORM_INIT_ESCAPE
    )

    # TODO : open ticket to SDK so this can be included within the SDK
    mcux_add_source(PREINCLUDE true
        BASE_PATH ${CMAKE_BINARY_DIR}
        SOURCES edgefast_bluetooth_config_Gen.h
                edgefast_bluetooth_extension_config_Gen.h
                edgefast_bluetooth_audio_config_Gen.h
    )
endif()

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
    SDK_DEBUGCONSOLE_UART=1
    PRINT_FLOAT_ENABLE=0
    __USE_CMSIS
    __GCC
)

target_include_directories(McuxSDK PUBLIC
    # Here we include config files for lwip and wifi
    ${NXP_MATTER_SUPPORT_DIR}/gn_build/rt_sdk/lwip/wifi
    ${NXP_MATTER_SUPPORT_DIR}/gn_build/rt_sdk/lwip/common
    ${NXP_MATTER_SUPPORT_DIR}/gn_build/rt_sdk/transceiver
    # Matter BT/BLE hooks
    ${NXP_MATTER_SUPPORT_DIR}/gn_build/rt_sdk/sdk_hook
)

endif() # CONFIG_CHIP_NXP_PLATFORM_RW61X OR CONFIG_CHIP_NXP_PLATFORM_RT1170 OR CONFIG_CHIP_NXP_PLATFORM_RT1060