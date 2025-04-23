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

/* FreeRTOS is always supported for RW612 and RT platforms so enable threading */
#define MBEDTLS_MCUX_FREERTOS_THREADING_ALT
#ifndef MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_C
#endif // MBEDTLS_THREADING_C
#ifndef MBEDTLS_THREADING_ALT
#define MBEDTLS_THREADING_ALT
#endif // MBEDTLS_THREADING_ALT

#ifndef MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#endif // MBEDTLS_ENTROPY_HARDWARE_ALT

/* OpenThread mbedtls config defines MBEDTLS_ECP_MAX_BITS to 256 so we need to disable
 * els_pkc features that require a higher value */
#ifdef MBEDTLS_ECP_DP_SECP384R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP384R1_ENABLED
#endif // MBEDTLS_ECP_DP_SECP384R1_ENABLED
#ifdef MBEDTLS_ECP_DP_SECP521R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP521R1_ENABLED
#endif // MBEDTLS_ECP_DP_SECP521R1_ENABLED
#ifdef MBEDTLS_ECP_DP_BP384R1_ENABLED
#undef MBEDTLS_ECP_DP_BP384R1_ENABLED
#endif // MBEDTLS_ECP_DP_BP384R1_ENABLED
#ifdef MBEDTLS_ECP_DP_BP512R1_ENABLED
#undef MBEDTLS_ECP_DP_BP512R1_ENABLED
#endif // MBEDTLS_ECP_DP_BP512R1_ENABLED

#include <stdio.h>
//#include "FreeRTOS.h"

#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf

#if CONFIG_NET_L2_OPENTHREAD
#define MBEDTLS_AES_C
#if (MBEDTLS_VERSION_NUMBER >= 0x03050000)
#define MBEDTLS_AES_ONLY_128_BIT_KEY_LENGTH
#endif
#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BIGNUM_C
#if (MBEDTLS_VERSION_NUMBER >= 0x03050000)
#define MBEDTLS_BLOCK_CIPHER_NO_DECRYPT
#endif
#define MBEDTLS_CCM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CMAC_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_DEPRECATED_REMOVED
#define MBEDTLS_DEPRECATED_WARNING
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_NIST_OPTIM
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_MD_C
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_OID_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
#define MBEDTLS_SHA224_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA256_SMALLER
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_DTLS_ANTI_REPLAY
#define MBEDTLS_SSL_DTLS_HELLO_VERIFY
#define MBEDTLS_SSL_EXPORT_KEYS
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_PROTO_DTLS
#define MBEDTLS_SSL_TLS_C

#ifdef MBEDTLS_MCUX_USE_ELS
/* Enable PSA */
#define MBEDTLS_PSA_BUILTIN_ALG_SHA_256 1
#define MBEDTLS_PSA_CRYPTO_C
#define MBEDTLS_PSA_CRYPTO_CONFIG

#define MBEDTLS_ENTROPY_FORCE_SHA256

/* need to be defined to 0, else config_psa.h will define it to 1 and ecp.h will define wrong MBEDTLS_ECP_MAX_BITS_MIN value*/
#define MBEDTLS_PSA_ACCEL_ECC_SECP_R1_521 0
#define MBEDTLS_PSA_ACCEL_ECC_BRAINPOOL_P_R1_512 0
#define MBEDTLS_PSA_ACCEL_ECC_BRAINPOOL_P_R1_384 0
#define MBEDTLS_PSA_ACCEL_ECC_SECP_R1_384 0
#endif /* MBEDTLS_MCUX_USE_ELS */

#endif // CONFIG_NET_L2_OPENTHREAD

#if !defined(MBEDTLS_PLATFORM_STD_CALLOC) && !defined(MBEDTLS_PLATFORM_STD_FREE)
#if defined(USE_RTOS) && defined(SDK_OS_FREE_RTOS)
#include "FreeRTOS.h"

#ifdef MBEDTLS_MCUX_USE_ELS
/* Allows to align with RT configuration to have mbedtls allocation redirected to freeRTOS functions for both Matter over Wi-Fi and Matter over Thread */
void *pvPortCalloc(size_t num, size_t size); /*Calloc for HEAP3.*/

#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_STD_CALLOC pvPortCalloc
#define MBEDTLS_PLATFORM_STD_FREE vPortFree

#endif /* USE_RTOS*/
#endif /* !defined(MBEDTLS_PLATFORM_STD_CALLOC) && !defined(MBEDTLS_PLATFORM_STD_FREE) */

#endif /* MBEDTLS_MCUX_USE_ELS */

#if defined CRYPTO_USE_DRIVER_CAAM || ( defined(FSL_FEATURE_SOC_DCP_COUNT) && (FSL_FEATURE_SOC_DCP_COUNT > 0))
#ifdef MBEDTLS_SHA512_C
#undef MBEDTLS_SHA512_C /* RT1060  HW crypto module does not support SDH512, therefore it is mandatory to disable it otherwise crypto operation would be done in software (example: ecp_drbg_seed). To align all RT configs it would be also disabled on RT1170 */
#endif
#endif
/*
 * These configs are required if OPENTHREAD_CONFIG_BORDER_AGENT_ENABLE \
 * || OPENTHREAD_CONFIG_COMMISSIONER_ENABLE || OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
*/
#if CONFIG_CHIP_WIFI && CONFIG_NET_L2_OPENTHREAD
#define MBEDTLS_SSL_COOKIE_C
#define MBEDTLS_SSL_SRV_C
#endif

/* 
 * Define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED if OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
*/
//#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED

/*
 * Define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED 
 * if (OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE || OPENTHREAD_CONFIG_TLS_ENABLE)
*/
//#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED

/*
 * Define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE and MBEDTLS_GCM_C
 * if OPENTHREAD_CONFIG_BLE_TCAT_ENABLE
*/
//#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
//#define MBEDTLS_GCM_C

#ifdef MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_BASE64_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#endif

/*
 * These macros are required if OPENTHREAD_CONFIG_ECDSA_ENABLE
*/
#if CONFIG_NET_L2_OPENTHREAD
#define MBEDTLS_BASE64_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECDSA_DETERMINISTIC
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PK_WRITE_C
#endif

#define MBEDTLS_MPI_WINDOW_SIZE            1 /**< Maximum windows size used. */
#ifdef MBEDTLS_MPI_MAX_SIZE
#undef MBEDTLS_MPI_MAX_SIZE
#endif // MBEDTLS_MPI_MAX_SIZE
#define MBEDTLS_MPI_MAX_SIZE              32 /**< Maximum number of bytes for usable MPIs. */
#ifdef MBEDTLS_ECP_MAX_BITS
#undef MBEDTLS_ECP_MAX_BITS
#endif // MBEDTLS_ECP_MAX_BITS
#define MBEDTLS_ECP_MAX_BITS             256 /**< Maximum bit size of groups */
#define MBEDTLS_ECP_WINDOW_SIZE            2 /**< Maximum window size used */
#define MBEDTLS_ECP_FIXED_POINT_OPTIM      0 /**< Enable fixed-point speed-up */
#define MBEDTLS_ENTROPY_MAX_SOURCES        1 /**< Maximum number of sources supported */

/* Not required to be enabled for MCXW platform as calloc and free would be register dynamically during Matter boot.
For RTs such confis are enabled in KSDK_mbedtls_config,h and for RW it has already been defined above */
//#define MBEDTLS_PLATFORM_STD_CALLOC      pvPortMalloc /**< Default allocator to use, can be undefined */
//#define MBEDTLS_PLATFORM_STD_FREE        vPortFree /**< Default free to use, can be undefined */

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


#include "mbedtls/version.h"
#if (MBEDTLS_VERSION_NUMBER < 0x03000000)
    // Configuration sanity check. Done automatically in Mbed TLS >= 3.0.
    #include "mbedtls/check_config.h"
#endif


/* Undef this flag to make sure to use hardware entropy */
#undef MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES

#endif // NXP_MATTER_MBEDTLS_CONFIG_H
