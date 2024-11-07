/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 *    @file
 *          Provides FreeRTOS low power hooks
 */

#include "FreeRTOS.h"

#include "PWR_Interface.h"
#include "fsl_os_abstraction.h"

#if (configUSE_TICKLESS_IDLE != 0)

/*! *********************************************************************************
 *\private
 *\fn           void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
 *\brief        This function will try to put the MCU into a deep sleep mode for at
 *              most the maximum OS idle time specified. Else the MCU will enter a
 *              sleep mode until the first IRQ.
 *
 *\param  [in]  xExpectedIdleTime    The idle time in OS ticks.
 *
 *\retval       none.
 *
 *\remarks      This feature is available only for FreeRTOS.
 ********************************************************************************** */
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    bool abortIdle = false;
    uint64_t actualIdleTimeUs, expectedIdleTimeUs;

    /* The OSA_InterruptDisable() API will prevent us to wakeup so we use
     * OSA_DisableIRQGlobal() */
    OSA_DisableIRQGlobal();

    /* Disable and prepare systicks for low power */
    abortIdle = PWR_SysticksPreProcess((uint32_t) xExpectedIdleTime, &expectedIdleTimeUs);

    if (abortIdle == false)
    {
        /* Enter low power with a maximal timeout */
        actualIdleTimeUs = PWR_EnterLowPower(expectedIdleTimeUs);

        /* Re enable systicks and compensate systick timebase */
        PWR_SysticksPostProcess(expectedIdleTimeUs, actualIdleTimeUs);
    }

    /* Exit from critical section */
    OSA_EnableIRQGlobal();
}
#endif
