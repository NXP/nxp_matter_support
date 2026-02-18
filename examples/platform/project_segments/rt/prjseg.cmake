# Copyright 2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

# NOTE:
# This file is structured to support the board files copy with the "west export_app --bf" tool.
# - Full relative paths to ${SdkRootDirPath} must be used in mcux_add_sources/mcux_add_include.
# - prjseg Kconfig must be evaluated before mcux_add_source/mcux_add_include.
# - Do not refactor into helper functions or use globbing.
# - This is only supported if Matter is integrated into the MCUX SDK.

if("${CHIP_ROOT}" STREQUAL "${SdkRootDirPath}/middleware/matter")
# ============================================================
# RW61X Platform
# ============================================================
if(CONFIG_CHIP_NXP_PLATFORM_RW61X)
if(CONFIG_MCUX_PRJSEG_module.board.matter.board)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support/examples/platform/rt/rw61x/board/
        SOURCES
            ${board}/board.c
            ${board}/board_comp.c
            ${board}/clock_config.c
            hardware_init.c
            pin_mux.c 
    )
    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support/examples/platform/rt/rw61x/board/
        INCLUDES ${board}/
            .
    )
    if(CONFIG_MCUX_PRJSEG_module.board.matter.peripherals)
        mcux_add_source(
            BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support/examples/platform/rt/rw61x/board/
            SOURCES
                peripherals.c
        )
    endif()
endif()
# ============================================================
# RT1170 Platform
# ============================================================
elseif(CONFIG_CHIP_NXP_PLATFORM_RT1170)
if(CONFIG_MCUX_PRJSEG_module.board.matter.board)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support/examples/platform/rt/rt1170/board/
        SOURCES
            board.c
            clock_config.c
            hardware_init.c
            pin_mux.c
            sdmmc_config.c
            wifi_bt_config.c
    )
    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support/examples/platform/rt/rt1170/board/
        INCLUDES .
    )
    if(CONFIG_MCUX_PRJSEG_module.board.matter.peripherals)
        mcux_add_source(
            BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support/examples/platform/rt/rt1170/board/
            SOURCES
                peripherals.c
        )
    endif()
endif()
# ============================================================
# RT1060 Platform
# ============================================================
elseif(CONFIG_CHIP_NXP_PLATFORM_RT1060)
if(CONFIG_MCUX_PRJSEG_module.board.matter.board)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support/examples/platform/rt/rt1060/board/
        SOURCES
            board.c
            clock_config.c
            hardware_init.c
            ${board}/pin_mux.c
            ${board}/sdmmc_config.c
            ${board}/wifi_bt_config.c
    )
    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support/examples/platform/rt/rt1060/board/
        INCLUDES .
            ${board}/
    )
    if(CONFIG_MCUX_PRJSEG_module.board.matter.peripherals)
        mcux_add_source(
            BASE_PATH ${SdkRootDirPath}/middleware/matter/third_party/nxp/nxp_matter_support/examples/platform/rt/rt1060/board/
            SOURCES
                peripherals.c
        )
    endif()
endif()
endif()
else()
# ============================================================
# Fallback: Matter Not Integrated in MCUX SDK
# ============================================================
if(CONFIG_MCUX_PRJSEG_module.board.matter.board)
    # Add board files
    file(GLOB BOARD_FILES
        "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/${board}/*.c"
        "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/${board}/*.h"
        "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/*.c"
        "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/*.h"
    )

    # Remove peripherals files if not LittleFS FileSytem
    if(NOT CONFIG_MCUX_PRJSEG_module.board.matter.peripherals)
        list(REMOVE_ITEM BOARD_FILES 
            "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/peripherals.c"
            "${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/peripherals.h" 
        )
    endif()

    foreach(BOARD_FILE IN LISTS BOARD_FILES)
    #Extract file name and directory path to be usable by mcux cmake function
    get_filename_component(BOARD_DIR_PATH "${BOARD_FILE}" DIRECTORY)
    get_filename_component(BOARD_FILE_NAME "${BOARD_FILE}" NAME)
    mcux_add_source(
        BASE_PATH ${BOARD_DIR_PATH}
        SOURCES
        ${BOARD_FILE_NAME}
    )
    endforeach()

    mcux_add_include(
        BASE_PATH ${NXP_MATTER_SUPPORT_DIR}
        INCLUDES
        examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board/${board}
        examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/board
    )
endif()
endif()