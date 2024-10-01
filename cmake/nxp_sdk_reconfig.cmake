# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

# The purpose of this file is to reconfigure NXP SDK to work with Matter applications.

# ****************************************************************
if(CONFIG_CHIP_SDK_DEPENDENCIES)

get_filename_component(NXP_MATTER_SUPPORT_DIR ${CMAKE_CURRENT_LIST_DIR}/.. REALPATH)

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/nxp_sdk_reconfig.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/nxp_sdk_reconfig.cmake)
endif()

mcux_add_configuration(
    CX "-std=c++17 -std=gnu++17"
)

mcux_add_configuration(
    CC "-Wno-maybe-uninitialized -Wno-unused-variable"
    CX "-Wno-maybe-uninitialized -Wno-unused-variable"
)

mcux_add_configuration(
    LD "\
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

if (CONFIG_CHIP_SECURE_DAC_PRIVATE_KEY_STORAGE OR CONFIG_CHIP_ENABLE_SECURE_WHOLE_FACTORY_DATA)
    mcux_add_macro(
        MBEDTLS_NIST_KW_C
        MBEDTLS_PSA_CRYPTO_CLIENT
    )
endif()

# Temporary workaround until we fully switch to SDK 3.0
# Explicitly include NVS header files from framework middleware
# to keep backward compatibility with SDK 2.16.100
if(CONFIG_MCUX_COMPONENT_middleware.wireless.framework.zephyr_port.nvs)
    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}
        INCLUDES middleware/wireless/framework/zephyr/include/zephyr/settings/
    )
endif() # CONFIG_MCUX_COMPONENT_middleware.wireless.framework.zephyr_port.nvs

# Here it is required to remove the default linker script added by the SDK Next build system
# and include the application linker script in the platform dedicated nxp_sdk_reconfig.cmake
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

endif() # CONFIG_CHIP_SDK_DEPENDENCIES

# *******************************************************