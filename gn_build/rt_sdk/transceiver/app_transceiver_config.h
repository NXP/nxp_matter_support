/*
 *  Copyright 2020-2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#define CONTROLLER_ID kUSB_ControllerEhci0

#if defined(WIFI_IW416_BOARD_AW_AM457_USD) || defined(WIFI_IW416_BOARD_AW_AM510_USD) ||                                            \
    defined(WIFI_88W8987_BOARD_AW_CM358_USD) || defined(WIFI_IW612_BOARD_RD_USD) || defined(WIFI_BOARD_RW610) ||                   \
    defined(WIFI_88W8801_BOARD_MURATA_2DS_USD) || defined(WIFI_IW612_BOARD_MURATA_2EL_USD) ||                                      \
    defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
#define WIFI_TRANSCEIVER_SUPPORT 1
#else
#define WIFI_TRANSCEIVER_SUPPORT 0
#endif

/*
 * Check if the transceiver is supported.
 * Applies only for Matter over Wi-fi or Matter over Thread
 */
#if (CHIP_DEVICE_CONFIG_ENABLE_WPA || CHIP_DEVICE_CONFIG_ENABLE_THREAD)
#if !WIFI_TRANSCEIVER_SUPPORT && !defined(K32W061_TRANSCEIVER)
#error The transceiver module is unsupported
#endif
#endif /* CHIP_DEVICE_CONFIG_ENABLE_WPA || CHIP_DEVICE_CONFIG_ENABLE_THREAD */

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CONFIG_BT_DEVICE_NAME "NXP BLE test"
#include "edgefast_bluetooth_config.h"
#endif /* CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE */
