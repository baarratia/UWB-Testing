/*
 * Copyright (c) 2021 Qorvo US, Inc. All rights reserved.
 *
 * This software is proprietary to Qorvo; you may not use it for any purpose
 * without first obtaining a license from Qorvo.
 *
 * Please contact Qorvo to inquire about licensing terms.
 */

#include "mcps_crypto.h"
#include "nrf_crypto_aes.h"
#include "nrf_crypto_aead.h"
#include "nrf_crypto_init.h"
#include "nrf_crypto_rng.h"
#include "linux/errno.h"
#include "app_error.h"

static int nrf_crypto_error_to_std_error(ret_code_t rc)
{
    switch(rc) {
    case NRF_SUCCESS:
        return 0;
    case NRF_ERROR_CRYPTO_ALLOC_FAILED:
    case NRF_ERROR_CRYPTO_STACK_OVERFLOW:
        return -ENOMEM;
    case NRF_ERROR_CRYPTO_AEAD_INVALID_MAC:
        return -EBADMSG;
    case NRF_ERROR_CRYPTO_BUSY:
        return -EBUSY;
    case NRF_ERROR_CRYPTO_FEATURE_UNAVAILABLE:
        return -EOPNOTSUPP;
    default:
        return -EINVAL;
    }
}

int mcps_crypto_cmac_aes_128_digest(const uint8_t *key, const uint8_t *data,
                                    unsigned int data_len, uint8_t *out)
{
    /* Init cbc_mac context */
    nrf_crypto_aes_context_t cmac_ctx;
    size_t out_len = 128;
    ret_code_t rc = nrf_crypto_aes_init(&cmac_ctx, &g_nrf_crypto_aes_cmac_128_info,
                                        NRF_CRYPTO_MAC_CALCULATE);
    if(rc != NRF_SUCCESS) {
        return nrf_crypto_error_to_std_error(rc);
    }
    /* Set encryption key */
    rc = nrf_crypto_aes_key_set(&cmac_ctx, (uint8_t *)key);
    if(rc != NRF_SUCCESS) {
        return nrf_crypto_error_to_std_error(rc);
    }
    /* Calculate MAC */
    rc = nrf_crypto_aes_finalize(&cmac_ctx, (uint8_t *)data, data_len,
                                 out, &out_len);
    if(rc != NRF_SUCCESS) {
        return nrf_crypto_error_to_std_error(rc);
    }
    /* Free memory */
    rc = nrf_crypto_aes_uninit(&cmac_ctx);
    if(rc != NRF_SUCCESS) {
        return nrf_crypto_error_to_std_error(rc);
    }
    return 0;
}

void *mcps_crypto_aead_aes_ccm_star_128_create(const uint8_t *key)
{
    nrf_crypto_aead_context_t *ccm_ctx;
    ccm_ctx = malloc(sizeof(nrf_crypto_aead_context_t));
    ret_code_t rc = nrf_crypto_aead_init(ccm_ctx,
                                         &g_nrf_crypto_aes_ccm_128_info,
                                         (uint8_t *)key);
    if(rc != NRF_SUCCESS) {
        return NULL;
    }
    return ccm_ctx;
}

void mcps_crypto_aead_aes_ccm_star_128_destroy(void *ctx)
{
    nrf_crypto_aead_context_t *ccm_ctx = ctx;
    ret_code_t rc = nrf_crypto_aead_uninit(ccm_ctx);
    if(rc != NRF_SUCCESS) {
        return;
    }
    free(ccm_ctx);
}

int mcps_crypto_aead_aes_ccm_star_128_encrypt(
    void *ctx, const uint8_t *nonce, const uint8_t *header, unsigned int header_len,
    uint8_t *data, unsigned int data_len, uint8_t *mac, unsigned int mac_len)
{
    nrf_crypto_aead_context_t *ccm_ctx = ctx;
    ret_code_t rc = nrf_crypto_aead_crypt(ccm_ctx, NRF_CRYPTO_ENCRYPT,
                                          (uint8_t *)nonce, MCPS_CRYPTO_AES_CCM_STAR_NONCE_LEN,
                                          (uint8_t *)header, header_len,
                                          data, data_len, data, mac, mac_len);
    if(rc != NRF_SUCCESS) {
         return nrf_crypto_error_to_std_error(rc);
    }
    return 0;
}

int mcps_crypto_aead_aes_ccm_star_128_decrypt(
    void *ctx, const uint8_t *nonce, const uint8_t *header, unsigned int header_len,
    uint8_t *data, unsigned int data_len, uint8_t *mac, unsigned int mac_len)
{
    nrf_crypto_aead_context_t *ccm_ctx = ctx;
    ret_code_t rc = nrf_crypto_aead_crypt(ccm_ctx, NRF_CRYPTO_DECRYPT,
                                          (uint8_t *)nonce, MCPS_CRYPTO_AES_CCM_STAR_NONCE_LEN,
                                          (uint8_t *)header, header_len,
                                          data, data_len, data, mac, mac_len);
    if(rc != NRF_SUCCESS) {
        return nrf_crypto_error_to_std_error(rc);
    }
    return 0;
}

void *mcps_crypto_aes_ecb_128_create(const uint8_t *key)
{
    nrf_crypto_aes_context_t *ecb_ctx;
    ecb_ctx = malloc(sizeof(nrf_crypto_aes_context_t));
    ret_code_t rc = nrf_crypto_aes_init(ecb_ctx,
                                        &g_nrf_crypto_aes_ecb_128_info,
                                        NRF_CRYPTO_ENCRYPT);
    if(rc != NRF_SUCCESS) {
        return NULL;
    }
    rc = nrf_crypto_aes_key_set(ecb_ctx, (uint8_t *)key);
    if(rc != NRF_SUCCESS) {
        return NULL;
    }
    return ecb_ctx;
}

void mcps_crypto_aes_ecb_128_destroy(void *ctx)
{
    nrf_crypto_aes_context_t *ecb_ctx = ctx;
    ret_code_t rc = nrf_crypto_aes_uninit(ecb_ctx);
    if(rc != NRF_SUCCESS) {
        return;
    }
    free(ecb_ctx);
}

int mcps_crypto_aes_ecb_128_encrypt(void *ctx, const uint8_t *data,
                                    unsigned int data_len, uint8_t *out)
{
    nrf_crypto_aes_context_t *ecb_ctx = ctx;
    size_t out_len = data_len;
    ret_code_t rc = nrf_crypto_aes_finalize(ecb_ctx, (uint8_t *)data, data_len,
                                            out, &out_len);
    if(rc != NRF_SUCCESS) {
        return nrf_crypto_error_to_std_error(rc);
    }
    return 0;
}
