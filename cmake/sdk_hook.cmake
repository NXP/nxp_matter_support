# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

# Configuration CMake file for SDK hooks

if(CONFIG_CHIP_DEVICE_USE_ZEPHYR_BLE)
    # Include Matter BT/BLE hooks
    mcux_add_include(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        INCLUDES gn_build/rt_sdk/sdk_hook
    )
endif() # CONFIG_CHIP_DEVICE_USE_ZEPHYR_BLE

if(CONFIG_CHIP_FACTORY_DATA AND CONFIG_MCUX_COMPONENT_middleware.mbedtls.port.els_pkc)
    mcux_add_include(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        INCLUDES gn_build/rt_sdk/sdk_hook/els_pkc
    )
    mcux_add_source(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        SOURCES gn_build/rt_sdk/sdk_hook/els_pkc/ELSFactoryData.c
    )
    mcux_add_configuration(
        CC "-Wno-address"
        CX "-Wno-address"
    )
endif()
