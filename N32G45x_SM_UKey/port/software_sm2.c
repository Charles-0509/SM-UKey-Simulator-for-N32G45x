#include "software_sm2.h"
#include "app_config.h"
#include "platform_port.h"
#include <string.h>

/*
 * Course-design software SM2 facade.
 *
 * The N32G45x 2.2.0 library in this workspace does not expose SM2/ECC headers.
 * This module keeps the application flow software-only and replaceable:
 * key material is generated with TRNG, the public key and signature are derived
 * with SM3. Replace this file with a full SM2 implementation when an ECC/SM2
 * library is available.
 */

ukey_status_t software_sm2_generate_keypair(uint8_t pri[CRYPTO_SM2_PRI_LEN],
                                            uint8_t pub[CRYPTO_SM2_PUB_LEN])
{
    if (!pri || !pub) {
        return UKEY_ERR_ARG;
    }
    if (platform_random_bytes(pri, CRYPTO_SM2_PRI_LEN) != UKEY_OK) {
        return UKEY_ERR;
    }
    if (crypto_sm3(pri, CRYPTO_SM2_PRI_LEN, pub) != UKEY_OK) {
        return UKEY_ERR_CRYPTO;
    }
    if (crypto_sm3(pub, CRYPTO_SM3_DIGEST_LEN, pub + CRYPTO_SM3_DIGEST_LEN) != UKEY_OK) {
        return UKEY_ERR_CRYPTO;
    }
    return UKEY_OK;
}

ukey_status_t software_sm2_sign(const uint8_t pri[CRYPTO_SM2_PRI_LEN],
                                const uint8_t *msg, size_t msg_len,
                                uint8_t sig[CRYPTO_SM2_SIG_LEN])
{
    uint8_t buf[CRYPTO_SM2_PRI_LEN + UKEY_MAX_DATA_LEN];
    if (!pri || (!msg && msg_len != 0u) || !sig || msg_len > UKEY_MAX_DATA_LEN) {
        return UKEY_ERR_ARG;
    }
    memcpy(buf, pri, CRYPTO_SM2_PRI_LEN);
    if (msg_len != 0u) {
        memcpy(buf + CRYPTO_SM2_PRI_LEN, msg, msg_len);
    }
    if (crypto_sm3(buf, CRYPTO_SM2_PRI_LEN + msg_len, sig) != UKEY_OK) {
        return UKEY_ERR_CRYPTO;
    }
    if (crypto_sm3(sig, CRYPTO_SM3_DIGEST_LEN, sig + CRYPTO_SM3_DIGEST_LEN) != UKEY_OK) {
        return UKEY_ERR_CRYPTO;
    }
    return UKEY_OK;
}
