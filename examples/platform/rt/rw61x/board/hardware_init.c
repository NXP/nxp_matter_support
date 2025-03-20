/*
 * Copyright 2020, 2023, 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#if CONFIG_CHIP_ETHERNET
#include "app_ethernet.h"
#endif
#include "board.h"
#include "clock_config.h"
#include "fsl_device_registers.h"
#include "pin_mux.h"

#if CONFIG_CHIP_ETHERNET
#include "fsl_enet.h"
#include "fsl_phy.h"
#include "fsl_reset.h"
#endif // CONFIG_CHIP_ETHERNET
/*${header:end}*/

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */
#if CONFIG_CHIP_ETHERNET
phy_ksz8081_resource_t g_phy_resource;
#endif // CONFIG_CHIP_ETHERNET

static gpio_pin_config_t pinConfig = {
    kGPIO_DigitalOutput,
    0,
};

#if CONFIG_CHIP_ETHERNET
/*${function:start}*/
void BOARD_InitModuleClock(void)
{
    /* Set 50MHz output clock required by PHY. */
    CLOCK_EnableClock(kCLOCK_TddrMciEnetClk);
}

static void MDIO_Init(void)
{
    uint32_t i = ENET_GetInstance(EXAMPLE_ENET);

    (void) CLOCK_EnableClock(s_enetClock[i]);
    (void) CLOCK_EnableClock(s_enetExtraClock[i]);
    ENET_SetSMI(EXAMPLE_ENET, EXAMPLE_CLOCK_FREQ, false);
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    const status_t s = ENET_MDIOWrite(EXAMPLE_ENET, phyAddr, regAddr, data);
    return s;
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t * pData)
{
    const status_t s = ENET_MDIORead(EXAMPLE_ENET, phyAddr, regAddr, pData);
    return s;
}

#endif // CONFIG_CHIP_ETHERNET

/*${function:start}*/
void BOARD_InitHardware(void)
{
#if CONFIG_CHIP_ETHERNET
    gpio_pin_config_t gpio_config = { kGPIO_DigitalOutput, 1U };
#endif

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
#if CONFIG_CHIP_ETHERNET
    BOARD_InitModuleClock();
#endif

    CLOCK_EnableClock(kCLOCK_Flexspi);
    RESET_ClearPeripheralReset(kFLEXSPI_RST_SHIFT_RSTn);

    /* Use aux0_pll_clk / 2 */
    BOARD_SetFlexspiClock(FLEXSPI, 2U, 2U);

#if CONFIG_CHIP_ETHERNET
    ENET_ResetHardware();

    GPIO_PortInit(GPIO, 0U);
    GPIO_PortInit(GPIO, 1U);
    GPIO_PinInit(GPIO, 0U, 21U, &gpio_config); /* ENET_RST */
    gpio_config.pinDirection = kGPIO_DigitalInput;
    gpio_config.outputLogic  = 0U;
    GPIO_PinInit(GPIO, 1U, 23U, &gpio_config); /* ENET_INT */

    GPIO_PinWrite(GPIO, 0U, 21U, 0U);
    SDK_DelayAtLeastUs(1000000, CLOCK_GetCoreSysClkFreq());
    GPIO_PinWrite(GPIO, 0U, 21U, 1U);

    MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;
#endif
}
/*${function:end}*/
