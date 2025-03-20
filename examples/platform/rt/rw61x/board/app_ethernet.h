/*
 * Copyright 2021-2022, 2025 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_ETHERNET_
#define _APP_ETHERNET_

#include "fsl_phyksz8081.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
/* Ethernet configuration. */
extern phy_ksz8081_resource_t g_phy_resource;
#define EXAMPLE_ENET ENET
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS 0x02U

/* PHY operations. */
#define EXAMPLE_PHY_OPS &phyksz8081_ops
#define EXAMPLE_PHY_RESOURCE &g_phy_resource

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetMainClkFreq()
#endif