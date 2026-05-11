#include "secure_store.h"
#include <string.h>

static const uint8_t k_store_key[16] = {
    0x47, 0x4D, 0x55, 0x4B, 0x2D, 0x4E, 0x33, 0x32,
    0x47, 0x34, 0x35, 0x78, 0x2D, 0x30, 0x31, 0x21
};

static const uint8_t k_store_iv[16] = {
    0x55, 0x53, 0x42, 0x2D, 0x43, 0x44, 0x43, 0x2D,
    0x53, 0x4D, 0x34, 0x2D, 0x49, 0x56, 0x30, 0x31
};

static size_t plain_crypt_len(void)
{
    size_t len = sizeof(secure_store_plain_t);
    return (len + 15u) & ~((size_t)15u);
}

static void checksum_make(secure_store_plain_t *plain)
{
    uint8_t old[CRYPTO_SM3_DIGEST_LEN];
    memcpy(old, plain->checksum, sizeof(old));
    memset(plain->checksum, 0, sizeof(plain->checksum));
    (void)crypto_sm3((const uint8_t *)plain, sizeof(*plain), plain->checksum);
    (void)old;
}

static bool checksum_ok(secure_store_plain_t *plain)
{
    uint8_t stored[CRYPTO_SM3_DIGEST_LEN];
    uint8_t calc[CRYPTO_SM3_DIGEST_LEN];
    memcpy(stored, plain->checksum, sizeof(stored));
    memset(plain->checksum, 0, sizeof(plain->checksum));
    if (crypto_sm3((const uint8_t *)plain, sizeof(*plain), calc) != UKEY_OK) {
        return false;
    }
    memcpy(plain->checksum, stored, sizeof(stored));
    return memcmp(stored, calc, sizeof(calc)) == 0;
}

void secure_store_factory_default(secure_store_plain_t *plain)
{
    memset(plain, 0, sizeof(*plain));
    plain->magic = UKEY_SECURE_MAGIC;
    plain->version = UKEY_SECURE_VERSION;
    plain->pin_retries = UKEY_PIN_RETRY_LIMIT;
}

ukey_status_t secure_store_load(secure_store_plain_t *plain)
{
    uint8_t raw[PLATFORM_SECURE_AREA_SIZE];
    uint8_t dec[PLATFORM_SECURE_AREA_SIZE];
    size_t len = plain_crypt_len();

    if (!plain || len > sizeof(raw)) return UKEY_ERR_ARG;
    if (platform_flash_load_secure_area(raw, len) != UKEY_OK) {
        secure_store_factory_default(plain);
        return UKEY_ERR_STORAGE;
    }
    if (crypto_sm4_decrypt_cbc(k_store_key, k_store_iv, raw, dec, len) != UKEY_OK) {
        secure_store_factory_default(plain);
        return UKEY_ERR_CRYPTO;
    }
    memcpy(plain, dec, sizeof(*plain));
    if (plain->magic != UKEY_SECURE_MAGIC || plain->version != UKEY_SECURE_VERSION || !checksum_ok(plain)) {
        secure_store_factory_default(plain);
        return UKEY_ERR_STORAGE;
    }
    return UKEY_OK;
}

ukey_status_t secure_store_save(const secure_store_plain_t *plain_in)
{
    secure_store_plain_t copy;
    uint8_t raw[PLATFORM_SECURE_AREA_SIZE];
    uint8_t enc[PLATFORM_SECURE_AREA_SIZE];
    size_t len = plain_crypt_len();

    if (!plain_in || len > sizeof(raw)) return UKEY_ERR_ARG;
    memset(raw, 0, sizeof(raw));
    copy = *plain_in;
    checksum_make(&copy);
    memcpy(raw, &copy, sizeof(copy));
    if (crypto_sm4_encrypt_cbc(k_store_key, k_store_iv, raw, enc, len) != UKEY_OK) {
        return UKEY_ERR_CRYPTO;
    }
    return platform_flash_save_secure_area(enc, len);
}

