#
#   Copyright (c) 2024 Project CHIP Authors
#   Copyright 2024 NXP
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

#
#   @file
#     CMake file that defines cmake functions common to NXP platforms
#

#*********************************************************************************
# Set common variables
#*********************************************************************************

if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../.. REALPATH)
endif()

# ARMGCC_DIR shall be exported in order to build SDK examples with CMake (required for mcuboot build)
if(DEFINED ENV{ARMGCC_DIR})
    set(ARMGCC_DIR $ENV{ARMGCC_DIR})
else()
    if(DEFINED ENV{PW_ENVIRONMENT_ROOT})
        set(ARMGCC_DIR $ENV{PW_ENVIRONMENT_ROOT}/cipd/packages/arm)
    else()
        set(ARMGCC_DIR ${CHIP_ROOT}/.environment/cipd/packages/arm)
    endif()
endif()

message(STATUS "ARMGCC_DIR is set to : ${ARMGCC_DIR}")

find_package(Python3 REQUIRED)

#*********************************************************************************
# Retrieve NXP SDK path
#*********************************************************************************
# This function will retrieve where the platform SDK is located and export it 
# in a dedicated variable
function(nxp_get_sdk_path platform sdk_path_variable)
    if(DEFINED ENV{${sdk_path_variable}})
        set(${sdk_path_variable} "$ENV{${sdk_path_variable}}" CACHE INTERNAL "NXP SDK root path")   
    else()
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "from nxp_matter_support.scripts.update_nxp_sdk import *; [print(p.sdk_storage_location_abspath) for p in ALL_PLATFORM_SDK if p.sdk_name == '${platform}'][0];"
            OUTPUT_VARIABLE OUTPUT_VAR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${CHIP_ROOT}/third_party/nxp
        )
        set(${sdk_path_variable} "${OUTPUT_VAR}" CACHE INTERNAL "NXP SDK root path")
    endif()
endfunction()

# Set the NXP SDK path
if(CONFIG_CHIP_BUILD_APP_WITH_GN)
if(CONFIG_CHIP_NXP_PLATFORM STREQUAL "k32w0")
    nxp_get_sdk_path("k32w0" NXP_K32W0_SDK_ROOT)
    message(STATUS "NXP_K32W0_SDK_ROOT is set to : ${NXP_K32W0_SDK_ROOT}")
else()
    nxp_get_sdk_path("common" NXP_SDK_ROOT)
    message(STATUS "NXP_SDK_ROOT is set to : ${NXP_SDK_ROOT}")
endif()
else() # SDK 3.0 build system
    set(NXP_SDK_ROOT ${SdkRootDirPath})
endif()

# Define output variable based on the build system used
if(CONFIG_CHIP_BUILD_APP_WITH_GN)
    if (NOT DEFINED gn_build)
        set(gn_build ${CMAKE_BINARY_DIR}/out/debug CACHE INTERNAL "Path to the GN build output")
    endif()
    set(APP_OUTPUT_DIR ${gn_build})
    set(APP_EXECUTABLE_NAME ${PROJECT_NAME})
else() # SDK 3.0 build system
    set(APP_OUTPUT_DIR ${CMAKE_BINARY_DIR})
    set(APP_EXECUTABLE_NAME app)
    set(APP_EXECUTABLE_SUFFIX .elf)
endif()


#*********************************************************************************
# Build example application with GN and Ninja
#*********************************************************************************
function(nxp_build_app_with_gn)
    
    add_custom_command(
        OUTPUT          ${gn_build}/build.ninja
        COMMAND         export NXP_SDK_ROOT=${NXP_SDK_ROOT}
        COMMAND         ${Python3_EXECUTABLE} ${CHIP_ROOT}/config/common/cmake/make_gn_args.py 
                        @${CMAKE_CURRENT_BINARY_DIR}/chip/args.tmp > ${gn_build}/args.gn
        COMMAND         gn gen ${gn_build}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

    add_custom_target(build_app_with_gn ALL
        COMMAND ninja -C ${gn_build}
        DEPENDS ${gn_build}/build.ninja
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Building ${PROJECT_NAME} with GN and Ninja"
    )

    # Make sure the pre-build process is executed before building the application
    add_dependencies(build_app_with_gn pre_build)
        
endfunction(nxp_build_app_with_gn)

#*********************************************************************************
# Generate MCUBoot application
#*********************************************************************************
if(DEFINED CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING)
    string(REGEX REPLACE "\\\\" "" CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING ${CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING})
endif()

get_filename_component(MCUBOOT_OPENSOURCE_DIR "${NXP_SDK_ROOT}/middleware/mcuboot_opensource" REALPATH)
if(CONFIG_CHIP_BUILD_APP_WITH_GN)
    get_filename_component(MCUBOOT_EXAMPLE_DIR "${NXP_SDK_ROOT}/examples/${CONFIG_BOARD_NAME}/ota_examples/mcuboot_opensource/${CONFIG_CORE_FOLDER}" REALPATH)
else()
    get_filename_component(MCUBOOT_EXAMPLE_DIR "${NXP_SDK_ROOT}/examples/ota_examples/mcuboot_opensource" REALPATH)
endif()
function(nxp_generate_mcuboot)
if(CONFIG_CHIP_BUILD_APP_WITH_GN)
    add_custom_target(build_mcuboot ALL
        COMMAND chmod +x build_${CONFIG_MCUBOOT_BUILD_TYPE}.sh
        COMMAND export ARMGCC_DIR=${ARMGCC_DIR}
        COMMAND ./build_${CONFIG_MCUBOOT_BUILD_TYPE}.sh
        WORKING_DIRECTORY ${MCUBOOT_EXAMPLE_DIR}/armgcc
        COMMENT "Generating MCUBoot binary"
    )
    add_dependencies(build_mcuboot build_app_with_gn)
    add_custom_command(
        TARGET build_mcuboot POST_BUILD
        COMMAND arm-none-eabi-objcopy -O binary ${MCUBOOT_EXAMPLE_DIR}/armgcc/${CONFIG_MCUBOOT_BUILD_TYPE}/mcuboot_opensource.elf ${gn_build}/mcuboot_opensource.bin
        COMMENT "Copying mcuboot binary to example outputs folder"
    )
else() # SDK Next Gen build system
    if(DEFINED core_id)
        add_custom_target(build_mcuboot ALL
            COMMAND export ARMGCC_DIR=${ARMGCC_DIR}
            COMMAND west build -d ${CMAKE_CURRENT_BINARY_DIR}/mcuboot -b ${board} ${MCUBOOT_EXAMPLE_DIR} -Dcore_id=${core_id}  --config flexspi_nor_release
            WORKING_DIRECTORY ${SdkRootDirPath}
            COMMENT "Generating MCUBoot binary"
        )
    else()
        add_custom_target(build_mcuboot ALL
            COMMAND export ARMGCC_DIR=${ARMGCC_DIR}
            COMMAND west build -d ${CMAKE_CURRENT_BINARY_DIR}/mcuboot -b ${board} ${MCUBOOT_EXAMPLE_DIR}
            WORKING_DIRECTORY ${SdkRootDirPath}
            COMMENT "Generating MCUBoot binary"
        )
    endif()
    add_dependencies(build_mcuboot app)
endif()

endfunction(nxp_generate_mcuboot)


#*********************************************************************************
# Sign application binary with MCUBoot imgtool
#*********************************************************************************
function(nxp_sign_app_imgtool bin_sections_to_remove)
    
    string(REPLACE " " ";" bin_sections_to_remove ${bin_sections_to_remove})
    add_custom_command(
        OUTPUT ${APP_OUTPUT_DIR}/${APP_EXECUTABLE_NAME}.bin
        COMMAND arm-none-eabi-objcopy ${bin_sections_to_remove} -O binary ${APP_OUTPUT_DIR}/${APP_EXECUTABLE_NAME}${APP_EXECUTABLE_SUFFIX} ${APP_OUTPUT_DIR}/${APP_EXECUTABLE_NAME}.bin
        COMMENT "Converting the application .elf into binary"
    )

    # The mcuboot args "--pad --confirm" should only be used for images 
    # which will be programmed directly into flash.
    # For OTA update images, the "--pad --confirm" should not be applied.
    # Here, we are assuming that images with SW version > 1 are update images.
    if(CONFIG_CHIP_DEVICE_SOFTWARE_VERSION LESS_EQUAL 1)
        set(CONFIG_MCUBOOT_ADDITIONAL_ARGS --pad --confirm)
    else()
        set(CONFIG_MCUBOOT_ADDITIONAL_ARGS "")
    endif()

    add_custom_target(sign_application ALL
        COMMAND ${Python3_EXECUTABLE} imgtool.py sign --key ${MCUBOOT_OPENSOURCE_DIR}/boot/nxp_mcux_sdk/keys/sign-rsa2048-priv.pem --align 4 --header-size ${CONFIG_CHIP_MCUBOOT_HEADER_SIZE} --pad-header
               --slot-size ${CONFIG_CHIP_MCUBOOT_SLOT_SIZE} --max-sectors ${CONFIG_CHIP_MCUBOOT_MAX_SECTORS} --version ${CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING} ${CONFIG_MCUBOOT_ADDITIONAL_ARGS} ${APP_OUTPUT_DIR}/${APP_EXECUTABLE_NAME}.bin ${APP_OUTPUT_DIR}/${APP_EXECUTABLE_NAME}_SIGNED.bin
        WORKING_DIRECTORY ${MCUBOOT_OPENSOURCE_DIR}/scripts
        DEPENDS ${APP_OUTPUT_DIR}/${APP_EXECUTABLE_NAME}.bin
        COMMENT "Sign the application binary with imgtool.py"
    )

if(CONFIG_CHIP_BUILD_APP_WITH_GN)
    add_dependencies(sign_application build_app_with_gn)
else()
    add_dependencies(sign_application app)
endif()
    
endfunction(nxp_sign_app_imgtool)


#*********************************************************************************
# Sign application binary with MCUBoot imgtool
#*********************************************************************************
get_filename_component(CHIP_OTA_IMGTOOL_DIR "${CHIP_ROOT}/src/app" REALPATH)

function(nxp_generate_ota_file)

    add_custom_target(chip-ota-image ALL
        COMMAND ./ota_image_tool.py create -v ${CONFIG_CHIP_DEVICE_VENDOR_ID} -p ${CONFIG_CHIP_DEVICE_PRODUCT_ID} -vn ${CONFIG_CHIP_DEVICE_SOFTWARE_VERSION} -vs ${CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING} -da sha256 ${APP_OUTPUT_DIR}/${APP_EXECUTABLE_NAME}_SIGNED.bin ${APP_OUTPUT_DIR}/${APP_EXECUTABLE_NAME}.ota
        WORKING_DIRECTORY ${CHIP_OTA_IMGTOOL_DIR}
        COMMENT "Generating ota file"
    )

    add_dependencies(chip-ota-image sign_application)

endfunction(nxp_generate_ota_file)

#*********************************************************************************
# Pre-build process
#*********************************************************************************
function(nxp_pre_build_process)
    # Note : application build target should add dependency on the pre_build target
    # to ensure the pre-build process is executed before the application build
    
    add_custom_target(pre_build ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Running pre-build process"
    )

    if (CONFIG_BUILD_K32W0_OT_RCP_IMAGE)

        # TODO : use k32w0 SDK cloned from nxp_matter_support repo instead,
        # once it is supported by the OT RCP build of k32w0.
        # nxp_get_sdk_path("k32w0" NXP_K32W0_SDK_ROOT)

        # Set up the k32w061 SDK inside ot-nxp repo
        add_custom_command(
            OUTPUT ${CHIP_ROOT}/third_party/openthread/ot-nxp/third_party/k32w061_sdk/.west
            COMMAND rm -rf ../.west && west init -l manifest --mf west.yml && west update
            WORKING_DIRECTORY ${CHIP_ROOT}/third_party/openthread/ot-nxp/third_party/k32w061_sdk/repo
        )

        # Build the k32w061 OT RCP image
        add_custom_target(build_k32w0_rcp ALL
            COMMAND ./script/build_k32w061 ot_rcp_ble_hci_bb_single_uart_fc
            DEPENDS ${CHIP_ROOT}/third_party/openthread/ot-nxp/third_party/k32w061_sdk/.west
            WORKING_DIRECTORY ${CHIP_ROOT}/third_party/openthread/ot-nxp
        )

        # Make sure this is run as pre-build
        add_dependencies(pre_build build_k32w0_rcp)
    
    endif()

endfunction(nxp_pre_build_process)
