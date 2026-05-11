#include "crypto_port.h"
#include "n32g45x_hash.h"
#include "software_sm2.h"
#include "software_sm4.h"

ukey_status_t crypto_sm3(const uint8_t *data, size_t len, uint8_t out[CRYPTO_SM3_DIGEST_LEN])
{
    uint8_t empty = 0u;
    if ((!data && len != 0u) || !out || len > 0xFFFFFFFFu) {
        return UKEY_ERR_ARG;
    }
    if (!data) {
        data = &empty;
    }
    return (SM3_Hash((uint8_t *)data, (uint32_t)len, out) == SM3_Hash_OK) ? UKEY_OK : UKEY_ERR_CRYPTO;
}

ukey_status_t crypto_sm4_encrypt_cbc(const uint8_t key[16], const uint8_t iv[16],
                                     const uint8_t *in, uint8_t *out, size_t len)
{
    if (!key || !iv || !in || !out || (len % 16u) != 0u || len > 0xFFFFFFFFu) {
        return UKEY_ERR_ARG;
    }
    software_sm4_crypt_cbc(key, iv, in, out, (uint32_t)len, 1);
    return UKEY_OK;
}

ukey_status_t crypto_sm4_decrypt_cbc(const uint8_t key[16], const uint8_t iv[16],
                                     const uint8_t *in, uint8_t *out, size_t len)
{
    if (!key || !iv || !in || !out || (len % 16u) != 0u || len > 0xFFFFFFFFu) {
        return UKEY_ERR_ARG;
    }
    software_sm4_crypt_cbc(key, iv, in, out, (uint32_t)len, 0);
    return UKEY_OK;
}

ukey_status_t crypto_sm2_generate_keypair(uint8_t pri[CRYPTO_SM2_PRI_LEN],
                                          uint8_t pub[CRYPTO_SM2_PUB_LEN])
{
    return software_sm2_generate_keypair(pri, pub);
}

ukey_status_t crypto_sm2_sign(const uint8_t pri[CRYPTO_SM2_PRI_LEN],
                              const uint8_t *msg, size_t msg_len,
                              uint8_t sig[CRYPTO_SM2_SIG_LEN])
{
    return software_sm2_sign(pri, msg, msg_len, sig);
}
