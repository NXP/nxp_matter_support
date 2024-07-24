/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Peripherals v11.0
processor: MIMXRT1062xxxxA
package_id: MIMXRT1062DVL6A
mcu_data: ksdk2_0
processor_version: 11.0.1
board: MIMXRT1060-EVKB
functionalGroups:
- name: BOARD_InitPeripherals
  UUID: 96c1cec6-3bd3-47a2-8301-f38e4b0dd25f
  called_from_default_init: true
  selectedCore: core0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'system'
- type_id: 'system'
- global_system_definitions:
  - user_definitions: 'extern uint32_t NV_STORAGE_START_ADDRESS_OFFSET[];\nextern uint32_t NV_STORAGE_MAX_SECTORS[];\n\n\n#define LITTLEFS_START_ADDR (uint32_t)(NV_STORAGE_START_ADDRESS_OFFSET)\n'
  - user_includes: '\n'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'uart_cmsis_common'
- type_id: 'uart_cmsis_common_9cb8e302497aa696fdbb5a4fd622c2a8'
- global_USART_CMSIS_common:
  - quick_selection: 'default'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'gpio_adapter_common'
- type_id: 'gpio_adapter_common_57579b9ac814fe26bf95df0a384c36b6'
- global_gpio_adapter_common:
  - quick_selection: 'default'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "peripherals.h"
#ifdef WATCHDOG_ALLOWED
#include "fsl_wdog.h"
#endif
/***********************************************************************************************************************
 * BOARD_InitPeripherals functional group
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Watchdog initialization code
 **********************************************************************************************************************/

#ifdef WATCHDOG_ALLOWED
static void WDOG1_init(void)
{
    wdog_config_t config;
    /*
     * wdogConfig->enableWdog = true;
     * wdogConfig->workMode.enableWait = false;
     * wdogConfig->workMode.enableStop = false;
     * wdogConfig->workMode.enableDebug = false;
     * wdogConfig->enableInterrupt = false;
     * wdogConfig->enablePowerdown = false;
     * wdogConfig->resetExtension = false;
     * wdogConfig->timeoutValue = 0xFFU;
     * wdogConfig->interruptTimeValue = 0x04u;
     */
    WDOG_GetDefaultConfig(&config);
    /*
     * Timeout value is (0xF + 1)/2 = 8 sec.
     * Once the WDOG is activated, it must be serviced by the software on a periodic basis, by calling WDOG_Refresh. If
     * servicing does not take place, the timer times out. Upon timeout, the WDOG asserts the internal system reset signal,
     * WDOG_RESET_B_DEB to the System Reset Controller (SRC).
     */
    config.timeoutValue = 0xF;
    /* WDOG peripheral initialization */
    WDOG_Init(WDOG1, &config);
};
#endif

/***********************************************************************************************************************
 * LittleFS initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'LittleFS'
- type: 'littlefs'
- mode: 'general'
- custom_name_enabled: 'false'
- type_id: 'littlefs_7e89bf6c938031bfd17176a3aacf1bc3'
- functional_group: 'BOARD_InitPeripherals'
- config_sets:
  - general_config:
    - moduleInclude: 'fwk_platform_flash.h'
    - lfsConfig:
      - enableUserContext: 'true'
      - userContext:
        - contextVar: '(void*)&LittleFS_ctx'
        - contextDef: 'extern struct lfs_mflash_ctx LittleFS_ctx;'
      - userCallbacks:
        - read: 'lfs_mflash_read'
        - prog: 'lfs_mflash_prog'
        - erase: 'lfs_mflash_erase'
        - sync: 'lfs_mflash_sync'
        - lock: 'lfs_mutex_lock'
        - unlock: 'lfs_mutex_unlock'
      - readSize: '16'
      - progSize: '256'
      - blockSize: '4096'
      - firstBlock: '(uint32_t)(NV_STORAGE_START_ADDRESS_OFFSET)'
      - blockCount: '(uint32_t)(NV_STORAGE_MAX_SECTORS)'
      - blockCycles: '100'
      - cacheSize: '256'
      - lookaheadSize: '16'
      - enableReadBuff: 'false'
      - readBuffer:
        - customBuffer: 'false'
      - enableProgBuff: 'false'
      - progBuffer:
        - customBuffer: 'false'
      - enableLookaheadBuff: 'false'
      - lookaheadBuffer:
        - customBuffer: 'false'
      - enableOptionalSizes: 'false'
      - optionalSizes:
        - name_max: '255'
        - file_max: '0x7FFFFFFF'
        - attr_max: '1022'
        - metadata_max: '4096'
    - initLFS: 'false'
    - initConfig:
      - lfsObj: 'LittleFS_system'
      - mountLFS: 'disable'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
/* LittleFS context */
extern struct lfs_mflash_ctx LittleFS_ctx;
const struct lfs_config LittleFS_config = { .context = (void *) &LittleFS_ctx,
                                            .read    = lfs_mflash_read,
                                            .prog    = lfs_mflash_prog,
                                            .erase   = lfs_mflash_erase,
                                            .sync    = lfs_mflash_sync,
#ifdef LFS_THREADSAFE
                                            .lock   = lfs_mutex_lock,
                                            .unlock = lfs_mutex_unlock,
#endif
                                            .read_size      = LITTLEFS_READ_SIZE,
                                            .prog_size      = LITTLEFS_PROG_SIZE,
                                            .block_size     = LITTLEFS_BLOCK_SIZE,
                                            .block_count    = LITTLEFS_BLOCK_COUNT,
                                            .block_cycles   = LITTLEFS_BLOCK_CYCLES,
                                            .cache_size     = LITTLEFS_CACHE_SIZE,
                                            .lookahead_size = LITTLEFS_LOOKAHEAD_SIZE };

/* Empty initialization function (commented out)
static void LittleFS_init(void) {
} */

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
    /* Initialize components */
#ifdef WATCHDOG_ALLOWED
    WDOG1_init();
#endif
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
    BOARD_InitPeripherals();
}
