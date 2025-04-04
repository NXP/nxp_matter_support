/*
 * Copyright 2023,2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ELSFactoryData.h"

void write_uint32_msb_first(uint8_t * pos, uint32_t data)
{
    pos[0] = ((data) >> 24) & 0xFF;
    pos[1] = ((data) >> 16) & 0xFF;
    pos[2] = ((data) >> 8) & 0xFF;
    pos[3] = ((data) >> 0) & 0xFF;
}

void printf_buffer(const char * name, const unsigned char * buffer, size_t size)
{
#define PP_BYTES_PER_LINE (32)
    char line_buffer[PP_BYTES_PER_LINE * 2 + 2];
    const unsigned char * pos = buffer;
    size_t remaining          = size;
    while (remaining > 0)
    {
        size_t block_size = remaining > PP_BYTES_PER_LINE ? PP_BYTES_PER_LINE : remaining;
        uint32_t len      = 0;
        for (size_t i = 0; i < block_size; i++)
        {
            line_buffer[len++] = nibble_to_char[((*pos) & 0xf0) >> 4];
            line_buffer[len++] = nibble_to_char[(*pos++) & 0x0f];
        }
        line_buffer[len++] = '\n';
        line_buffer[len++] = '\0';
        PRINTF("%s (%p): %s", name, pos, line_buffer);
        remaining -= block_size;
    }
}

uint32_t get_required_keyslots(mcuxClEls_KeyProp_t prop)
{
    return prop.bits.ksize == MCUXCLELS_KEYPROPERTY_KEY_SIZE_128 ? 1U : 2U;
}

bool els_is_active_keyslot(mcuxClEls_KeyIndex_t keyIdx)
{
    mcuxClEls_KeyProp_t key_properties;
    key_properties.word.value = ((const volatile uint32_t *) (&ELS->ELS_KS0))[keyIdx];
    return key_properties.bits.kactv;
}

status_t els_enable()
{
    PLOG_INFO("Enabling ELS...");
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_Enable_Async());

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_Enable_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_Enable_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

status_t els_get_key_properties(mcuxClEls_KeyIndex_t key_index, mcuxClEls_KeyProp_t * key_properties)
{
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_GetKeyProperties(key_index, key_properties));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_GetKeyProperties) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_GetKeyProperties failed: 0x%08lx", result);
        return STATUS_ERROR_GENERIC;
    }

    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

mcuxClEls_KeyIndex_t els_get_free_keyslot(uint32_t required_keyslots)
{
    for (mcuxClEls_KeyIndex_t keyIdx = 0U; keyIdx <= (MCUXCLELS_KEY_SLOTS - required_keyslots); keyIdx++)
    {
        bool is_valid_keyslot = true;
        for (uint32_t i = 0U; i < required_keyslots; i++)
        {
            if (els_is_active_keyslot(keyIdx + i))
            {
                is_valid_keyslot = false;
                break;
            }
        }

        if (is_valid_keyslot)
        {
            return keyIdx;
        }
    }
    return MCUXCLELS_KEY_SLOTS;
}

status_t els_derive_key(mcuxClEls_KeyIndex_t src_key_index, mcuxClEls_KeyProp_t key_prop, const uint8_t * dd,
                        mcuxClEls_KeyIndex_t * dst_key_index)
{
    uint32_t required_keyslots = get_required_keyslots(key_prop);

    *dst_key_index = els_get_free_keyslot(required_keyslots);

    if (!(*dst_key_index < MCUXCLELS_KEY_SLOTS))
    {
        PLOG_ERROR("no free keyslot found");
        return STATUS_ERROR_GENERIC;
    }

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_Ckdf_Sp800108_Async(src_key_index, *dst_key_index, key_prop, dd));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_Ckdf_Sp800108_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_Ckdf_Sp800108_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

status_t els_delete_key(mcuxClEls_KeyIndex_t key_index)
{
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_KeyDelete_Async(key_index));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_KeyDelete_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_KeyDelete_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

status_t els_import_key(const uint8_t * wrapped_key, size_t wrapped_key_size, mcuxClEls_KeyProp_t key_prop,
                        mcuxClEls_KeyIndex_t unwrap_key_index, mcuxClEls_KeyIndex_t * dst_key_index)
{
    uint32_t required_keyslots = get_required_keyslots(key_prop);
    *dst_key_index             = els_get_free_keyslot(required_keyslots);

    if (!(*dst_key_index < MCUXCLELS_KEY_SLOTS))
    {
        PLOG_ERROR("no free keyslot found");
        return STATUS_ERROR_GENERIC;
    }

    mcuxClEls_KeyImportOption_t options;
    options.bits.kfmt = MCUXCLELS_KEYIMPORT_KFMT_RFC3394;
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(
        result, token, mcuxClEls_KeyImport_Async(options, wrapped_key, wrapped_key_size, unwrap_key_index, *dst_key_index));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_KeyImport_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_KeyImport_Async failed: 0x%08lx", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08lx", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

status_t els_keygen(mcuxClEls_KeyIndex_t key_index, uint8_t * public_key, size_t * public_key_size)
{
    status_t status = STATUS_SUCCESS;

    mcuxClEls_EccKeyGenOption_t key_gen_options;
    key_gen_options.word.value    = 0u;
    key_gen_options.bits.kgsign   = MCUXCLELS_ECC_PUBLICKEY_SIGN_DISABLE;
    key_gen_options.bits.kgsrc    = MCUXCLELS_ECC_OUTPUTKEY_DETERMINISTIC;
    key_gen_options.bits.skip_pbk = MCUXCLELS_ECC_GEN_PUBLIC_KEY;

    mcuxClEls_KeyProp_t key_properties;
    status = els_get_key_properties(key_index, &key_properties);
    STATUS_SUCCESS_OR_EXIT_MSG("get_key_properties failed: 0x%08x", status);

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(
        result, token,
        mcuxClEls_EccKeyGen_Async(key_gen_options, (mcuxClEls_KeyIndex_t) 0, key_index, key_properties, NULL, &public_key[0]));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_EccKeyGen_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PRINTF("Css_EccKeyGen_Async failed: 0x%08lx\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PRINTF("Css_EccKeyGen_Async WaitForOperation failed: 0x%08lx\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
exit:
    return status;
}

status_t import_die_int_wrapped_key_into_els(const uint8_t * wrapped_key, size_t wrapped_key_size,
                                             mcuxClEls_KeyProp_t key_properties, mcuxClEls_KeyIndex_t * index_output)
{
    status_t status                   = STATUS_SUCCESS;
    mcuxClEls_KeyIndex_t index_unwrap = MCUXCLELS_KEY_SLOTS;

    PLOG_INFO("Deriving wrapping key for import of die_int wrapped key on ELS...");
    status = els_derive_key(DIE_INT_MK_SK_INDEX, wrap_out_key_prop, ckdf_derivation_data_wrap_out, &index_unwrap);
    STATUS_SUCCESS_OR_EXIT_MSG("derive_key failed: 0x%08x", status);

    status = els_import_key(wrapped_key, wrapped_key_size, key_properties, index_unwrap, index_output);
    STATUS_SUCCESS_OR_EXIT_MSG("import_wrapped_key failed: 0x%08x", status);

    status = els_delete_key(index_unwrap);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
    index_unwrap = MCUXCLELS_KEY_SLOTS;

exit:
    if (index_unwrap < MCUXCLELS_KEY_SLOTS)
    {
        (void) els_delete_key(index_unwrap);
    }
    return status;
}

status_t ELS_sign_hash(uint8_t * digest, mcuxClEls_EccByte_t * ecc_signature, mcuxClEls_EccSignOption_t * sign_options,
                       mcuxClEls_KeyIndex_t key_index)
{
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token,
                                     mcuxClEls_EccSign_Async(       // Perform signature generation.
                                         *sign_options,             // Set the prepared configuration.
                                         key_index,                 // Set index of private key in keystore.
                                         digest, NULL, (size_t) 0U, // Pre-hashed data to sign. Note that inputLength parameter is
                                                                    // ignored since pre-hashed data has a fixed length.
                                         ecc_signature // Output buffer, which the operation will write the signature to.
                                         ));
    PLOG_DEBUG_BUFFER("mcuxClEls_EccSign_Async ecc_signature", ecc_signature, MCUXCLELS_ECC_SIGNATURE_SIZE);
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_EccSign_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_EccSign_Async failed. token: 0x%08x, result: 0x%08x", token, result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed. token: 0x%08x, result: 0x%08x", token, result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

status_t ELS_Cipher_Aes_Ecb(mcuxClEls_KeyIndex_t key_index, uint8_t const * input, size_t input_length, uint8_t * output,
                            bool is_encrypt)
{
    mcuxClEls_CipherOption_t cipher_options = {
        0U
    }; // Initialize a new configuration for the planned mcuxClEls_Cipher_Async operation.
    if (is_encrypt)
        cipher_options.bits.dcrpt = MCUXCLELS_CIPHER_ENCRYPT;
    else
        cipher_options.bits.dcrpt = MCUXCLELS_CIPHER_DECRYPT;
    cipher_options.bits.cphmde = MCUXCLELS_CIPHERPARAM_ALGORITHM_AES_ECB;
    cipher_options.bits.cphsie = MCUXCLELS_CIPHER_STATE_IN_DISABLE;
    cipher_options.bits.cphsoe = MCUXCLELS_CIPHER_STATE_OUT_DISABLE;
    cipher_options.bits.extkey = MCUXCLELS_CIPHER_INTERNAL_KEY;

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token,
                                     mcuxClEls_Cipher_Async(cipher_options, key_index, NULL, 0, input, input_length, NULL, output));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_Cipher_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PRINTF("mcuxClEls_Cipher_Async failed: 0x%x\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PRINTF("mcuxClEls_WaitForOperation failed: 0x%x\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    return STATUS_SUCCESS;
}

static status_t els_generate_keypair(mcuxClEls_KeyIndex_t * dst_key_index, uint8_t * public_key, size_t * public_key_size)
{
    if (*public_key_size < 64)
    {
        PLOG_ERROR("insufficient space for public key");
        return STATUS_ERROR_GENERIC;
    }

    mcuxClEls_EccKeyGenOption_t options = { 0 };
    options.bits.kgsrc                  = MCUXCLELS_ECC_OUTPUTKEY_RANDOM;
    options.bits.kgtypedh               = MCUXCLELS_ECC_OUTPUTKEY_KEYEXCHANGE;

    uint32_t keypair_required_keyslots = get_required_keyslots(keypair_prop);
    *dst_key_index                     = (mcuxClEls_KeyIndex_t) els_get_free_keyslot(keypair_required_keyslots);

    if (!(*dst_key_index < MCUXCLELS_KEY_SLOTS))
    {
        PLOG_ERROR("no free keyslot found");
        return STATUS_ERROR_GENERIC;
    }

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(
        result, token,
        mcuxClEls_EccKeyGen_Async(options, (mcuxClEls_KeyIndex_t) 0U, *dst_key_index, keypair_prop, NULL, public_key));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_EccKeyGen_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_EccKeyGen_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    *public_key_size = 64;
    return STATUS_SUCCESS;
}

static status_t els_get_random(unsigned char * out, size_t out_size)
{
    /* Get random IV for sector metadata encryption. */
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClCss_Rng_DrbgRequest_Async(out, out_size));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClCss_Rng_DrbgRequest_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PRINTF("mcuxClCss_Rng_DrbgRequest_Async failed: 0x%08lx\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClCss_WaitForOperation(MCUXCLCSS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PRINTF("Css_EccKeyGen_Async WaitForOperation failed: 0x%08lx\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

static int get_random_mbedtls_callback(void * ctx, unsigned char * out, size_t out_size)
{
    status_t status = els_get_random(out, out_size);
    if (status != STATUS_SUCCESS)
    {
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }
    return 0;
}

static status_t host_perform_key_agreement(const uint8_t * public_key, size_t public_key_size, uint8_t * shared_secret,
                                           size_t * shared_secret_size)
{
    assert(public_key != NULL);
    assert(public_key_size == 64);
    assert(shared_secret != NULL);
    assert(*shared_secret_size >= 32);

    status_t status = STATUS_SUCCESS;

    int ret = 0;
    mbedtls_ecp_group grp;
    mbedtls_ecp_point qB;
    mbedtls_mpi dA, zA;
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&qB);
    mbedtls_mpi_init(&dA);
    mbedtls_mpi_init(&zA);

    unsigned char strbuf[128] = { 0 };
    size_t strlen             = sizeof(strbuf);

    uint8_t public_key_compressed[65] = { 0 };
    public_key_compressed[0]          = 0x04;

    *shared_secret_size = 32;
    ret                 = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_ecp_group_load failed: 0x%08x", ret);

    ret = mbedtls_mpi_read_binary(&dA, import_die_int_ecdh_sk, sizeof(import_die_int_ecdh_sk));
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_mpi_read_binary failed: 0x%08x", ret);

    memcpy(&public_key_compressed[1], public_key, public_key_size);

    ret = mbedtls_ecp_point_read_binary(&grp, &qB, public_key_compressed, sizeof(public_key_compressed));
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_ecp_point_read_binary failed: 0x%08x", ret);

    ret = mbedtls_ecdh_compute_shared(&grp, &zA, &qB, &dA, &get_random_mbedtls_callback, NULL);
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_ecdh_compute_shared failed: 0x%08x", ret);

    mbedtls_ecp_point_write_binary(&grp, &qB, MBEDTLS_ECP_PF_UNCOMPRESSED, &strlen, &strbuf[0], sizeof(strbuf));
    printf_buffer("public_key", strbuf, strlen);

    mbedtls_mpi_write_binary(&zA, shared_secret, *shared_secret_size);
    PLOG_DEBUG_BUFFER("shared_secret", shared_secret, *shared_secret_size);
exit:
    return status;
}

static status_t host_derive_key(const uint8_t * input_key, size_t input_key_size, const uint8_t * derivation_data,
                                size_t derivation_data_size, uint32_t key_properties, uint8_t * output, size_t * output_size)
{
    status_t status = STATUS_SUCCESS;

    int ret          = 0;
    uint32_t counter = 1;
    mbedtls_cipher_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    bool ctx_valid = false;

    assert(input_key != NULL);
    assert(input_key_size == 32);
    assert(derivation_data != NULL);
    assert(derivation_data_size == 12);
    assert(output != NULL);
    assert(*output_size == 32);

    uint32_t lsbit         = key_properties & 0x01;
    uint32_t length_blocks = 1 + lsbit;
    uint32_t length_bytes  = length_blocks * AES_BLOCK_SIZE;
    assert(*output_size >= length_bytes);
    *output_size = length_bytes;

    // KDF in counter mode implementation as described in Section 5.1
    // of NIST SP 800-108, Recommendation for Key Derivation Using Pseudorandom Functions
    //  Derivation data[191:0](sic!) = software_derivation_data[95:0] || 64'h0 || requested_
    //  properties[31:0 || length[31:0] || counter[31:0]

    uint8_t dd[32] = { 0 };
    memcpy(&dd[0], derivation_data, derivation_data_size);
    memset(&dd[12], 0, 8);
    write_uint32_msb_first(&dd[20], key_properties);
    write_uint32_msb_first(&dd[24], length_bytes * 8); // expected in bits!
    write_uint32_msb_first(&dd[28], counter);

    mbedtls_cipher_type_t mbedtls_cipher_type = MBEDTLS_CIPHER_AES_256_ECB;
    const mbedtls_cipher_info_t * cipher_info = mbedtls_cipher_info_from_type(mbedtls_cipher_type);

    PLOG_DEBUG_BUFFER("input_key", input_key, input_key_size);
    PLOG_DEBUG_BUFFER("dd", dd, sizeof(dd));

    uint8_t * pos = output;
    do
    {
        mbedtls_cipher_init(&ctx);
        ctx_valid = true;

        ret = mbedtls_cipher_setup(&ctx, cipher_info);
        RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_cipher_setup failed: 0x%08x", ret);

        ret = mbedtls_cipher_cmac_starts(&ctx, input_key, input_key_size * 8);
        RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_cipher_cmac_starts failed: 0x%08x", ret);

        ret = mbedtls_cipher_cmac_update(&ctx, dd, sizeof(dd));
        RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_cipher_cmac_update failed: 0x%08x", ret);

        ret = mbedtls_cipher_cmac_finish(&ctx, pos);
        RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_cipher_cmac_finish failed: 0x%08x", ret);

        mbedtls_cipher_free(&ctx);
        ctx_valid = false;

        write_uint32_msb_first(&dd[28], ++counter);
        pos += AES_BLOCK_SIZE;
    } while (counter * AES_BLOCK_SIZE <= length_bytes);

    PLOG_DEBUG_BUFFER("output", output, length_bytes);

exit:
    if (ctx_valid)
    {
        mbedtls_cipher_free(&ctx);
        ctx_valid = false;
    }

    return status;
}

static status_t host_wrap_key(const uint8_t * data, size_t data_size, const uint8_t * key, size_t key_size, uint8_t * output,
                              size_t * output_size)
{
    status_t status = STATUS_SUCCESS;
    int ret         = 0;
    mbedtls_nist_kw_context ctx;
    mbedtls_nist_kw_init(&ctx);
    ret = mbedtls_nist_kw_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, key_size * 8, true);
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_nist_kw_setkey failed: 0x%08x", ret);
    ret = mbedtls_nist_kw_wrap(&ctx, MBEDTLS_KW_MODE_KW, data, data_size, output, output_size, *output_size);
    RET_MBEDTLS_SUCCESS_OR_EXIT_MSG("mbedtls_nist_kw_wrap failed: 0x%08x", ret);
    PLOG_DEBUG_BUFFER("wrapped buffer", output, *output_size);
exit:
    mbedtls_nist_kw_free(&ctx);
    return status;
}

static status_t create_els_import_keyblob(const uint8_t * plain_key, size_t plain_key_size, mcuxClEls_KeyProp_t plain_key_prop,
                                          const uint8_t * key_wrap_in, size_t key_wrap_in_size, uint8_t * blob, size_t * blob_size)
{
    assert(plain_key_size == 16 || plain_key_size == 32);
    assert(key_wrap_in_size == 16);

    uint8_t buffer[ELS_BLOB_METADATA_SIZE + MAX_ELS_KEY_SIZE] = { 0 };
    size_t buffer_size                                        = ELS_BLOB_METADATA_SIZE + plain_key_size;

    // Enforce the wrpok bit - the key needs to be re-wrappable!
    plain_key_prop.bits.wrpok = MCUXCLELS_KEYPROPERTY_WRAP_TRUE;

    // This is what ELS documentation says. It does not work though??
    // memset(&buffer[0], 0xA6, 8);
    // write_uint32_msb_first(&buffer[8], plain_key_prop.word.value);
    // memset(&buffer[12], 0, 4);
    // memcpy(&buffer[16], plain_key, plain_key_size);

    write_uint32_msb_first(&buffer[0], plain_key_prop.word.value);
    memset(&buffer[4], 0, 4);
    memcpy(&buffer[8], plain_key, plain_key_size);
    PLOG_DEBUG_BUFFER("plain buffer before wrapping for import", buffer, buffer_size);

    status_t status = host_wrap_key(buffer, buffer_size, key_wrap_in, key_wrap_in_size, blob, blob_size);
    return status;
}

static status_t els_perform_key_agreement(mcuxClEls_KeyIndex_t keypair_index, mcuxClEls_KeyProp_t shared_secret_prop,
                                          mcuxClEls_KeyIndex_t * dst_key_index, const uint8_t * public_key, size_t public_key_size)
{
    uint32_t shared_secret_required_keyslots = get_required_keyslots(shared_secret_prop);
    *dst_key_index                           = els_get_free_keyslot(shared_secret_required_keyslots);

    if (!(*dst_key_index < MCUXCLELS_KEY_SLOTS))
    {
        PLOG_ERROR("no free keyslot found");
        return STATUS_ERROR_GENERIC;
    }

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token,
                                     mcuxClEls_EccKeyExchange_Async(keypair_index, public_key, *dst_key_index, shared_secret_prop));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_EccKeyExchange_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_EccKeyExchange_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    return STATUS_SUCCESS;
}

static inline uint32_t els_get_key_size(mcuxClEls_KeyIndex_t keyIdx)
{
    mcuxClEls_KeyProp_t key_properties;
    key_properties.word.value = ((const volatile uint32_t *) (&ELS->ELS_KS0))[keyIdx];
    return (key_properties.bits.ksize == MCUXCLELS_KEYPROPERTY_KEY_SIZE_256) ? (256U / 8U) : (128U / 8U);
}

static status_t els_export_key(mcuxClEls_KeyIndex_t src_key_index, mcuxClEls_KeyIndex_t wrap_key_index, uint8_t * els_key_out_blob,
                               size_t * els_key_out_blob_size)

{
    uint32_t key_size           = els_get_key_size(src_key_index);
    uint32_t required_blob_size = ELS_BLOB_METADATA_SIZE + key_size + ELS_WRAP_OVERHEAD;
    assert(required_blob_size <= *els_key_out_blob_size);

    *els_key_out_blob_size = required_blob_size;

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_KeyExport_Async(wrap_key_index, src_key_index, els_key_out_blob));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_KeyExport_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_KeyExport_Async failed: 0x%08lx", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08lx", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

status_t export_key_from_els(mcuxClEls_KeyIndex_t key_index, uint8_t * output, size_t * output_size)
{
    assert(output != NULL);
    status_t status = STATUS_SUCCESS;

    mcuxClEls_KeyIndex_t key_wrap_out_index = MCUXCLELS_KEY_SLOTS;
    PLOG_INFO("Deriving wrapping key for export on ELS...");
    status = els_derive_key(DIE_INT_MK_SK_INDEX, wrap_out_key_prop, ckdf_derivation_data_wrap_out, &key_wrap_out_index);
    STATUS_SUCCESS_OR_EXIT_MSG("derive_key failed: 0x%08x", status);

    PLOG_INFO("Exporting/wrapping key...");
    status = els_export_key(key_index, key_wrap_out_index, output, output_size);
    STATUS_SUCCESS_OR_EXIT_MSG("export_key failed: 0x%08x", status);

    status = els_delete_key(key_wrap_out_index);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
    key_wrap_out_index = MCUXCLELS_KEY_SLOTS;
exit:
    return status;
}

status_t import_plain_key_into_els(const uint8_t * plain_key, size_t plain_key_size, mcuxClEls_KeyProp_t key_properties,
                                   mcuxClEls_KeyIndex_t * index_output)
{
    status_t status                                       = STATUS_SUCCESS;
    mcuxClEls_KeyIndex_t index_plain                      = MCUXCLELS_KEY_SLOTS;
    mcuxClEls_KeyIndex_t index_shared_secret              = MCUXCLELS_KEY_SLOTS;
    mcuxClEls_KeyIndex_t index_unwrap                     = MCUXCLELS_KEY_SLOTS;
    mcuxClEls_KeyIndex_t * potentially_used_key_indices[] = { &index_plain, &index_shared_secret, &index_unwrap };

    uint8_t els_key_in_blob[ELS_BLOB_METADATA_SIZE + MAX_ELS_KEY_SIZE + ELS_WRAP_OVERHEAD];
    size_t els_key_in_blob_size = sizeof(els_key_in_blob);

    uint8_t shared_secret[32] = { 0 };
    size_t shared_secret_len  = sizeof(shared_secret);

    uint8_t key_wrap_in[32];
    size_t key_wrap_in_size = sizeof(key_wrap_in);

    PLOG_INFO("Generating random ECC keypair...");
    uint8_t public_key[64] = { 0u };
    size_t public_key_size = sizeof(public_key);
    status                 = els_generate_keypair(&index_plain, &public_key[0], &public_key_size);
    STATUS_SUCCESS_OR_EXIT_MSG("generate_keypair failed: 0x%08x", status);

    PLOG_INFO("Calculating shared secret on host...");
    status = host_perform_key_agreement(public_key, public_key_size, &shared_secret[0], &shared_secret_len);
    STATUS_SUCCESS_OR_EXIT_MSG("perform_key_agreement_host failed: 0x%08x", status);

    PLOG_INFO("Deriving wrapping key for import on host...");
    status = host_derive_key(shared_secret, shared_secret_len, ckdf_derivation_data_wrap_in, sizeof(ckdf_derivation_data_wrap_in),
                             wrap_in_key_prop.word.value, &key_wrap_in[0], &key_wrap_in_size);
    STATUS_SUCCESS_OR_EXIT_MSG("ckdf_host failed: 0x%08x", status);

    PLOG_INFO("Creating ELS keyblob for import...");

    status = create_els_import_keyblob(plain_key, plain_key_size, key_properties, key_wrap_in, key_wrap_in_size,
                                       &els_key_in_blob[0], &els_key_in_blob_size);
    STATUS_SUCCESS_OR_EXIT_MSG("create_els_import_keyblob failed: 0x%08x", status);

    PLOG_INFO("Calculating shared secret on ELS...");
    status = els_perform_key_agreement(index_plain, shared_secret_prop, &index_shared_secret, import_die_int_ecdh_pk,
                                       sizeof(import_die_int_ecdh_pk));
    STATUS_SUCCESS_OR_EXIT_MSG("perform_key_agreement failed: 0x%08x", status);

    status = els_delete_key(index_plain);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
    index_plain = MCUXCLELS_KEY_SLOTS;

    PLOG_INFO("Deriving wrapping key for import on ELS...");
    status = els_derive_key(index_shared_secret, wrap_in_key_prop, ckdf_derivation_data_wrap_in, &index_unwrap);
    STATUS_SUCCESS_OR_EXIT_MSG("derive_key failed: 0x%08x", status);

    status = els_delete_key(index_shared_secret);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
    index_shared_secret = MCUXCLELS_KEY_SLOTS;

    PLOG_INFO("Importing wrapped key...");
    status = els_import_key(els_key_in_blob, els_key_in_blob_size, key_properties, index_unwrap, index_output);
    STATUS_SUCCESS_OR_EXIT_MSG("import_wrapped_key failed: 0x%08x", status);

    status = els_delete_key(index_unwrap);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
    index_unwrap = MCUXCLELS_KEY_SLOTS;

exit:
    for (size_t i = 0; i < ARRAY_SIZE(potentially_used_key_indices); i++)
    {
        mcuxClEls_KeyIndex_t key_index = *(potentially_used_key_indices[i]);
        if (key_index < MCUXCLELS_KEY_SLOTS)
        {
            (void) els_delete_key(key_index);
        }
    }
    return status;
}

static uint32_t get_uint32_val(const uint8_t * input)
{
    uint32_t output = 0U;
    output          = *(input);
    output <<= 8;
    output |= *(input + 1);
    output <<= 8;
    output |= *(input + 2);
    output <<= 8;
    output |= *(input + 3);
    return output;
}

static uint16_t get_uint16_val(const uint8_t * input)
{
    uint16_t output = 0U;
    output          = *input;
    output <<= 8;
    output |= *(input + 1);
    return output;
}

static status_t get_len(const unsigned char ** p, const unsigned char * end, size_t * len)
{
    status_t status = STATUS_SUCCESS;

    ASSERT_OR_EXIT_MSG((end - *p) >= 1, "Issue in length encoding");

    if ((**p & 0x80) == 0)
        *len = *(*p)++;
    else
    {
        switch (**p & 0x7F)
        {
        case 1:
            ASSERT_OR_EXIT_MSG((end - *p) >= 2, "Issue in length encoding");
            *len = (*p)[1];
            (*p) += 2;
            break;

        case 2:
            ASSERT_OR_EXIT_MSG((end - *p) >= 3, "Issue in length encoding");

            *len = ((size_t) (*p)[1] << 8) | (*p)[2];
            (*p) += 3;
            break;

        case 3:
            ASSERT_OR_EXIT_MSG((end - *p) >= 4, "Issue in length encoding");

            *len = ((size_t) (*p)[1] << 16) | ((size_t) (*p)[2] << 8) | (*p)[3];
            (*p) += 4;
            break;

        case 4:
            ASSERT_OR_EXIT_MSG((end - *p) >= 5, "Issue in length encoding");

            *len = ((size_t) (*p)[1] << 24) | ((size_t) (*p)[2] << 16) | ((size_t) (*p)[3] << 8) | (*p)[4];
            (*p) += 5;
            break;

        default:
            status = STATUS_ERROR_GENERIC;
            goto exit;
        }
    }

    ASSERT_OR_EXIT_MSG(*len <= (size_t) (end - *p), "Issue in calculated length");
exit:
    return status;
}

static status_t get_tag(const unsigned char ** p, const unsigned char * end, size_t * len, int tag)
{
    status_t status = STATUS_SUCCESS;

    ASSERT_OR_EXIT_MSG((end - *p) >= 1, "Issue in ltag encoding");
    ASSERT_OR_EXIT_MSG(**p == tag, "Issue in tag encoding");

    (*p)++;

    status = get_len(p, end, len);
exit:
    return status;
}

static size_t get_len_custom(const unsigned char * p)
{
    size_t len = 0U;
    size_t i   = 0U;

    while (i <= MAX_TLV_BYTE_LENGTH)
    {
        if (p + i == NULL)
            return 0;
        i++;
    }

    if ((*p & 0x80) == 0U)
        len = ((size_t) *p + 1);
    else
    {
        switch (*p & 0x7F)
        {
        case 1:
            len = p[1];
            len += 2;
            break;
        case 2:
            len = ((size_t) p[1] << 8) | p[2];
            len += 3;
            break;
        case 3:
            len = ((size_t) p[1] << 16) | ((size_t) p[2] << 8) | p[3];
            len += 4;
            break;
        case 4:
            len = ((size_t) p[1] << 24) | ((size_t) p[2] << 16) | ((size_t) p[3] << 8) | p[4];
            len += 5;
            break;
        default:
            len = 0;
        }
    }

    return len;
}

static bool is_blob_magic(const uint8_t * ptr, const uint8_t * end)
{
    if (ptr + MAGIC_TLV_SIZE >= end)
        return false;

    // The magic TLV is 104 bits long, sufficient to not randomly appear inside the blob (for practical purposes)
    return get_uint32_val(ptr) == MAGIC_TLV_1 && get_uint32_val(ptr + 4) == MAGIC_TLV_2 && get_uint32_val(ptr + 8) == MAGIC_TLV_3 &&
        *(ptr + 12) == MAGIC_TLV_4;
}

static size_t calc_blob_size(uint8_t * blob_ptr)
{
    size_t blob_size = 0U;
    size_t tlv_size  = 0U;

    uint8_t tag = 0U;

    while (1)
    {
        tag = *blob_ptr;
        switch (tag)
        {
        case TAG_MAGIC:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_KEY_ID:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_PSA_PERMITTED_ALGORITHM:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_PSA_USAGE:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_PSA_KEY_TYPE:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_PSA_KEY_BITS:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_PSA_KEY_LIFETIME:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_PSA_DEVICE_LIFECYLE:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_WRAPPING_KEY_ID:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_WRAPPING_ALGORITHM:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_IV:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_SIGNATURE_KEY_ID:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_SIGNATURE_ALGORITHM:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_PLAIN:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_ptr += tlv_size;
            break;
        case TAG_SIGNATURE:
            tlv_size = get_len_custom(blob_ptr + 1) + 1;
            blob_size += tlv_size;
            return blob_size;
        default:
            return 0U;
        }
        blob_size += tlv_size;
    }
}

static status_t parse_blob(const uint8_t * blob, size_t blob_size, blob_context_t * blob_ctx)
{
    status_t status = STATUS_SUCCESS;

    uint8_t tag     = 0U; // the tag of the current TLV
    size_t blob_len = 0U; // the length of the current TLV

    const uint8_t * blob_ptr = NULL;
    const uint8_t * end      = NULL;

    ASSERT_OR_EXIT_MSG(blob != NULL, "blob address is NULL");
    ASSERT_OR_EXIT_MSG(blob_ctx != NULL, "blob_ctx address is NULL");

    memset(blob_ctx, 0, sizeof(blob_context_t));

    blob_ptr = blob;
    end      = blob_ptr + blob_size;

    while ((blob_ptr + 1) < end)
    {
        tag    = *blob_ptr;
        status = get_tag(&blob_ptr, end, &blob_len, tag);
        STATUS_SUCCESS_OR_EXIT_MSG("Issue in getting the tag: 0x%08x", status);

        switch (tag)
        {
        case TAG_MAGIC:
            blob_ctx->magic      = blob_ptr;
            blob_ctx->magic_size = blob_len;
            break;
        case TAG_KEY_ID:
            blob_ctx->key_id = get_uint32_val(blob_ptr);
            break;
        case TAG_PSA_PERMITTED_ALGORITHM:
            blob_ctx->psa_permitted_algorithm = get_uint32_val(blob_ptr);
            break;
        case TAG_PSA_USAGE:
            blob_ctx->psa_usage = get_uint32_val(blob_ptr);
            break;
        case TAG_PSA_KEY_TYPE:
            blob_ctx->psa_key_type = get_uint16_val(blob_ptr);
            break;
        case TAG_PSA_KEY_BITS:
            blob_ctx->psa_key_bits = get_uint32_val(blob_ptr);
            break;
        case TAG_PSA_KEY_LIFETIME:
            blob_ctx->psa_key_lifetime = get_uint32_val(blob_ptr);
            break;
        case TAG_PSA_DEVICE_LIFECYLE:
            blob_ctx->psa_device_lifecycle = get_uint32_val(blob_ptr);
            break;
        case TAG_WRAPPING_KEY_ID:
            blob_ctx->wrapping_key_id = get_uint32_val(blob_ptr);
            break;
        case TAG_WRAPPING_ALGORITHM:
            blob_ctx->wrapping_algorithm = get_uint32_val(blob_ptr);
            break;
        case TAG_IV:
            blob_ctx->iv = blob_ptr;
            break;
        case TAG_SIGNATURE_KEY_ID:
            blob_ctx->signature_key_id = get_uint32_val(blob_ptr);
            break;
        case TAG_SIGNATURE_ALGORITHM:
            blob_ctx->signature_algorithm = get_uint32_val(blob_ptr);
            break;
        case TAG_PLAIN:
            blob_ctx->plain_blob      = blob_ptr;
            blob_ctx->plain_blob_size = blob_len;
            break;
        case TAG_SIGNATURE:
            blob_ctx->signature      = blob_ptr;
            blob_ctx->signature_size = blob_len;
            break;
        default:
            break;
        }
        blob_ptr += blob_len;
    }
exit:
    return status;
}

status_t read_el2go_blob(const uint8_t * blob_area, size_t blob_area_size, size_t object_id, uint8_t * blob, size_t blob_size,
                         size_t * blob_length)
{
    status_t status         = STATUS_SUCCESS;
    uint8_t * blob_start    = NULL;
    bool object_id_match    = false;
    blob_context_t blob_ctx = { 0U };

    ASSERT_OR_EXIT_MSG(blob_area != NULL, "blob_area address is NULL");
    ASSERT_OR_EXIT_MSG(blob != NULL, "blob address is NULL");
    ASSERT_OR_EXIT_MSG(blob_length != NULL, "blob_length address is NULL");

    const uint8_t * blob_area_end = blob_area + blob_area_size;

    // the blob area should start with an EL2GO blob
    ASSERT_OR_EXIT_MSG(is_blob_magic(blob_area, blob_area_end), "No blob present in the blob area");

    uint8_t * blob_ptr = (uint8_t *) blob_area;

    do
    {
        blob_start = blob_ptr;
        do
        {
            blob_ptr++;
        } while (!is_blob_magic(blob_ptr, blob_area_end) && blob_ptr < blob_area_end);

        // This will be longer than the actual blob size for the last blob (handled by the blob parser)
        *blob_length = calc_blob_size(blob_start);
        status       = parse_blob(blob_start, *blob_length, &blob_ctx);
        STATUS_SUCCESS_OR_EXIT_MSG("Issue in parsing the blob: 0x%08x", status);

        if (object_id == blob_ctx.key_id)
        {
            object_id_match = true;
            break;
        }
    } while (blob_ptr < blob_area_end);

    ASSERT_OR_EXIT_MSG(object_id_match == true, "Object id not found");
    ASSERT_OR_EXIT_MSG(*blob_length <= blob_size, "Blob size exceeds the maximum allowed size");
    memset(blob, 0, blob_size);
    memcpy(blob, blob_start, *blob_length);
exit:
    return status;
}

static status_t cmac_verify(const uint8_t * data, size_t data_size, mcuxClEls_KeyIndex_t key_index, uint8_t * pCmac)
{
    mcuxClEls_CmacOption_t options;
    options.bits.initialize = MCUXCLELS_CMAC_INITIALIZE_ENABLE;
    options.bits.finalize   = MCUXCLELS_CMAC_FINALIZE_ENABLE;
    options.bits.extkey     = MCUXCLELS_CMAC_EXTERNAL_KEY_DISABLE;

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_Cmac_Async(options, key_index, NULL, 0, data, data_size, pCmac));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_Cmac_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_Cmac_Async failed: 0x%x\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_Cmac_Async LimitedWaitForOperation failed: 0x%x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    return STATUS_SUCCESS;
}

static status_t pad_iso7816d4(uint8_t * data, size_t unpadded_length, size_t blocksize, size_t * padded_length)
{
    status_t status = STATUS_SUCCESS;
    ASSERT_OR_EXIT_MSG(data != NULL, "data is NULL");
    ASSERT_OR_EXIT_MSG(padded_length != NULL, "padded_length is NULL");
    *padded_length        = ((unpadded_length + blocksize) / blocksize) * blocksize;
    data[unpadded_length] = 0x80U;
    memset(&data[unpadded_length + 1], 0, *padded_length - (unpadded_length + 1));

exit:
    return status;
}

static status_t verify_blob(const uint8_t * blob, size_t blob_size)
{
    status_t status                         = STATUS_SUCCESS;
    size_t blob_to_be_signed_length         = 0U;
    uint8_t cmac[CMAC_BLOCK_SIZE]           = { 0U };
    uint8_t blob_signature[CMAC_BLOCK_SIZE] = { 0U };
    uint8_t * blob_to_be_signed             = NULL;

    ASSERT_OR_EXIT_MSG(blob != NULL, "blob is NULL");

    memcpy(blob_signature, (blob + blob_size - CMAC_BLOCK_SIZE), CMAC_BLOCK_SIZE);

    blob_to_be_signed = calloc(1, blob_size);
    blob_size         = blob_size - CMAC_BLOCK_SIZE;

    memcpy(blob_to_be_signed, blob, blob_size);

    status = pad_iso7816d4(blob_to_be_signed, blob_size, CMAC_BLOCK_SIZE, &blob_to_be_signed_length);
    STATUS_SUCCESS_OR_EXIT_MSG("pad_iso7816d4 failed: 0x%08x", status);

    // Attention, CSS expects size before padding
    status = cmac_verify(blob_to_be_signed, blob_size, EL2GOIMPORT_AUTH_SK_ID, cmac);
    STATUS_SUCCESS_OR_EXIT_MSG("pad_iso7816d4 failed: 0x%08x", status);

    for (size_t i = 0; i < CMAC_BLOCK_SIZE; i++)
    {
        if (blob_signature[i] != cmac[i])
        {
            status = STATUS_ERROR_GENERIC;
            PLOG_ERROR("#### Blob signature does not match");
            goto exit;
        }
    }
exit:
    free(blob_to_be_signed);
    return status;
}

static status_t derive_aes_key(mcuxClEls_KeyIndex_t parent_key_idx, mcuxClEls_KeyIndex_t key_idx, uint8_t * derivation_data,
                               mcuxClEls_KeyProp_t key_properties)
{
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token,
                                     mcuxClEls_Ckdf_Sp800108_Async(parent_key_idx, key_idx, key_properties, derivation_data));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_Ckdf_Sp800108_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_Ckdf_Sp800108_Async failed: 0x%08x\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

static status_t derive_el2go_die_keys()
{
    status_t status                                = STATUS_SUCCESS;
    mcuxClEls_KeyIndex_t el2gooem_mk_sk_idx        = 4U;
    mcuxClEls_KeyIndex_t el2goimport_auth_sk_idx   = 16U;
    mcuxClEls_KeyIndex_t el2goimporttfm_kek_sk_idx = 18U;
    PLOG_INFO("#### derive_el2go_die_keys");

    mcuxClEls_KeyProp_t el2goimport_kek_sk_prop = { 0 };

    el2goimport_kek_sk_prop.bits.upprot_priv = MCUXCLELS_KEYPROPERTY_PRIVILEGED_TRUE;
    el2goimport_kek_sk_prop.bits.upprot_sec  = MCUXCLELS_KEYPROPERTY_SECURE_TRUE;
    el2goimport_kek_sk_prop.bits.kactv       = MCUXCLELS_KEYPROPERTY_ACTIVE_TRUE;
    el2goimport_kek_sk_prop.bits.ksize       = MCUXCLELS_KEYPROPERTY_KEY_SIZE_256;
    el2goimport_kek_sk_prop.bits.ukuok       = MCUXCLELS_KEYPROPERTY_KUOK_TRUE;

    uint8_t el2goimport_kek_sk_dd[12] = { 0x00, 0x65, 0x32, 0x67, 0x69, 0x6b, 0x65, 0x6b, 0x5f, 0x73, 0x6b, 0x00 };

    status = derive_aes_key(EL2GOOEM_MK_SK_ID, EL2GOIMPORT_KEK_SK_ID, el2goimport_kek_sk_dd, el2goimport_kek_sk_prop);
    STATUS_SUCCESS_OR_EXIT_MSG("Derivation of el2goimport_kek failed", status);

    mcuxClEls_KeyProp_t el2goimporttfm_kek_sk_prop = { 0 };

    el2goimporttfm_kek_sk_prop.bits.upprot_priv = MCUXCLELS_KEYPROPERTY_PRIVILEGED_TRUE;
    el2goimporttfm_kek_sk_prop.bits.upprot_sec  = MCUXCLELS_KEYPROPERTY_SECURE_TRUE;
    el2goimporttfm_kek_sk_prop.bits.kactv       = MCUXCLELS_KEYPROPERTY_ACTIVE_TRUE;
    el2goimporttfm_kek_sk_prop.bits.ksize       = MCUXCLELS_KEYPROPERTY_KEY_SIZE_256;
    el2goimporttfm_kek_sk_prop.bits.uaes        = MCUXCLELS_KEYPROPERTY_AES_TRUE;

    uint8_t el2goimporttfm_kek_sk_dd[12] = { 0x00, 0x65, 0x32, 0x67, 0x69, 0x74, 0x66, 0x6d, 0x5f, 0x73, 0x6b, 0x00 };

    status = derive_aes_key(EL2GOOEM_MK_SK_ID, EL2GOIMPORTTFM_KEK_SK_ID, el2goimporttfm_kek_sk_dd, el2goimporttfm_kek_sk_prop);
    STATUS_SUCCESS_OR_EXIT_MSG("Derivation of el2goimporttfm_kek failed", status);

    mcuxClEls_KeyProp_t el2goimport_auth_sk_prop = { 0 };

    el2goimport_auth_sk_prop.bits.upprot_priv = MCUXCLELS_KEYPROPERTY_PRIVILEGED_TRUE;
    el2goimport_auth_sk_prop.bits.upprot_sec  = MCUXCLELS_KEYPROPERTY_SECURE_TRUE;
    el2goimport_auth_sk_prop.bits.kactv       = MCUXCLELS_KEYPROPERTY_ACTIVE_TRUE;
    el2goimport_auth_sk_prop.bits.ksize       = MCUXCLELS_KEYPROPERTY_KEY_SIZE_256;
    el2goimport_auth_sk_prop.bits.ucmac       = MCUXCLELS_KEYPROPERTY_CMAC_TRUE;

    uint8_t el2goimport_auth_sk_dd[12] = { 0x00, 0x65, 0x32, 0x67, 0x69, 0x61, 0x75, 0x74, 0x5f, 0x73, 0x6b, 0x00 };

    status = derive_aes_key(EL2GOOEM_MK_SK_ID, EL2GOIMPORT_AUTH_SK_ID, el2goimport_auth_sk_dd, el2goimport_auth_sk_prop);
    STATUS_SUCCESS_OR_EXIT_MSG("Derivation of el2goimport_auth failed", status);
exit:
    return status;
}

static status_t delete_el2go_die_keys()
{
    status_t status = STATUS_SUCCESS;

    status = els_delete_key(EL2GOIMPORT_KEK_SK_ID);
    STATUS_SUCCESS_OR_EXIT_MSG("Deletion of el2goimport_kek failed", status);

    status = els_delete_key(EL2GOIMPORTTFM_KEK_SK_ID);
    STATUS_SUCCESS_OR_EXIT_MSG("Deletion of el2goimporttfm_kek failed", status);

    status = els_delete_key(EL2GOIMPORT_AUTH_SK_ID);
    STATUS_SUCCESS_OR_EXIT_MSG("Deletion of el2goimport_auth failed", status);

exit:
    return status;
}

status_t import_el2go_key_in_els(const uint8_t * blob, size_t blob_size, mcuxClEls_KeyIndex_t * key_index)
{
    status_t status          = STATUS_SUCCESS;
    const uint8_t * els_blob = NULL;
    size_t els_blob_size     = 0U;
    PLOG_INFO("#### import_el2go_blob_in_els");

    ASSERT_OR_EXIT_MSG(blob != NULL, "blob is NULL");
    ASSERT_OR_EXIT_MSG(key_index != NULL, "key_index is NULL");

    status = derive_el2go_die_keys();
    STATUS_SUCCESS_OR_EXIT_MSG("derive_el2go_die_keys failed: 0x%08x", status);

    status = verify_blob(blob, blob_size);
    STATUS_SUCCESS_OR_EXIT_MSG("verify_blob failed: 0x%08x", status);

    blob_context_t blob_ctx = { 0U };
    status                  = parse_blob(blob, blob_size, &blob_ctx);
    STATUS_SUCCESS_OR_EXIT_MSG("parse_blob failed: 0x%08x", status);

    mcuxClEls_KeyProp_t key_prop = { .word = { .value = MCUXCLELS_KEYPROPERTY_VALUE_SECURE |
                                                   MCUXCLELS_KEYPROPERTY_VALUE_PRIVILEGED | MCUXCLELS_KEYPROPERTY_VALUE_ECSGN } };
    status = els_import_key(blob_ctx.plain_blob, blob_ctx.plain_blob_size, key_prop, EL2GOIMPORT_KEK_SK_ID, key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_el2go_die_keys failed: 0x%08x", status);

    status = delete_el2go_die_keys();
    STATUS_SUCCESS_OR_EXIT_MSG("delete_el2go_die_keys failed: 0x%08x", status);

exit:
    return status;
}

static status_t export_public_key_from_cert(const uint8_t * cert, size_t cert_size, uint8_t * public_key, size_t public_key_size,
                                            size_t * public_key_length)
{
    status_t status = STATUS_SUCCESS;
    int mbedtls_status;
    mbedtls_x509_crt client_cert                       = { 0 };
    char outBuf[128]                                   = { '\0' };
    uint8_t temp_buf[MBEDTLS_PK_ECP_PUB_DER_MAX_BYTES] = { 0U };

    mbedtls_x509_crt_init(&client_cert);

    ASSERT_OR_EXIT_MSG(cert != NULL, "cert is NULL");
    ASSERT_OR_EXIT_MSG(public_key != NULL, "public_key is NULL");
    ASSERT_OR_EXIT_MSG(public_key_length != NULL, "public_key_length is NULL");

    mbedtls_status = mbedtls_x509_crt_parse_der(&client_cert, cert, cert_size);
    ASSERT_OR_EXIT_MSG(mbedtls_status == 0, "Error in parsing the client certificate");

    uint8_t * ptr = temp_buf + sizeof(temp_buf);
    int len       = mbedtls_pk_write_pubkey(&ptr, temp_buf, &client_cert.pk);

    ASSERT_OR_EXIT_MSG(len >= 0, "Issue in size of extracted public key");
    *public_key_length = len - 1;
    ASSERT_OR_EXIT_MSG(*public_key_length <= public_key_size, "Issue in size of extracted public key");

    memcpy(public_key, ptr + 1, *public_key_length);
exit:
    mbedtls_x509_crt_free(&client_cert);
    return status;
}

static status_t unpad_iso7816d4(uint8_t * data, size_t * data_size)
{
    status_t status = STATUS_SUCCESS;
    ASSERT_OR_EXIT_MSG(data_size != NULL, "data_size is NULL");
    ASSERT_OR_EXIT_MSG(*data_size > 0u, "data_size lower then 0");

    uint32_t count = *data_size - 1u;
    while (count > 0u && data[count] == 0u)
    {
        count--;
    }

    ASSERT_OR_EXIT_MSG(data[count] == 0x80u, "Pad block corrupted");

    *data_size -= *data_size - count;

exit:
    return status;
}

static status_t decrypt_external_blob(const uint8_t * enc_data, size_t enc_data_size, mcuxClEls_KeyIndex_t tfmKekKeyIdx,
                                      const uint8_t * iv, uint8_t * plain_data, size_t * plain_data_size)
{
    status_t status                         = STATUS_SUCCESS;
    mcuxClEls_CipherOption_t cipher_options = { 0U };
    cipher_options.bits.dcrpt               = MCUXCLELS_CIPHER_DECRYPT;
    cipher_options.bits.cphmde              = MCUXCLELS_CIPHERPARAM_ALGORITHM_AES_CBC;
    cipher_options.bits.cphsie              = MCUXCLELS_CIPHER_STATE_IN_DISABLE;
    cipher_options.bits.cphsoe              = MCUXCLELS_CIPHER_STATE_OUT_DISABLE;
    cipher_options.bits.extkey              = MCUXCLELS_CIPHER_INTERNAL_KEY;

    // We use CSS in a mode where it will not output its state, so casting away
    // the const is safe here.
    uint8_t * state = (uint8_t *) iv;
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(
        result, token,
        mcuxClEls_Cipher_Async(cipher_options, tfmKekKeyIdx, NULL, (size_t) 0u, enc_data, enc_data_size, state, plain_data));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_Cipher_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_Cipher_Async failed: 0x%x\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_Cipher_Async LimitedWaitForOperation failed: 0x%x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    *plain_data_size = enc_data_size;
    status           = unpad_iso7816d4(plain_data, plain_data_size);
    if (status != kStatus_Success)
    {
        PLOG_ERROR("Error,  unpad_iso7816d4 failed\r\n");
        return STATUS_ERROR_GENERIC;
    }

    return status;
}

status_t decrypt_el2go_cert_blob(const uint8_t * blob, size_t blob_size, uint8_t * cert, size_t cert_size, size_t * cert_length)
{
    status_t status = STATUS_SUCCESS;

    PLOG_INFO("#### decrypt_el2go_cert_blob");

    ASSERT_OR_EXIT_MSG(blob != NULL, "blob is NULL");
    ASSERT_OR_EXIT_MSG(cert != NULL, "cert is NULL");
    ASSERT_OR_EXIT_MSG(cert_length != NULL, "cert_length is NULL");

    status = derive_el2go_die_keys();
    STATUS_SUCCESS_OR_EXIT_MSG("derive_el2go_die_keys failed: 0x%08x", status);

    status = verify_blob(blob, blob_size);
    STATUS_SUCCESS_OR_EXIT_MSG("verify_blob failed: 0x%08x", status);

    blob_context_t blob_ctx = { 0U };
    status                  = parse_blob(blob, blob_size, &blob_ctx);
    STATUS_SUCCESS_OR_EXIT_MSG("parse_blob failed: 0x%08x", status);

    *cert_length = cert_size;
    status       = decrypt_external_blob(blob_ctx.plain_blob, blob_ctx.plain_blob_size, EL2GOIMPORTTFM_KEK_SK_ID, blob_ctx.iv, cert,
                                         cert_length);
    STATUS_SUCCESS_OR_EXIT_MSG("decrypt_external_blob failed: 0x%08x", status);

    status = delete_el2go_die_keys();
    STATUS_SUCCESS_OR_EXIT_MSG("delete_el2go_die_keys failed: 0x%08x", status);

exit:
    return status;
}
