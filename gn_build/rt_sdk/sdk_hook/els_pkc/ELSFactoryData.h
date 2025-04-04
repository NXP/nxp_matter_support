/*
 * Copyright 2023,2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ELS_FACTORY_DATA_H__
#define __ELS_FACTORY_DATA_H__

#include <fsl_debug_console.h>
#include <fsl_device_registers.h>

#include "fsl_common.h"
#include "mbedtls/bignum.h"
#include "mbedtls/ecp.h"
#include "mcuxClAes.h"
#include "mcuxClEls_Cipher.h"
#include "mcuxClEls_Cmac.h"
#include "mcuxClEls_Ecc.h"
#include "mcuxClEls_Hash.h"
#include "mcuxClEls_Kdf.h"
#include "mcuxClEls_KeyManagement.h"
#include "mcuxClEls_Rng.h"
#include "mcuxClEls_Types.h"
#include "mcuxClHashModes_Constants.h"
#include "mcuxClHash_Constants.h"

#include "psa/crypto.h"

#include "mbedtls/ecdh.h"
#include "mbedtls/entropy.h"
#include "mbedtls/nist_kw.h"
#include "mbedtls/x509_crt.h"

#define BLOCK_SIZE_16_BYTES 16
#define SHA256_OUTPUT_SIZE 32
#define HASH_ID 0xCE47BA5E
#define HASH_LEN 4
#define CBC_INITIAL_VECTOR_SIZE 16

#define STATUS_SUCCESS 0
#define STATUS_ERROR_GENERIC 1

#define AES_BLOCK_SIZE 16U
#define DIE_INT_MK_SK_INDEX 0U

#define ELS_BLOB_METADATA_SIZE 8
#define MAX_ELS_KEY_SIZE 32
#define ELS_WRAP_OVERHEAD 8

#if FACTORY_DATA_PROVIDER_LOG
#define PLOG_ERROR(...)                                                                                                            \
    for (;;)                                                                                                                       \
    {                                                                                                                              \
        PRINTF("ERROR  ");                                                                                                         \
        PRINTF(__VA_ARGS__);                                                                                                       \
        PRINTF(" (%s:%d)\n", __FILE__, __LINE__);                                                                                  \
        break;                                                                                                                     \
    }
#else
#define PLOG_ERROR(...)
#endif

#if FACTORY_DATA_PROVIDER_LOG
#define PLOG_INFO(...)                                                                                                             \
    for (;;)                                                                                                                       \
    {                                                                                                                              \
        PRINTF("INFO  ");                                                                                                          \
        PRINTF(__VA_ARGS__);                                                                                                       \
        PRINTF("\n");                                                                                                              \
        break;                                                                                                                     \
    }
#else
#define PLOG_INFO(...)
#endif

#if FACTORY_DATA_PROVIDER_LOG
#define PLOG_DEBUG(...)                                                                                                            \
    for (;;)                                                                                                                       \
    {                                                                                                                              \
        PRINTF("DEBUG  ");                                                                                                         \
        PRINTF(__VA_ARGS__);                                                                                                       \
        PRINTF("\n");                                                                                                              \
        break;                                                                                                                     \
    }
#else
#define PLOG_DEBUG(...)
#endif

#if FACTORY_DATA_PROVIDER_LOG
#define PLOG_DEBUG_BUFFER(...) printf_buffer(__VA_ARGS__)
#else
#define PLOG_DEBUG_BUFFER(...)
#endif

#define RET_MBEDTLS_SUCCESS_OR_EXIT_MSG(MSG, ...)                                                                                  \
    if (0 != ret)                                                                                                                  \
    {                                                                                                                              \
        status = STATUS_ERROR_GENERIC;                                                                                             \
        PLOG_ERROR(MSG, __VA_ARGS__);                                                                                              \
        goto exit;                                                                                                                 \
    }

#define STATUS_SUCCESS_OR_EXIT_MSG(MSG, ...)                                                                                       \
    if (STATUS_SUCCESS != status)                                                                                                  \
    {                                                                                                                              \
        PLOG_ERROR(MSG, __VA_ARGS__);                                                                                              \
        goto exit;                                                                                                                 \
    }

#define ASSERT_OR_EXIT_MSG(CONDITION, ...)                                                                                         \
    if (!(CONDITION))                                                                                                              \
    {                                                                                                                              \
        status = STATUS_ERROR_GENERIC;                                                                                             \
        PLOG_ERROR(__VA_ARGS__);                                                                                                   \
        goto exit;                                                                                                                 \
    }

#define NXP_DIE_INT_IMPORT_KEK_SK 0x7FFF817CU
#define NXP_DIE_INT_IMPORT_AUTH_SK 0x7FFF817EU

#define EL2GO_MAX_BLOB_SIZE 3072U
#define EL2GO_MAX_CERT_SIZE 2048U
#define PUBLIC_KEY_SIZE 64U

// Internal defines
#define MAGIC_TLV_SIZE 0x0D
#define MAGIC_TLV_1 0x400B6564U
#define MAGIC_TLV_2 0x67656C6FU
#define MAGIC_TLV_3 0x636B3267U
#define MAGIC_TLV_4 0x6FU
#define EL2GOOEM_MK_SK_ID 4U
#define EL2GOIMPORT_KEK_SK_ID 14U
#define EL2GOIMPORTTFM_KEK_SK_ID 16U
#define EL2GOIMPORT_AUTH_SK_ID 18U
#define CMAC_BLOCK_SIZE 16U
#define MAX_TLV_BYTE_LENGTH 0x4u

// Tags used in Secure elements blobs
#define TAG_MAGIC 0x40u
#define TAG_KEY_ID 0x41u
#define TAG_PSA_PERMITTED_ALGORITHM 0x42u
#define TAG_PSA_USAGE 0x43u
#define TAG_PSA_KEY_TYPE 0x44u
#define TAG_PSA_KEY_BITS 0x45u
#define TAG_PSA_KEY_LIFETIME 0x46u
#define TAG_PSA_DEVICE_LIFECYLE 0x47u
#define TAG_WRAPPING_KEY_ID 0x50u
#define TAG_WRAPPING_ALGORITHM 0x51u
#define TAG_IV 0x52u
#define TAG_SIGNATURE_KEY_ID 0x53u
#define TAG_SIGNATURE_ALGORITHM 0x54u
#define TAG_PLAIN 0x55u
#define TAG_SIGNATURE 0x5Eu

#define MBEDTLS_PK_ECP_PUB_DER_MAX_BYTES (30 + 2 * MBEDTLS_ECP_MAX_BYTES)

const mcuxClEls_KeyProp_t keypair_prop = { .bits = {
                                               .ksize       = MCUXCLELS_KEYPROPERTY_KEY_SIZE_256,
                                               .upprot_priv = MCUXCLELS_KEYPROPERTY_PRIVILEGED_TRUE,
                                               .upprot_sec  = MCUXCLELS_KEYPROPERTY_SECURE_TRUE,

                                           } };

const mcuxClEls_KeyProp_t shared_secret_prop = {
    .bits =
    {
        .ksize       = MCUXCLELS_KEYPROPERTY_KEY_SIZE_128,
        .uckdf       = MCUXCLELS_KEYPROPERTY_CKDF_TRUE,
        .upprot_priv = MCUXCLELS_KEYPROPERTY_PRIVILEGED_TRUE,
        .upprot_sec  = MCUXCLELS_KEYPROPERTY_SECURE_TRUE,


    },
};

const mcuxClEls_KeyProp_t wrap_in_key_prop = {
    .bits =
        {
            .ksize       = MCUXCLELS_KEYPROPERTY_KEY_SIZE_128,
            .kactv       = MCUXCLELS_KEYPROPERTY_ACTIVE_TRUE,
            .ukuok       = MCUXCLELS_KEYPROPERTY_KUOK_TRUE,
            .upprot_priv = MCUXCLELS_KEYPROPERTY_PRIVILEGED_TRUE,
            .upprot_sec  = MCUXCLELS_KEYPROPERTY_SECURE_TRUE,

        },
};

const uint8_t ckdf_derivation_data_wrap_in[12] = {
    0xc8, 0xac, 0x48, 0x88, 0xa6, 0x1b, 0x3d, 0x9b, 0x56, 0xa9, 0x75, 0xe7,
};

const mcuxClEls_KeyProp_t wrap_out_key_prop = {
    .bits =
        {
            .ksize       = MCUXCLELS_KEYPROPERTY_KEY_SIZE_256,
            .kactv       = MCUXCLELS_KEYPROPERTY_ACTIVE_TRUE,
            .ukwk        = MCUXCLELS_KEYPROPERTY_KWK_TRUE,
            .upprot_priv = MCUXCLELS_KEYPROPERTY_PRIVILEGED_TRUE,
            .upprot_sec  = MCUXCLELS_KEYPROPERTY_SECURE_TRUE,


        },
};

const uint8_t ckdf_derivation_data_wrap_out[12] = {
    0x4e, 0x5f, 0x0a, 0x1c, 0x43, 0x37, 0x2c, 0xd0, 0x54, 0x8e, 0x46, 0xc9,
};

const mcuxClEls_KeyProp_t mac_key_prop = {
    .bits =
        {
            .ksize       = MCUXCLELS_KEYPROPERTY_KEY_SIZE_256,
            .kactv       = MCUXCLELS_KEYPROPERTY_ACTIVE_TRUE,
            .ucmac       = MCUXCLELS_KEYPROPERTY_CMAC_TRUE,
            .upprot_priv = MCUXCLELS_KEYPROPERTY_PRIVILEGED_TRUE,
            .upprot_sec  = MCUXCLELS_KEYPROPERTY_SECURE_TRUE,
        },
};

const uint8_t ckdf_derivation_data_mac[12] = {
    0xea, 0x93, 0x05, 0x7a, 0x50, 0xb6, 0x4d, 0x58, 0x0a, 0xe6, 0x6b, 0x57,
};

const uint8_t import_die_int_ecdh_sk[32] = {
    0x82, 0x9b, 0xb4, 0x4a, 0x3b, 0x6d, 0x73, 0x35, 0x09, 0x5e, 0xd9, 0x8d, 0xf6, 0x09, 0x89, 0x98,
    0xac, 0x63, 0xab, 0x4e, 0x4e, 0x78, 0xf6, 0x0a, 0x70, 0xea, 0x64, 0x92, 0xd4, 0xfc, 0xe4, 0x92,
};

const uint8_t import_die_int_ecdh_pk[64] = {
    0x8c, 0xe2, 0x3a, 0x89, 0xe7, 0xc5, 0xe9, 0xb1, 0x3e, 0x89, 0xed, 0xdb, 0x69, 0xb9, 0x22, 0xf8,
    0xc2, 0x8f, 0x5d, 0xcc, 0x59, 0x3e, 0x5f, 0x7b, 0x6e, 0x5a, 0x6c, 0xb3, 0x62, 0xc0, 0x17, 0x8a,
    0x2f, 0xda, 0xe8, 0x72, 0x67, 0x7b, 0xdf, 0xfe, 0xdb, 0x4a, 0x6e, 0x39, 0x2a, 0x1b, 0xae, 0xf8,
    0x88, 0x8f, 0xc5, 0x11, 0xc3, 0x67, 0x85, 0x5a, 0xc5, 0x54, 0xbb, 0xeb, 0x19, 0xf6, 0x52, 0x66,
};

const uint8_t key_blob_magic[7] = { 'k', 'e', 'y', 'b', 'l', 'o', 'b' };

const size_t s50_blob_size = 100;

const char nibble_to_char[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

typedef struct
{
    size_t size;
    uint8_t data[EL2GO_MAX_CERT_SIZE];
} buffer_t;

typedef struct blob_context
{
    const uint8_t * magic;
    size_t magic_size;
    uint32_t key_id;
    uint32_t psa_permitted_algorithm;
    uint32_t psa_usage;
    uint32_t psa_key_type;
    uint32_t psa_key_bits;
    uint32_t psa_key_lifetime;
    uint32_t psa_device_lifecycle;
    uint32_t wrapping_key_id;
    uint32_t wrapping_algorithm;
    const uint8_t * iv;
    uint32_t signature_key_id;
    uint32_t signature_algorithm;
    const uint8_t * plain_blob;
    size_t plain_blob_size;
    const uint8_t * signature;
    size_t signature_size;
} blob_context_t;

uint8_t * append_u32(uint8_t * pos, uint32_t val);
uint8_t * append_u16(uint8_t * pos, uint32_t val);
void write_uint32_msb_first(uint8_t * pos, uint32_t data);
void printf_buffer(const char * name, const unsigned char * buffer, size_t size);
uint32_t get_required_keyslots(mcuxClEls_KeyProp_t prop);
bool els_is_active_keyslot(mcuxClEls_KeyIndex_t keyIdx);
status_t els_enable();
status_t els_get_key_properties(mcuxClEls_KeyIndex_t key_index, mcuxClEls_KeyProp_t * key_properties);
mcuxClEls_KeyIndex_t els_get_free_keyslot(uint32_t required_keyslots);
status_t els_delete_key(mcuxClEls_KeyIndex_t key_index);
status_t els_keygen(mcuxClEls_KeyIndex_t key_index, uint8_t * public_key, size_t * public_key_size);
status_t import_die_int_wrapped_key_into_els(const uint8_t * wrapped_key, size_t wrapped_key_size,
                                             mcuxClEls_KeyProp_t key_properties, mcuxClEls_KeyIndex_t * index_output);
status_t ELS_sign_hash(uint8_t * digest, mcuxClEls_EccByte_t * ecc_signature, mcuxClEls_EccSignOption_t * sign_options,
                       mcuxClEls_KeyIndex_t key_index);
status_t ELS_Cipher_Aes_Ecb(mcuxClEls_KeyIndex_t key_index, uint8_t const * input, size_t input_length, uint8_t * output,
                            bool is_encrypt);

status_t import_plain_key_into_els(const uint8_t * plain_key, size_t plain_key_size, mcuxClEls_KeyProp_t key_properties,
                                   mcuxClEls_KeyIndex_t * index_output);
status_t export_key_from_els(mcuxClEls_KeyIndex_t key_index, uint8_t * output, size_t * output_size);

status_t read_el2go_blob(const uint8_t * blob_area, size_t blob_area_size, size_t object_id, uint8_t * blob, size_t blob_size,
                         size_t * blob_length);
status_t import_el2go_key_in_els(const uint8_t * blob, size_t blob_size, mcuxClEls_KeyIndex_t * key_index);
status_t decrypt_el2go_cert_blob(const uint8_t * blob, size_t blob_size, uint8_t * cert, size_t cert_size, size_t * cert_length);
#endif
