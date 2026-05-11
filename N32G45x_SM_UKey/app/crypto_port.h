#ifndef CRYPTO_PORT_H
#define CRYPTO_PORT_H

#include "app_types.h"

#define CRYPTO_SM3_DIGEST_LEN 32u
#define CRYPTO_SM4_KEY_LEN    16u
#define CRYPTO_SM4_BLOCK_LEN  16u
#define CRYPTO_SM2_PRI_LEN    32u
#define CRYPTO_SM2_PUB_LEN    64u
#define CRYPTO_SM2_SIG_LEN    64u

ukey_status_t crypto_sm3(const uint8_t *data, size_t len, uint8_t out[CRYPTO_SM3_DIGEST_LEN]);
ukey_status_t crypto_sm4_encrypt_cbc(const uint8_t key[16], const uint8_t iv[16],
                                     const uint8_t *in, uint8_t *out, size_t len);
ukey_status_t crypto_sm4_decrypt_cbc(const uint8_t key[16], const uint8_t iv[16],
                                     const uint8_t *in, uint8_t *out, size_t len);
ukey_status_t crypto_sm2_generate_keypair(uint8_t pri[CRYPTO_SM2_PRI_LEN],
                                          uint8_t pub[CRYPTO_SM2_PUB_LEN]);
ukey_status_t crypto_sm2_sign(const uint8_t pri[CRYPTO_SM2_PRI_LEN],
                              const uint8_t *msg, size_t msg_len,
                              uint8_t sig[CRYPTO_SM2_SIG_LEN]);

#endif

