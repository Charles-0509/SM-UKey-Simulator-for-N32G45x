#include "ukey_state.h"
#include <string.h>

static ukey_context_t g_ctx;

static void make_pin_hash(const char *pin, const uint8_t salt[16], uint8_t out[CRYPTO_SM3_DIGEST_LEN])
{
    uint8_t buf[UKEY_PIN_MAX_LEN + 16u];
    size_t pin_len = strlen(pin);
    if (pin_len > UKEY_PIN_MAX_LEN) pin_len = UKEY_PIN_MAX_LEN;
    memcpy(buf, salt, 16u);
    memcpy(buf + 16u, pin, pin_len);
    (void)crypto_sm3(buf, 16u + pin_len, out);
}

void ukey_state_init(void)
{
    if (secure_store_load(&g_ctx.store) != UKEY_OK) {
        secure_store_factory_default(&g_ctx.store);
    }
    g_ctx.pending = PENDING_NONE;
    g_ctx.pending_msg_len = 0;
    g_ctx.mode = 0;
    if (!g_ctx.store.initialized) {
        g_ctx.state = UKEY_STATE_UNINIT;
    } else if (g_ctx.store.pin_retries == 0u) {
        g_ctx.state = UKEY_STATE_PIN_LOCKED;
    } else {
        g_ctx.state = UKEY_STATE_LOCKED;
    }
}

ukey_context_t *ukey_ctx(void)
{
    return &g_ctx;
}

const char *ukey_state_name(ukey_state_t state)
{
    switch (state) {
    case UKEY_STATE_UNINIT: return "UNINIT";
    case UKEY_STATE_LOCKED: return "LOCKED";
    case UKEY_STATE_AUTHED: return "AUTHED";
    case UKEY_STATE_PIN_LOCKED: return "PIN_LOCKED";
    default: return "UNKNOWN";
    }
}

ukey_status_t ukey_set_pin(const char *pin)
{
    if (!pin || strlen(pin) == 0u || strlen(pin) > UKEY_PIN_MAX_LEN) return UKEY_ERR_ARG;
    secure_store_factory_default(&g_ctx.store);
    if (platform_random_bytes(g_ctx.store.salt, sizeof(g_ctx.store.salt)) != UKEY_OK) return UKEY_ERR;
    make_pin_hash(pin, g_ctx.store.salt, g_ctx.store.pin_hash);
    if (crypto_sm2_generate_keypair(g_ctx.store.sm2_pri, g_ctx.store.sm2_pub) != UKEY_OK) return UKEY_ERR_CRYPTO;
    g_ctx.store.initialized = 1u;
    g_ctx.store.pin_retries = UKEY_PIN_RETRY_LIMIT;
    if (secure_store_save(&g_ctx.store) != UKEY_OK) return UKEY_ERR_STORAGE;
    g_ctx.state = UKEY_STATE_LOCKED;
    return UKEY_OK;
}

ukey_status_t ukey_verify_pin(const char *pin)
{
    uint8_t hash[CRYPTO_SM3_DIGEST_LEN];
    if (g_ctx.state == UKEY_STATE_UNINIT) return UKEY_ERR_STATE;
    if (g_ctx.state == UKEY_STATE_PIN_LOCKED || g_ctx.store.pin_retries == 0u) return UKEY_ERR_LOCKED;
    make_pin_hash(pin, g_ctx.store.salt, hash);
    if (memcmp(hash, g_ctx.store.pin_hash, sizeof(hash)) == 0) {
        g_ctx.store.pin_retries = UKEY_PIN_RETRY_LIMIT;
        g_ctx.state = UKEY_STATE_AUTHED;
        (void)secure_store_save(&g_ctx.store);
        return UKEY_OK;
    }
    if (g_ctx.store.pin_retries > 0u) g_ctx.store.pin_retries--;
    if (g_ctx.store.pin_retries == 0u) g_ctx.state = UKEY_STATE_PIN_LOCKED;
    (void)secure_store_save(&g_ctx.store);
    return UKEY_ERR_AUTH;
}

ukey_status_t ukey_store_value(const char *name, const uint8_t *data, size_t len)
{
    secure_store_slot_t *free_slot = NULL;
    size_t i;
    if (g_ctx.state != UKEY_STATE_AUTHED) return UKEY_ERR_AUTH;
    if (!name || !data || len > UKEY_STORE_VALUE_LEN || strlen(name) >= UKEY_STORE_NAME_LEN) return UKEY_ERR_ARG;

    for (i = 0; i < UKEY_STORE_SLOT_COUNT; i++) {
        if (g_ctx.store.slots[i].name[0] == '\0' && !free_slot) free_slot = &g_ctx.store.slots[i];
        if (strncmp(g_ctx.store.slots[i].name, name, UKEY_STORE_NAME_LEN) == 0) {
            free_slot = &g_ctx.store.slots[i];
            break;
        }
    }
    if (!free_slot) return UKEY_ERR_STORAGE;
    memset(free_slot, 0, sizeof(*free_slot));
    strncpy(free_slot->name, name, UKEY_STORE_NAME_LEN - 1u);
    free_slot->len = (uint16_t)len;
    memcpy(free_slot->value, data, len);
    return secure_store_save(&g_ctx.store);
}

ukey_status_t ukey_read_value(const char *name, uint8_t *out, size_t out_max, size_t *out_len)
{
    size_t i;
    if (g_ctx.state != UKEY_STATE_AUTHED) return UKEY_ERR_AUTH;
    if (!name || !out || !out_len) return UKEY_ERR_ARG;
    for (i = 0; i < UKEY_STORE_SLOT_COUNT; i++) {
        if (strncmp(g_ctx.store.slots[i].name, name, UKEY_STORE_NAME_LEN) == 0) {
            if (g_ctx.store.slots[i].len > out_max) return UKEY_ERR_ARG;
            memcpy(out, g_ctx.store.slots[i].value, g_ctx.store.slots[i].len);
            *out_len = g_ctx.store.slots[i].len;
            return UKEY_OK;
        }
    }
    return UKEY_ERR_STORAGE;
}
