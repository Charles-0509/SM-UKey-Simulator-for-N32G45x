#ifndef SOFTWARE_SM4_H
#define SOFTWARE_SM4_H

#include <stdint.h>

void software_sm4_crypt_cbc(const uint8_t key[16], const uint8_t iv[16],
                            const uint8_t *in, uint8_t *out,
                            uint32_t len, int encrypt);

#endif

