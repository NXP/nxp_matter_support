/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_config.h"
#include "fsl_clock.h"
#include "fsl_power.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 ************************ BOARD_InitBootClocks function ************************
 ******************************************************************************/
void BOARD_InitBootClocks(void)
{
    BOARD_BootClockRUN();
}

/*******************************************************************************
 ********************** Configuration BOARD_BootClockRUN ***********************
 ******************************************************************************/

/*******************************************************************************
 * Variables for BOARD_BootClockRUN configuration
 ******************************************************************************/
const clock_avpll_config_t avpllConfig_BOARD_BootClockRUN = {
    .ch1Freq    = kCLOCK_AvPllChFreq12p288m, /* AVPLL channel frequency 12.288 MHz */
    .ch2Freq    = kCLOCK_AvPllChFreq64m,     /* AVPLL channel frequency 64 MHz */
    .enableCali = true,                      /* AVPLL calibration is enabled */
};
/*******************************************************************************
 * Code for BOARD_BootClockRUN configuration
 ******************************************************************************/
void BOARD_BootClockRUN(void)
{
    /* Disable GDET and VSensors */
    POWER_DisableGDetVSensors();
    /* Enable CAU sleep clock for PMU */
    if ((PMU->CAU_SLP_CTRL & PMU_CAU_SLP_CTRL_SOC_SLP_RDY_MASK) == 0U)
    {
        /* Enable the CAU sleep clock. */
        CLOCK_EnableClock(kCLOCK_RefClkCauSlp);
    }
    if ((SYSCTL2->SOURCE_CLK_GATE & SYSCTL2_SOURCE_CLK_GATE_REFCLK_SYS_CG_MASK) != 0U)
    {
        /* Enable the REFCLK_SYS clock. */
        CLOCK_EnableClock(kCLOCK_RefClkSys);
    }
    /* Initialize T3 PLL and enable outputs that are not clock gated. */
    CLOCK_InitT3RefClk(kCLOCK_T3MciIrc48m);
    /* Enable FFRO - T3 PLL 48/60 MHz IRC clock output */
    CLOCK_EnableClock(kCLOCK_T3PllMciIrcClk);
    /* Enable T3 PLL 256 MHz clock output */
    CLOCK_EnableClock(kCLOCK_T3PllMci256mClk);

    BOARD_ClockPreConfig();
    /* Set core clock to safe system oscillator clock for initialization of other sources. */
    CLOCK_AttachClk(kSYSOSC_to_MAIN_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivSysCpuAhbClk, 1);
    /* Initialize TCPU PLL and enable outputs that are not clock gated. */
    CLOCK_InitTcpuRefClk(3120000000UL, kCLOCK_TcpuFlexspiDiv10);
    /* Enable TCPU PLL MCI clock output */
    CLOCK_EnableClock(kCLOCK_TcpuMciClk);
    /* Initialize TDDR PLL and enable outputs that are not clock gated. */
    CLOCK_InitTddrRefClk(kCLOCK_TddrFlexspiDiv10);
    /* Enable TDDR PLL FlexSPI clock output */
    CLOCK_EnableClock(kCLOCK_TddrMciFlexspiClk);
    /* Initialize AVPLL and enable both channels. */
    CLOCK_InitAvPll(&avpllConfig_BOARD_BootClockRUN);
    /* Set up clock selectors - Attach clocks to the peripheries */
    CLOCK_AttachClk(kRC32K_to_CLK32K); /* Switch CLK32K to RC32K */
    /*!< Please note SYSTICK_CLK source is used only if the SysTick SYST_CSR register CLKSOURCE bit is set to 0. */
    CLOCK_AttachClk(kSYSTICK_DIV_to_SYSTICK_CLK); /* Switch SYSTICK_CLK to SYSTICK_DIV */
    /* Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivAudioPllClk, 1U); /* Set .AUDIOPLLCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivPllFrgClk, 13U);  /* Set .FRGPLLCLKDIV divider to value 13 */
    CLOCK_SetClkDiv(kCLOCK_DivMainPllClk, 1U);  /* Set .MAINPLLCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivAux0PllClk, 1U);  /* Set .AUX0PLLCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk, 1U);  /* Set .SYSTICKFCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivPmuFclk, 5U);     /* Set .PMUFCLKDIV divider to value 5 */
    /* Select the main clock source for the main system clock (MAINCLKSELA and MAINCLKSELB). */
    CLOCK_AttachClk(kMAIN_PLL_to_MAIN_CLK);
    BOARD_ClockPostConfig();
    /*!< Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_HCLK;
}

/*******************************************************************************
 ********************** Configuration BOARD_BootClockLPR ***********************
 ******************************************************************************/

/*******************************************************************************
 * Variables for BOARD_BootClockLPR configuration
 ******************************************************************************/
/*******************************************************************************
 * Code for BOARD_BootClockLPR configuration
 ******************************************************************************/
void BOARD_BootClockLPR(void)
{
    /* Disable GDET and VSensors */
    POWER_DisableGDetVSensors();
    /* Enable CAU sleep clock for PMU */
    if ((PMU->CAU_SLP_CTRL & PMU_CAU_SLP_CTRL_SOC_SLP_RDY_MASK) == 0U)
    {
        /* Enable the CAU sleep clock. */
        CLOCK_EnableClock(kCLOCK_RefClkCauSlp);
    }
    if ((SYSCTL2->SOURCE_CLK_GATE & SYSCTL2_SOURCE_CLK_GATE_REFCLK_SYS_CG_MASK) != 0U)
    {
        /* Enable the REFCLK_SYS clock. */
        CLOCK_EnableClock(kCLOCK_RefClkSys);
    }
    /* Initialize T3 PLL and enable outputs that are not clock gated. */
    CLOCK_InitT3RefClk(kCLOCK_T3MciIrc48m);
    /* Enable T3 PLL 256 MHz clock output */
    CLOCK_EnableClock(kCLOCK_T3PllMci256mClk);
    BOARD_ClockPreConfig();
    /* Set core clock to safe system oscillator clock for initialization of other sources. */
    CLOCK_AttachClk(kSYSOSC_to_MAIN_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivSysCpuAhbClk, 1);
    /* Initialize TCPU PLL and enable outputs that are not clock gated. */
    CLOCK_InitTcpuRefClk(3120000000UL, kCLOCK_TcpuFlexspiDiv10);
    /* Enable TCPU PLL MCI clock output */
    CLOCK_EnableClock(kCLOCK_TcpuMciClk);
    /* Set up clock selectors - Attach clocks to the peripheries */
    CLOCK_AttachClk(kRC32K_to_CLK32K); /* Switch CLK32K to RC32K */
    /*!< Please note SYSTICK_CLK source is used only if the SysTick SYST_CSR register CLKSOURCE bit is set to 0. */
    CLOCK_AttachClk(kSYSTICK_DIV_to_SYSTICK_CLK); /* Switch SYSTICK_CLK to SYSTICK_DIV */
    /* Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivPllFrgClk, 13U); /* Set .FRGPLLCLKDIV divider to value 13 */
    CLOCK_SetClkDiv(kCLOCK_DivMainPllClk, 1U); /* Set .MAINPLLCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivAux0PllClk, 1U); /* Set .AUX0PLLCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk, 1U); /* Set .SYSTICKFCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivPmuFclk, 5U);    /* Set .PMUFCLKDIV divider to value 5 */
    /* Select the main clock source for the main system clock (MAINCLKSELA and MAINCLKSELB). */
    CLOCK_AttachClk(kMAIN_PLL_to_MAIN_CLK);
    /* Deinitialization of the AVPLL. */
    CLOCK_DeinitAvPll();
    /* Deinitialize TDDR PLL. */
    CLOCK_DeinitTddrRefClk();
    BOARD_ClockPostConfig();
    /*!< Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKLPR_HCLK;
}
