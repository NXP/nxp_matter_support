/*
 * Copyright 2021, 2023,2024, 2025 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "fsl_common.h"
#include "fsl_io_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void)
{ /*!< Function assigned for the core: Cortex-M33[cm33] */
    IO_MUX_SetPinMux(IO_MUX_FC3_USART_DATA);
    IO_MUX_SetPinMux(IO_MUX_GPIO25);
    /* FLEXCOMM0 TX for logging*/
    IO_MUX_SetPinMux(IO_MUX_FC0_USART_DATA);
#if CONFIG_CHIP_ETHERNET
    IO_MUX_SetPinMux(IO_MUX_ENET_CLK);
    IO_MUX_SetPinMux(IO_MUX_ENET_RX);
    IO_MUX_SetPinMux(IO_MUX_ENET_TX);
    IO_MUX_SetPinMux(IO_MUX_ENET_MDIO);
    IO_MUX_SetPinMux(IO_MUX_GPIO21);
    IO_MUX_SetPinMux(IO_MUX_GPIO55);
#endif
}

void BOARD_InitPinButton0(void)
{
    IO_MUX_SetPinMux(IO_MUX_GPIO11);
}

void BOARD_InitPinLEDRGB(void)
{
    /* BLUE LED */
    IO_MUX_SetPinMux(IO_MUX_GPIO0);
    /* RED LED */
    IO_MUX_SetPinMux(IO_MUX_GPIO1);
    /* GREEN LED */
    IO_MUX_SetPinMux(IO_MUX_GPIO12);
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
