/*
 *  Copyright (c) 2022-2023, 2025 NXP.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NXP_MATTER_MBEDTLS_CONFIG_H
#define NXP_MATTER_MBEDTLS_CONFIG_H

#include "mcux_mbedtls_accelerator_config.h"
#include <stdio.h>
//#include "FreeRTOS.h"


#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf
#define MBEDTLS_PLATFORM_C


#ifndef MBEDTLS_PSA_CRYPTO_STORAGE_C
#define MBEDTLS_PSA_CRYPTO_STORAGE_C 1
#endif // MBEDTLS_PSA_CRYPTO_STORAGE_C

// Algorithm flags
#define PSA_WANT_ALG_SHA_256 1
#define PSA_WANT_ALG_ECDSA 1
#define PSA_WANT_ALG_DETERMINISTIC_ECDSA 1
#define PSA_WANT_ALG_ECDH 1
#define PSA_WANT_ALG_HMAC 1
#define PSA_WANT_ALG_HKDF 1
#define PSA_WANT_ALG_HKDF_EXTRACT 1
#define PSA_WANT_ALG_HKDF_EXPAND 1
#define PSA_WANT_ALG_CCM 1
/* Openthread and factory data support */
#define PSA_WANT_ALG_ECB_NO_PADDING 1
/* Use HMAC instead as CMAC execute software aes operation */
//#define PSA_WANT_ALG_CMAC 1

/* Use AES CMAC for BLE connections in Matter BLE CS context */
#if CONFIG_CHIP_SDK_DEPENDENCIES_BLE_HOST_CS
#define PSA_WANT_ALG_CMAC 1
#endif

/* should be set to at least the sum of:
 * - CHIP_CONFIG_SECURE_SESSION_POOL_SIZE * 2
 *   each CASE/PASE requires 2 key slots: i2r + r2i
 * - PSA key slots required for the transport layer
 *   (e.g.: for Thread at least 3 PSA Key Slots: the Network Key, MLE Key and MAC Key)
 * - additional PSA key slots depending on the app use-case
 *
 * On a 32-bit system, psa_key_slot_t requires 64 to 80 bytes of SRAM when
 * MBEDTLS_PSA_KEY_STORE_DYNAMIC and MBEDTLS_PSA_STATIC_KEY_SLOT_BUFFER_SIZE 
 * are not defined.
*/
#define MBEDTLS_PSA_KEY_SLOT_COUNT 64

#if CONFIG_NET_L2_OPENTHREAD
#if CONFIG_CHIP_OPENTHREAD_FTD
#define PSA_WANT_ALG_PBKDF2_AES_CMAC_PRF_128 1
#endif
#define PSA_WANT_KEY_TYPE_PASSWORD 1
#define PSA_WANT_KEY_TYPE_RAW_DATA 1
#endif


// Curve flags
#define PSA_WANT_ECC_SECP_R1_256 1

// Key type flags
#define PSA_WANT_KEY_TYPE_AES 1
#define PSA_WANT_KEY_TYPE_HMAC 1
#define PSA_WANT_KEY_TYPE_DERIVE 1

#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR 1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC 1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_IMPORT 1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_EXPORT 1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_GENERATE 1
#define PSA_WANT_KEY_TYPE_ECC_PUBLIC_KEY 1


#ifndef MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_C
#endif
#ifndef MBEDTLS_THREADING_ALT
#define MBEDTLS_THREADING_ALT
#endif

#define MBEDTLS_PSA_CRYPTO_C            1
#define MBEDTLS_PSA_CRYPTO_CONFIG       1

#define MBEDTLS_USE_PSA_CRYPTO

#if defined (CONFIG_NET_L2_OPENTHREAD) && defined (CONFIG_CHIP_WIFI)

#define PSA_WANT_ALG_JPAKE

#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_SSL_PROTO_DTLS

/*
 * Define MBEDTLS_SSL_MAX_CONTENT_LEN as 2000 if OPENTHREAD_CONFIG_BLE_TCAT_ENABLE
 * Define MBEDTLS_SSL_MAX_CONTENT_LEN as 900 if OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
*/
#ifdef MBEDTLS_SSL_MAX_CONTENT_LEN
#undef MBEDTLS_SSL_MAX_CONTENT_LEN
#endif // MBEDTLS_SSL_MAX_CONTENT_LEN
#define MBEDTLS_SSL_MAX_CONTENT_LEN      768 /**< Maxium fragment length in bytes */

#define MBEDTLS_SSL_IN_CONTENT_LEN       MBEDTLS_SSL_MAX_CONTENT_LEN
#define MBEDTLS_SSL_OUT_CONTENT_LEN      MBEDTLS_SSL_MAX_CONTENT_LEN
#define MBEDTLS_SSL_CIPHERSUITES         MBEDTLS_TLS_ECJPAKE_WITH_AES_128_CCM_8
#endif // CONFIG_NET_L2_OPENTHREAD && CONFIG_CHIP_WIFI

#if CONFIG_CHIP_BUILD_TESTS
/* Needed for matter certificate operation */
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_CSR_PARSE_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_OID_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_ECP_C
#endif

#endif // NXP_MATTER_MBEDTLS_CONFIG_H
