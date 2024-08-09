#
#   Copyright (c) 2024 Project CHIP Authors
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
#     CMake file that maps cmake flags to SDK gn args
#

if(CONFIG_CHIP_RT_PLATFORM)

matter_add_gn_arg_bool  ("iwx12_transceiver"                      CONFIG_TRANSCEIVER_IWX12)
matter_add_gn_arg_bool  ("iw416_transceiver"                      CONFIG_TRANSCEIVER_IW416)
matter_add_gn_arg_bool  ("w8801_transceiver"                      CONFIG_TRANSCEIVER_W8801)
matter_add_gn_arg_bool  ("k32w0_transceiver"                      CONFIG_TRANSCEIVER_K32W0)
matter_add_gn_arg_bool  ("hci_spinel_single_uart"                 CONFIG_HCI_SPINEL_SINGLE_UART)

if(CONFIG_BOARD_EVK_NAME)
    matter_add_gn_arg_string("evkname" ${CONFIG_BOARD_EVK_NAME})
endif()

if(CONFIG_BOARD_VARIANT)
    matter_add_gn_arg_string("board_version" ${CONFIG_BOARD_VARIANT})
endif()

if(CONFIG_K32W0_TRANSCEIVER_BIN_PATH)
    matter_add_gn_arg_string("k32w0_transceiver_bin_path" ${CONFIG_K32W0_TRANSCEIVER_BIN_PATH})
endif()

endif() #CONFIG_CHIP_RT_PLATFORM
