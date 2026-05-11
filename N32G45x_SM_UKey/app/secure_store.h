#ifndef SECURE_STORE_H
#define SECURE_STORE_H

#include "app_config.h"
#include "crypto_port.h"
#include "platform_port.h"

typedef struct {
    char name[UKEY_STORE_NAME_LEN];
    uint16_t len;
    uint8_t value[UKEY_STORE_VALUE_LEN];
} secure_store_slot_t;

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t initialized;
    uint8_t pin_retries;
    uint8_t salt[16];
    uint8_t pin_hash[CRYPTO_SM3_DIGEST_LEN];
    uint8_t sm2_pri[CRYPTO_SM2_PRI_LEN];
    uint8_t sm2_pub[CRYPTO_SM2_PUB_LEN];
    secure_store_slot_t slots[UKEY_STORE_SLOT_COUNT];
    uint8_t checksum[CRYPTO_SM3_DIGEST_LEN];
} secure_store_plain_t;

ukey_status_t secure_store_load(secure_store_plain_t *plain);
ukey_status_t secure_store_save(const secure_store_plain_t *plain);
void secure_store_factory_default(secure_store_plain_t *plain);

#endif

