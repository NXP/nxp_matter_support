/*
 * Copyright 2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RT1060_MATTER_MBEDTLS_CONFIG_H
#define RT1060_MATTER_MBEDTLS_CONFIG_H

/* MBEDTLS_PSA_CRYPTO_C required MBEDTLS_CTR_DRBG_C or MBEDTLS_HMAC_DRBG_C or MBEDTLS_ENTROPY_C or MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG */
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_NO_PLATFORM_ENTROPY

// Algorithm acceleration flags
/* MBEDTLS_MD_SOME_PSA definition produce mbedtls_md_starts error because DCP didn't support HASH acceleration
Disable HASH acceleration by default because of CACHE consistency
 */
//#define MBEDTLS_PSA_ACCEL_ALG_SHA_256 1
#define MBEDTLS_PSA_ACCEL_ALG_ECDSA 1
/* Couldn't enable it due to MBEDTLS_HMAC_DRBG_C definition */
//#define MBEDTLS_PSA_ACCEL_ALG_DETERMINISTIC_ECDSA 1
/* Not supported by DCP */
//#define MBEDTLS_PSA_ACCEL_ALG_ECDH 1

/* Not supported by DCP */
//#define MBEDTLS_PSA_ACCEL_ALG_HMAC 1

// Need Software implementation when using PSA_ALG_HKDF(PSA_ALG_SHA_256) like in PsaKdf::Init
//#define MBEDTLS_PSA_ACCEL_ALG_HKDF 1
#define MBEDTLS_PSA_ACCEL_ALG_HKDF_EXTRACT 1
#define MBEDTLS_PSA_ACCEL_ALG_HKDF_EXPAND 1

/* Not supported by DCP */
//#define MBEDTLS_PSA_ACCEL_ALG_CCM 1


#if CONFIG_NET_L2_OPENTHREAD
#define MBEDTLS_PSA_ACCEL_ALG_ECB_NO_PADDING 1
#endif

// Curve acceleration flags
#define MBEDTLS_PSA_ACCEL_ECC_SECP_R1_256 1

// Key type acceleration flags
/* Not supported by DCP */
//#define MBEDTLS_PSA_ACCEL_KEY_TYPE_AES 1

/* Not supported by DCP */
//#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR 1
//#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_BASIC 1
//#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_IMPORT 1
//#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_EXPORT 1
//#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_GENERATE 1
//#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_PUBLIC_KEY 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_DERIVE 1


#endif /* RT1060_MATTER_MBEDTLS_CONFIG_H */