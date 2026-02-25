/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 *    @file
 *          Provides an implementation for BLE Host NVM functions
 *          based on Matter SDK
 */
#include <stdio.h>

#include <nvs_port.h>
#include <settings.h>

#include <ble_general.h>
#include <ble_constants.h>

#define KEY_DATA_OK 0x0000
#define KEY_NOT_FOUND 0x0001
#define KEY_BUFFER_OVERFLOW 0x0002
#define KEY_READ_FAILED 0x0004

/* NVS settings prefix for OT keys */
#define BT_KEY_PREFIX "bt"

/*
 * The format of the BLE key names is "bt/x/y", where <x> is a decimal
 * digit which designates the bonded device, and <y> is a string
 * representing the bonded structure saved (e,g header, dynamic, static etc).
 */
 #define KEY_NAME_SIZE 16 /* characters */

 struct key_data
 {
    char     name[KEY_NAME_SIZE];
    size_t   len;
    uint16_t flags;
    uint8_t *data;
 };

 /*
  * Zephyr settings subtree callback function used to search and read a certain
  * key
  */
 static int bt_subtree_cb_read_value(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param)
 {
    ssize_t          cnt;
    struct key_data *k = (struct key_data *)param;

    /* Reset key flags. */
    k->flags = KEY_DATA_OK;

    /*
    * If the real size of the key exceeds the size of the key value storage
    * buffer signal a data buffer overflow.
    */
    if (len > k->len)
        k->flags |= KEY_BUFFER_OVERFLOW;

    /* Read the key value as much as it fits in the provided buffer */
    cnt = read_cb(cb_arg, k->data, k->len);
    if (cnt > 0)
    {
        k->len = (uint16_t)cnt;
    }
    else
    {
        /*
        * We have either read an empty key or there was an error reading the
        * data.
        */
        k->len = 0;
        if (cnt == 0)
        {
            k->flags |= KEY_NOT_FOUND;
        }
        else
        {
            k->flags |= KEY_READ_FAILED;
        }
    }

    /* Return 1 (one) to stop processing further keys in this subtree */
    return 1;
 }

 /*
  * Zephyr settings subtree callback function used to wipe out all the keys
  * available in the subtree
  */
 static int bt_subtree_cb_wipe(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param)
 {
    int  err;
    char key_name[KEY_NAME_SIZE];

    sprintf(key_name, BT_KEY_PREFIX "/%s", key);
    err = settings_delete(key_name);
    if (err != 0)
    {
        /* An error has occurred hence abort this operation */
        return 1;
    }

    /* Continue browsing and removing the rest of the keys in this subtree */
    return 0;
 }

 void btSettingsInit(void)
{
    const struct flash_area *fa;
    int                      err;

    /* Get flash memory driver params */
    err = flash_area_open(SETTINGS_PARTITION, &fa);
    if (err != 0)
    {
        return;
    }

    /* Flash memory init */
    err = flash_init(fa->fa_dev);
    if (err != 0)
    {
        return;
    }

    /* Zephyr Settings module init */
    err = settings_subsys_init();
}

int btSettingsGet(uint8_t index, char *key, uint8_t *value, uint16_t length)
{
    int             err = 0;
    int             key_name_len = 0;
    struct key_data k   = {.flags = KEY_NOT_FOUND, .len = length, .data = value};

    /* key len + prefix len + index (one digit number) + 2 * '/' + null char */
    key_name_len = strlen(BT_KEY_PREFIX) + strlen(key) + 4;
    if (key_name_len > KEY_NAME_SIZE)
        return -1;

    /* Generate the name of the key */
    sprintf(k.name, BT_KEY_PREFIX "/%d/%s", index, key);
    err = settings_load_subtree_direct(k.name, bt_subtree_cb_read_value, &k);
    if ((err != 0) || ((k.flags & KEY_NOT_FOUND) != 0) || ((k.flags & KEY_READ_FAILED) != 0))
    {
        err = 1;
    }

    return err;
}

int btSettingsSet(uint8_t index, char *key, uint8_t *value, uint16_t length)
{
    int  err = 0;
    int  key_name_len = 0;
    char key_name[KEY_NAME_SIZE];

    /* key len + prefix len + index (one digit number) + 2 * '/' + null char */
    key_name_len = strlen(BT_KEY_PREFIX) + strlen(key) + 4;
    if (key_name_len > KEY_NAME_SIZE)
        return -1;

    /* Generate the name of the key */
    sprintf(key_name, BT_KEY_PREFIX "/%d/%s", index, key);
    err = settings_save_one(key_name, value, length);

    return err;
}

int btSettingsDelete(uint8_t index)
{
    int  err = 0;
    int  key_name_len = 0;
    char key_name[KEY_NAME_SIZE];

    /* prefix len + index (one digit number) + 1 * '/' + null char */
    key_name_len = strlen(BT_KEY_PREFIX) + 3;
    if (key_name_len > KEY_NAME_SIZE)
        return -1;

    /* Generate the name of the key */
    sprintf(key_name, BT_KEY_PREFIX "/%d", index);
    err = settings_delete(key_name);

    return err;
}

void btSettingsWipe(void)
{
    settings_load_subtree_direct(BT_KEY_PREFIX, bt_subtree_cb_wipe, NULL);
}


/*******************************************************************************
 * Functions needed by the BLE stack
 ******************************************************************************/
bleResult_t App_NvmRead(uint8_t mEntryIdx, void * pBondHeader, void * pBondDataDynamic, void * pBondDataStatic, void * pBondDataDeviceInfo,
    void * pBondDataDescriptor, uint8_t mDescriptorIndex)
{
    int ret = 0;

    if (pBondHeader) {
        ret = btSettingsGet(mEntryIdx, "bh", pBondHeader, gBleBondIdentityHeaderSize_c);
        if (ret)
            return gBleNVMError_c;
    }

    if (pBondDataDynamic) {
        ret = btSettingsGet(mEntryIdx, "bdyn", pBondDataDynamic, gBleBondDataDynamicSize_c);
        if (ret)
            return gBleNVMError_c;
    }

    if (pBondDataStatic) {
        ret = btSettingsGet(mEntryIdx, "bsta", pBondDataStatic, gBleBondDataStaticSize_c);
        if (ret)
            return gBleNVMError_c;
    }

    if (pBondDataDeviceInfo) {
        ret = btSettingsGet(mEntryIdx, "bdev", pBondDataDeviceInfo, gBleBondDataDeviceInfoSize_c);
        if (ret)
            return gBleNVMError_c;
    }

    if (pBondDataDescriptor) {
        ret = btSettingsGet(mEntryIdx, "bdes", pBondDataDescriptor, gBleBondDataDescriptorSize_c);
        if (ret)
            return gBleNVMError_c;
    }

    if (mDescriptorIndex) {
        ret = btSettingsGet(mEntryIdx, "bidx", &mDescriptorIndex, sizeof(uint8_t));
        if (ret)
            return gBleNVMError_c;
    }

    return gBleSuccess_c;
}

bleResult_t App_NvmWrite(uint8_t mEntryIdx, void * pBondHeader, void * pBondDataDynamic, void * pBondDataStatic,
    void * pBondDataDeviceInfo, void * pBondDataDescriptor, uint8_t mDescriptorIndex)
{
    int ret = 0;

    if (pBondHeader) {
        ret = btSettingsSet(mEntryIdx, "bh", pBondHeader, gBleBondIdentityHeaderSize_c);
        if (ret)
            return gBleNVMError_c;
    }

    if (pBondDataDynamic) {
        ret = btSettingsSet(mEntryIdx, "bdyn", pBondDataDynamic, gBleBondDataDynamicSize_c);
        if (ret)
            return gBleNVMError_c;
    }

    if (pBondDataStatic) {
        ret = btSettingsSet(mEntryIdx, "bsta", pBondDataStatic, gBleBondDataStaticSize_c);
        if (ret)
            return gBleNVMError_c;
    }

    if (pBondDataDeviceInfo) {
        ret = btSettingsSet(mEntryIdx, "bdev", pBondDataDeviceInfo, gBleBondDataDeviceInfoSize_c);
        if (ret)
            return gBleNVMError_c;
    }

    if (pBondDataDescriptor) {
        ret = btSettingsSet(mEntryIdx, "bdes", pBondDataDescriptor, gBleBondDataDescriptorSize_c);
        if (ret)
            return gBleNVMError_c;
    }

    if (mDescriptorIndex) {
        ret = btSettingsSet(mEntryIdx, "bidx", &mDescriptorIndex, sizeof(uint8_t));
        if (ret)
            return gBleNVMError_c;
    }

    return gBleSuccess_c;
}

bleResult_t App_NvmErase(uint8_t mEntryIdx)
{
    int ret = 0;

    ret = btSettingsDelete(mEntryIdx);
    if (ret)
        return gBleNVMError_c;

    return gBleSuccess_c;
}