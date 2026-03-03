/*
 * Copyright 2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RW61X_MATTER_MBEDTLS_CONFIG_H
#define RW61X_MATTER_MBEDTLS_CONFIG_H

/* MBEDTLS_PSA_CRYPTO_C required MBEDTLS_CTR_DRBG_C or MBEDTLS_HMAC_DRBG_C or MBEDTLS_ENTROPY_C or MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG */
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_NO_PLATFORM_ENTROPY

// Algorithm acceleration flags
#define MBEDTLS_PSA_ACCEL_ALG_SHA_256 1
#define MBEDTLS_PSA_ACCEL_ALG_ECDSA 1
/* Couldn't enable it due to MBEDTLS_HMAC_DRBG_C definition */
//#define MBEDTLS_PSA_ACCEL_ALG_DETERMINISTIC_ECDSA 1
#define MBEDTLS_PSA_ACCEL_ALG_ECDH 1
/* couldn't set it because openthread init need to do HMAC with SHA256 and it is not supported with HW acceleration

openthread caller: psa_mac_sign_setup(operation, aKey->mKeyRef, PSA_ALG_HMAC(PSA_ALG_SHA_256))
mcuxClPsaDriver_psa_driver_wrapper_mac_setupLayer function return error:

    No support for multipart Hmac
    if(PSA_ALG_IS_HMAC(alg) == true)
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }
*/
//#define MBEDTLS_PSA_ACCEL_ALG_HMAC 1

// Need Software implementation when using PSA_ALG_HKDF(PSA_ALG_SHA_256) like in PsaKdf::Init
//#define MBEDTLS_PSA_ACCEL_ALG_HKDF 1
#define MBEDTLS_PSA_ACCEL_ALG_HKDF_EXTRACT 1
#define MBEDTLS_PSA_ACCEL_ALG_HKDF_EXPAND 1
#define MBEDTLS_PSA_ACCEL_ALG_CCM 1

// Not supported: els_pkc_transparent_import_key return PSA_ERROR_NOT_SUPPORTED
//#define MBEDTLS_PSA_ACCEL_ALG_ECB_NO_PADDING 1


#if CONFIG_NET_L2_OPENTHREAD
#define MBEDTLS_PSA_ACCEL_ALG_PBKDF2_AES_CMAC_PRF_128 1
#endif


// Curve acceleration flags
#define MBEDTLS_PSA_ACCEL_ECC_SECP_R1_256 1

// Key type acceleration flags
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_AES 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_HMAC 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_DERIVE 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_RAW_DATA 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_PASSWORD 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_BASIC 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_IMPORT 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_EXPORT 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_GENERATE 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_PUBLIC_KEY 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_DERIVE 1


/* Config added by config_adjust_legacy_from_psa.h:

Due to non accelerated MBEDTLS_PSA_ACCEL_ALG_DETERMINISTIC_ECDSA :
#define MBEDTLS_PSA_ECC_ACCEL_INCOMPLETE_ALGS

Due to MBEDTLS_PSA_ECC_ACCEL_INCOMPLETE_ALGS:

#define MBEDTLS_PSA_BUILTIN_ECC_SECP_R1_256 1
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_PSA_BUILTIN_KEY_TYPE_ECC_PUBLIC_KEY 1
#define MBEDTLS_PSA_BUILTIN_KEY_TYPE_ECC_KEY_PAIR_BASIC 1

Special case: we don't support cooked key derivation in drivers yet :
#undef MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_DERIVE

Due to previous undef:
#define MBEDTLS_PSA_ECC_ACCEL_INCOMPLETE_KEY_TYPES
#define MBEDTLS_PSA_BUILTIN_KEY_TYPE_ECC_KEY_PAIR_DERIVE 1
#define MBEDTLS_ECP_LIGHT
#define MBEDTLS_BIGNUM_C

Due to non accelerated MBEDTLS_PSA_BUILTIN_ALG_HMAC

#define MBEDTLS_PSA_BUILTIN_ALG_HMAC 1

*/

#ifdef CONFIG_NXP_FACTORY_DAC_BLOB_GENERATION
#define MBEDTLS_NIST_KW_C
#define MBEDTLS_CMAC_C
#define MBEDTLS_ECDH_C
#endif

#endif /* RW61X_MATTER_MBEDTLS_CONFIG_H */