#ifndef SOFTWARE_SM2_H
#define SOFTWARE_SM2_H

#include "crypto_port.h"

ukey_status_t software_sm2_generate_keypair(uint8_t pri[CRYPTO_SM2_PRI_LEN],
                                            uint8_t pub[CRYPTO_SM2_PUB_LEN]);
ukey_status_t software_sm2_sign(const uint8_t pri[CRYPTO_SM2_PRI_LEN],
                                const uint8_t *msg, size_t msg_len,
                                uint8_t sig[CRYPTO_SM2_SIG_LEN]);

#endif

