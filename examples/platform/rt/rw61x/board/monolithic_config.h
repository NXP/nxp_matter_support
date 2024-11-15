/*
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

/* Configuration file for monolithic property */

/* 
 * Including this file will enable the CPU1/CPU2 firmwares to be embedded to the application,
 * if "gPlatformMonolithicApp_d" is enabled.
 */

#if (defined(gPlatformMonolithicApp_d) && (gPlatformMonolithicApp_d > 0))

#ifndef RW610
#define RW610
#endif

#define CONFIG_SOC_SERIES_RW6XX_REVISION_A2     1

/* Check if we are in the case of BLE only or BLE+15.4 combo */
#if !CONFIG_NET_L2_OPENTHREAD
#define CONFIG_MONOLITHIC_BLE 1
#define COMBO_FW_ADDRESS 0
#else
#define CONFIG_MONOLITHIC_BLE_15_4 1
#define BLE_FW_ADDRESS 0
#endif

#define CONFIG_MONOLITHIC_WIFI 1

#endif /* gPlatformMonolithicApp_d */
