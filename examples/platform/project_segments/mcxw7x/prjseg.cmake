# Copyright 2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

# NOTE:
# This file is structured to support the board files copy with the "west export_app --bf" tool.
# - Full relative paths to ${SdkRootDirPath} must be used in mcux_add_sources/mcux_add_include.
# - prjseg Kconfig must be evaluated before mcux_add_source/mcux_add_include.
# - Do not refactor into helper functions or use globbing.

if(CONFIG_CHIP_NXP_PLATFORM_MCXW71 OR CONFIG_CHIP_NXP_PLATFORM_MCXW72)
if(CONFIG_MCUX_PRJSEG_module.board.matter.board)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/examples/_common/project_segments/wireless/wireless_mcu
        SOURCES
        board.c
        app_common/app_services_init.c
        app_common/hardware_init.c
        components/board_comp.c
        dcdc/board_dcdc.c
        ext_flash/board_extflash.c
    )

    if("${CHIP_ROOT}" STREQUAL "${SdkRootDirPath}/middleware/matter")
        mcux_add_source(
            BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support
            SOURCES examples/platform/project_segments/mcxw7x/low_power/board_lp.c
        )
    else()
        # Matter not integrated in MCU SDK
        mcux_add_source(
            BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
            SOURCES examples/platform/project_segments/mcxw7x/low_power/board_lp.c
        )
    endif()

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/examples/_boards/${board}/wireless_examples
        SOURCES
        clock_config.c
    )

    if (CONFIG_CHIP_SE05X)
        if("${CHIP_ROOT}" STREQUAL "${SdkRootDirPath}/middleware/matter")
            mcux_add_source(
                BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support
                SOURCES examples/platform/project_segments/mcxw7x/pin_mux.c
            )
        else()
            # Matter not integrated in MCU SDK
            mcux_add_source(
                BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
                SOURCES examples/platform/project_segments/mcxw7x/pin_mux.c
            )
        endif()
    else()
        mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/examples/_boards/${board}/wireless_examples
        SOURCES
        pin_mux.c
    )
    endif()

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
endif()
endif()
