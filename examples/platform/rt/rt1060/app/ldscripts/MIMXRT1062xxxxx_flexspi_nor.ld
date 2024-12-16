/*
** ###################################################################
**     Processors:          MIMXRT1062CVJ5A
**                          MIMXRT1062CVL5A
**                          MIMXRT1062DVJ6A
**                          MIMXRT1062DVL6A
**
**     Compiler:            GNU C Compiler
**     Reference manual:    IMXRT1060RM Rev.1, 12/2018 | IMXRT1060SRM Rev.3
**     Version:             rev. 0.1, 2017-01-10
**     Build:               b210709
**
**     Abstract:
**         Linker file for the GNU C Compiler
**
**     Copyright 2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2024 NXP
**     All rights reserved.
**
**     SPDX-License-Identifier: BSD-3-Clause
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
** ###################################################################
*/

/**
 * @file
 *   Linker instructions for RT1060
 */


/** If OTA is enabled : MCUBoot bootloader resides at the base of the flash and occupies 0x11000 (68kBytes) 
  slot 0 : main application partition (active app)
           slot0 (3MB) = header (4kB) + ivt (1kB) + m_text + trailer (4kB) + padding (4kB)
  slot 1 : OTA Update storage partition (candidate app)
           slot1 (3MB) = OTA_image (code is already wrapped with header & trailer)
**/

/************************ Flash Layout with OTA enabled *********************

             0x6000_0000    FLASH_START
    - - - +---------------+ - - - - - - - - - - - - - - - - - - - -
          |               |                 
          |    MCUBoot    |                 
   256k   |   bootloader  |                 
          |               |
    - - - +---------------+ - - - - - - - - - - - - - - - - - - - -
   4k     |  Header_slot0 |                                 '
          +---------------+ m_interrupts_start              '
          |  m_interrupts |                                 '
          +---------------+ m_text_start                    '
          |               |                                 '      
 m_app_   |               |                                 '
_stated_  |     m_text    |                                 '   ACTIVE_APP (slot 0)
 _size    |               |                                 '    (4 MB  for evkc)
          |               |                                 '    (3 MB  for evkb)
          |               |                                 '
          +---------------+ m_text_end                      '
   4k     | Trailer_slot0 |                                 '
          +---------------+                                 '
   4k     |    padding    |                                 '
    - - - +---------------+ - - - - - - - - - - - - - - - - - - - -
          |               | FW_UPDATE_STORAGE_START         '
          |               |                                 '
          |               |                                 '
          |               |                                 '
          |               |                                 '
          |      OTA      |                                 '
          |     Update    |                                 '   CANDIDATE_APP (slot 1)
          |     Image     |                                 '      (4 MB  for evkc)
          |               |                                 '      (3 MB  for evkb)
          |               |                                 '
          |               |                                 '
          |               |                                 '
          |               | FW_UPDATE_STORAGE_END           '
    - - - +---------------+ - - - - - - - - - - - - - - - - - - - -
          +               +
          +               + 
    - - - +---------------+ - - - - - - - - - - - - - - - - - - - -
    4k    |  FactoryData  |
    - - - +---------------+ - - - - - - - - - - - - - - - - - - - - 
          |               | NV_STORAGE_START_ADDRESS
          |   NVM_region  | 
          |               | NV_STORAGE_END_ADDRESS
    - - - +---------------+ - - - - - - - - - - - - - - - - - - - -
            FLASH_END        0x61000000  for evkc , 0x60800000 for evkb 

*****************************************************************************/


/* Entry Point */
ENTRY(Reset_Handler)

FLASH_START                  = 0x60000000;
FLASH_SIZE                   = DEFINED(gEVKCFlashSize_d) ? gEVKCFlashSize_d : 0x800000; /* 16 MB for evkc, otherwise 8 MB */
FLASH_END                    = FLASH_START + FLASH_SIZE - 1;
FLASH_SECTOR_SIZE            = 0x1000;  /* 4k flash sector size */

/*** FileSystem Configuration ***/
/* OT/Matter/BLE  filesystem configuration*/
NV_STORAGE_END_ADDRESS       = FLASH_END;
NV_STORAGE_MAX_SECTORS       = DEFINED(gNVMSectorCountLink_d) ? gNVMSectorCountLink_d : 32;
NV_STORAGE_SECTOR_SIZE       = FLASH_SECTOR_SIZE;
NV_STORAGE_SIZE              = FLASH_SECTOR_SIZE * NV_STORAGE_MAX_SECTORS;
NV_STORAGE_START_ADDRESS     = NV_STORAGE_END_ADDRESS - NV_STORAGE_SIZE + 1;
NV_STORAGE_START_ADDRESS_OFFSET = NV_STORAGE_START_ADDRESS - FLASH_START;

/*** FactoryData space 1 sector is reserved ***/
__FACTORY_DATA_SIZE   = FLASH_SECTOR_SIZE;
__FACTORY_DATA_END    = NV_STORAGE_START_ADDRESS -1;
__FACTORY_DATA_START  = __FACTORY_DATA_END - __FACTORY_DATA_SIZE + 1;
__FACTORY_DATA_START_OFFSET = __FACTORY_DATA_START - FLASH_START;

m_flash_start                       = FLASH_START;
m_flash_sector_size                 = FLASH_SECTOR_SIZE;

m_boot_flash_conf_sector_start      = m_flash_start;
m_boot_flash_conf_offset            = 0x400;
m_boot_flash_conf_start             = m_boot_flash_conf_sector_start + m_boot_flash_conf_offset;
m_boot_flash_conf_sector_end        = m_boot_flash_conf_sector_start + m_flash_sector_size - 1;
m_boot_flash_conf_size              = m_flash_sector_size - m_boot_flash_conf_offset;


m_ivt_start                         = m_boot_flash_conf_sector_end + 1;
m_ivt_size                          = m_flash_sector_size;
m_ivt_end                           = m_ivt_start + m_ivt_size - 1;

/* 
 * If OTA is enabled, we must reserve enough space for MCUboot and the image header.
 * Note that MCUboot itself embeds the FlashConfig section.
*/
m_mcuboot_size                      = DEFINED(__m_mcuboot_size__)  ? __m_mcuboot_size__  : 0; 
m_mcuboot_start                     = m_flash_start; /* The flash config section is comprised */
m_mcuboot_end                       = m_mcuboot_start + m_mcuboot_size - 1;
m_mcuboot_img_hdr_sz                = ((0x400 + (m_flash_sector_size -1 )) / m_flash_sector_size) * m_flash_sector_size;

/* 
 * Active application section
*/
m_app_start                         = DEFINED(__m_mcuboot_size__)  ? m_mcuboot_end + 1 : m_ivt_end + 1;
m_app_trailer_size                  = m_flash_sector_size;
m_padding_size                      = m_flash_sector_size;

/* Maximum number of sectors per slot */
m_app_max_sectors                   = DEFINED(gEVKCFlashSize_d) ? 0x400 : 0x300; /* 4 MB for evkc, otherwise 2 MB / m_sector_size */

/* The active image section contains the mcuboot header + active app + trailer 
 * Note that an empty sector (padding) must be kept at the end of the active image section,
 * this would be used by mcuboot to perform a swap-move operation of the upgrade.
 */
FW_ACTIVE_APP_START                 = m_app_start;
FW_ACTIVE_APP_OFFSET                = m_app_start - m_flash_start;
FW_ACTIVE_APP_SIZE                  = m_flash_sector_size * m_app_max_sectors - m_padding_size;
FW_ACTIVE_APP_END                   = FW_ACTIVE_APP_START + FW_ACTIVE_APP_SIZE - 1;

m_app_stated_size                   = FW_ACTIVE_APP_SIZE - m_mcuboot_img_hdr_sz - m_app_trailer_size;

m_interrupts_start                  = DEFINED(__m_mcuboot_size__)  ? m_app_start + m_mcuboot_img_hdr_sz : m_app_start;
m_interrupts_size                   = 0x00000400;
m_interrupts_end                    = m_interrupts_start + m_interrupts_size - 1;
m_text_start                        = m_interrupts_end + 1;
m_text_size                         = m_app_stated_size - m_interrupts_size;
m_text_end                          = m_text_start + m_text_size - 1;


/* Candidate application (OTA Update Image) section 
 * The size of the candidate image (slot 1) is equal to the active image (slot 0) size
*/
FW_UPDATE_STORAGE_START             = FW_ACTIVE_APP_END + 1 + m_padding_size;
FW_UPDATE_STORAGE_SIZE              = DEFINED(__m_mcuboot_size__)  ? FW_ACTIVE_APP_SIZE : 0;
FW_UPDATE_STORAGE_OFFSET            = FW_UPDATE_STORAGE_START - m_flash_start;
FW_UPDATE_STORAGE_END               = FW_UPDATE_STORAGE_START + FW_UPDATE_STORAGE_SIZE - 1;




HEAP_SIZE  = DEFINED(__heap_size__)  ? __heap_size__  : 0;
STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x0800;
VECTOR_RAM_SIZE = DEFINED(__ram_vector_table__) ? 0x00000400 : 0;

/* Specify the memory areas */
MEMORY
{
  m_flash_config        (RX)  : ORIGIN = m_boot_flash_conf_start, LENGTH = m_boot_flash_conf_size
  m_ivt                 (RX)  : ORIGIN = m_ivt_start, LENGTH = m_ivt_size
  m_interrupts          (RX)  : ORIGIN = m_interrupts_start, LENGTH = m_interrupts_size
  m_text                (RX)  : ORIGIN = m_text_start, LENGTH = m_text_size
  NVM_region            (RW)  : ORIGIN = NV_STORAGE_START_ADDRESS, LENGTH = NV_STORAGE_SIZE
  /* m_qacode zone is used to store code for functions to be executed from RAM. Adding an offset
   * to prevent overwrite in case of using NULL pointers to reference data
  */
  m_qacode              (RX)  : ORIGIN = 0x00000400, LENGTH = 0x00020000 /* ITCM 128 KB */
  m_ncache              (RW)  : ORIGIN = 0x20000000, LENGTH = 0x00020000 /* DTCM 128 KB */
  m_data                (RW)  : ORIGIN = 0x20208000, LENGTH = 0x000B8000 /* OCRAM2 480 KB + OCRAM-FlexRAM 256 KB */
}

/* Define output sections */
SECTIONS
{
  __NCACHE_REGION_START = ORIGIN(m_ncache);
  __NCACHE_REGION_SIZE  = LENGTH(m_ncache);

  /* If OTA is enabled this section should not be loaded as mcuboot already includes it*/
  .flash_config :
  {
    . = ALIGN(4);
    __FLASH_BASE = .;
    KEEP(* (.boot_hdr.conf))     /* flash config section */
    . = ALIGN(4);
  } > m_flash_config

  ivt_begin = ORIGIN(m_flash_config) + LENGTH(m_flash_config);

  /* If OTA is enabled this section should not be loaded as mcuboot already includes it*/
  .ivt : AT(ivt_begin)
  {
    . = ALIGN(4);
	KEEP(* (.boot_hdr.ivt))           /* ivt section */
	KEEP(* (.boot_hdr.boot_data))     /* boot section */
	KEEP(* (.boot_hdr.dcd_data))      /* dcd section */
    . = ALIGN(4);
  } > m_ivt

  /* The startup code goes first into internal RAM */
  .interrupts :
  {
    __VECTOR_TABLE = .;
    __Vectors = .;
    . = ALIGN(4);
    KEEP(*(.isr_vector))     /* Startup code */
    . = ALIGN(4);
  } > m_interrupts

  /* The program code and other data goes into internal RAM */
  .text :
  {
    . = ALIGN(4);
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        *fsl_adapter_flexspi_nor_flash.c.o*
        *mflash_drv.c.o*
        *fsl_flexspi.c.o*
        *queue.c.o*
        *tcp_out.c.o*
        *mlan_11n_rxreorder.c.o*
        *lib_a-memcpy-stub.o*
        *tcp_in.c.o*
        *inet_chksum.c.o*
        *timers.c.o*
        *fsl_usdhc.c.o*
        *port.c.o*
        *wifi-sdio.c.o*
        *fsl_sdio.c.o*
        *pbuf.c.o*
        *tasks.c.o*
        *lib_a-memset.o*
        *fsl_cache.c.o*
        *wifi.c.o*
      ) .text*)              /* .text* sections (code) */
    *(.rodata .rodata.* .constdata .constdata.*)
    *(.glue_7)               /* glue arm to thumb code */
    *(.glue_7t)              /* glue thumb to arm code */
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    KEEP(*freertos*/sdk.tasks.c.o(.rodata*)) /* FreeRTOS Debug Config */
    . = ALIGN(4);
  } > m_text

  ._bt_settings_area :
  {
    . = ALIGN(4);
    _bt_settings_start = .;
    KEEP(*(SORT(._bt_settings.static.*)))
    _bt_settings_end = .;
    . = ALIGN(4);
  } > m_text

  ._settings_handler_static :
  {
    . = ALIGN(4);
    _settings_handler_static_list_start = .;
    KEEP(*(SORT(._settings_handler_static.static.*)))
    _settings_handler_static_list_end = .;
    . = ALIGN(4);
  } > m_text

  ._bt_conn_cb :
  {
    . = ALIGN(4);
    _bt_conn_cb_list_start = .;
    KEEP(*(SORT(._bt_conn_cb.static.*)))
    _bt_conn_cb_list_end = .;
    . = ALIGN(4);
  } > m_text

  ._bt_gatt_service_static :
  {
    . = ALIGN(4);
    _bt_gatt_service_static_list_start = .;
    KEEP(*(SORT(._bt_gatt_service_static.static.*)))
    _bt_gatt_service_static_list_end = .;
    . = ALIGN(4);
  } > m_text

  ._bt_l2cap_fixed_chan :
  {
    . = ALIGN(4);
    _bt_l2cap_fixed_chan_list_start = .;
    KEEP(*(SORT(._bt_l2cap_fixed_chan.static.*)))
    _bt_l2cap_fixed_chan_list_end = .;
    . = ALIGN(4);
  } > m_text

  ._bt_l2cap_br_fixed_chan :
  {
    . = ALIGN(4);
    _bt_l2cap_br_fixed_chan_list_start = .;
    KEEP(*(SORT(._bt_l2cap_br_fixed_chan.static.*)))
    _bt_l2cap_br_fixed_chan_list_end = .;
    . = ALIGN(4);
  } > m_text


  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > m_text

  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } > m_text

 .ctors :
  {
    __CTOR_LIST__ = .;
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       from the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    __CTOR_END__ = .;
  } > m_text

  .dtors :
  {
    __DTOR_LIST__ = .;
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    __DTOR_END__ = .;
  } > m_text

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } > m_text

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } > m_text

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } > m_text

  __etext = .;    /* define a global symbol at end of code */
  __DATA_ROM = .; /* Symbol is used by startup for data initialization */

  .interrupts_ram :
  {
    . = ALIGN(4);
    __VECTOR_RAM__ = .;
    __interrupts_ram_start__ = .; /* Create a global symbol at data start */
    *(.m_interrupts_ram)     /* This is a user defined section */
    . += VECTOR_RAM_SIZE;
    . = ALIGN(4);
    __interrupts_ram_end__ = .; /* Define a global symbol at data end */
  } > m_data

  __VECTOR_RAM = DEFINED(__ram_vector_table__) ? __VECTOR_RAM__ : ORIGIN(m_interrupts);
  __RAM_VECTOR_TABLE_SIZE_BYTES = DEFINED(__ram_vector_table__) ? (__interrupts_ram_end__ - __interrupts_ram_start__) : 0x0;

  .data : AT(__DATA_ROM)
  {
    . = ALIGN(4);
    __DATA_RAM = .;
    __data_start__ = .;      /* create a global symbol at data start */
    *(m_usb_dma_init_data)
    *(.data)                 /* .data sections */
    *(.data*)                /* .data* sections */
    *(.wlan_data .wlan_data.*)
    KEEP(*(.jcr*))
    . = ALIGN(4);
    PROVIDE(__start_NVM_TABLE = .);
    KEEP(*(.NVM_TABLE));
    PROVIDE(__stop_NVM_TABLE = .);
    . = ALIGN(4);
    _k_mem_slab_list_start = .;
    KEEP(*(SORT(._k_mem_slab*)))
    _k_mem_slab_list_end = .;
    . = ALIGN(4);
    _net_buf_pool_list_start = .;
    KEEP(*(SORT(._net_buf_pool*)))
    _net_buf_pool_list_end = .;
    . = ALIGN(4);
    __data_end__ = .;        /* define a global symbol at data end */
  } > m_data

  __ram_function_flash_start = __DATA_ROM + (__data_end__ - __data_start__); /* Symbol is used by startup for TCM data initialization */

  .ram_function : AT(__ram_function_flash_start)
  {
    . = ALIGN(32);
    __ram_function_start__ = .;
    *(CodeQuickAccess)
    /* Explicit placement of flash and frequently executed functions in RAM */
    *fsl_adapter_flexspi_nor_flash.c.o*(.text*)
    *mflash_drv.c.o*(.text*)
    *fsl_flexspi.c.o*(.text*)
    *queue.c.o*(.text*)
    *tcp_out.c.o*(.text*)
    *mlan_11n_rxreorder.c.o*(.text*)
    *lib_a-memcpy-stub.o*(.text*)
    *tcp_in.c.o*(.text*)
    *inet_chksum.c.o*(.text*)
    *timers.c.o*(.text*)
    *fsl_usdhc.c.o*(.text*)
    *port.c.o*(.text*)
    *wifi-sdio.c.o*(.text*)
    *fsl_sdio.c.o*(.text*)
    *pbuf.c.o*(.text*)
    *tasks.c.o*(.text*)
    *lib_a-memset.o*(.text*)
    *fsl_cache.c.o*(.text*)
    *wifi.c.o*(.text*)
    . = ALIGN(128);
    __ram_function_end__ = .;
  } > m_qacode

  __NDATA_ROM = __ram_function_flash_start + (__ram_function_end__ - __ram_function_start__);
  .ncache.init : AT(__NDATA_ROM)
  {
    __noncachedata_start__ = .;   /* create a global symbol at ncache data start */
    *(NonCacheable.init)
    . = ALIGN(4);
    __noncachedata_init_end__ = .;   /* create a global symbol at initialized ncache data end */
  } > m_ncache
  . = __noncachedata_init_end__;
  .ncache :
  {
    *(NonCacheable)
    . = ALIGN(4);
    __noncachedata_end__ = .;     /* define a global symbol at ncache data end */
  } > m_ncache

  __DATA_END = __NDATA_ROM + (__noncachedata_init_end__ - __noncachedata_start__);
  text_end = ORIGIN(m_text) + LENGTH(m_text);
  ASSERT(__DATA_END <= text_end, "region m_text overflowed with text and data")

  /* Uninitialized data section */
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    . = ALIGN(4);
    __START_BSS = .;
    __bss_start__ = .;
    *(m_usb_dma_noninit_data)
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
    __END_BSS = .;
  } > m_data

  .heap (NOLOAD) :
  {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    __HeapBase = .;
    KEEP(*(.heap*))
    __HeapLimit = .;
    __heap_limit = .; /* Add for _sbrk */
  } > m_data

  .stack :
  {
    . = ALIGN(8);
    __StackLimit = .;
    . += STACK_SIZE;
    __StackTop = .;
  } > m_data

  PROVIDE(__stack = __StackTop);

  /* Remove this section to keep the nvm section on writting the device */
  .NVM :
  {
    FILL(0xFFFFFFFF);
    . = ORIGIN(NVM_region) + LENGTH(NVM_region) - 1;
    BYTE(0xFF);
  } > NVM_region

  .ARM.attributes 0 : { *(.ARM.attributes) }

  ASSERT(__StackLimit >= __HeapLimit, "region m_data overflowed with stack and heap")

  ASSERT( (DEFINED(__m_mcuboot_size__) ? (__m_mcuboot_size__ % FLASH_SECTOR_SIZE) : 0) == 0, "mcuboot size is not aligned on flash sector size")
  
  ASSERT(__FACTORY_DATA_START >= FW_UPDATE_STORAGE_END, "OTA storage section overflowed")

}
