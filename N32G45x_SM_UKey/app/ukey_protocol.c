#include "ukey_protocol.h"
#include "utils_hex.h"
#include <stdio.h>
#include <string.h>

static void reply_status(char *out, size_t out_size, ukey_status_t st)
{
    const char *s = "ERR";
    if (st == UKEY_ERR_ARG) s = "ERR ARG";
    else if (st == UKEY_ERR_STATE) s = "ERR STATE";
    else if (st == UKEY_ERR_AUTH) s = "ERR AUTH";
    else if (st == UKEY_ERR_LOCKED) s = "ERR PIN_LOCKED";
    else if (st == UKEY_ERR_STORAGE) s = "ERR STORAGE";
    else if (st == UKEY_ERR_CRYPTO) s = "ERR CRYPTO";
    snprintf(out, out_size, "%s", s);
}

static char *next_token(char **p)
{
    char *s = *p;
    char *tok;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0' || *s == '\r' || *s == '\n') {
        *p = s;
        return NULL;
    }
    tok = s;
    while (*s && *s != ' ' && *s != '\t' && *s != '\r' && *s != '\n') s++;
    if (*s) *s++ = '\0';
    *p = s;
    return tok;
}

void ukey_protocol_init(void)
{
    ukey_state_init();
}

void ukey_on_key(ukey_key_t key, char *out, size_t out_size)
{
    ukey_context_t *ctx = ukey_ctx();
    uint8_t sig[CRYPTO_SM2_SIG_LEN];
    char hex[CRYPTO_SM2_SIG_LEN * 2u + 1u];

    if (key == UKEY_KEY_CANCEL) {
        ctx->pending = PENDING_NONE;
        snprintf(out, out_size, "ERR USER_CANCEL");
        return;
    }
    if (key == UKEY_KEY_MODE) {
        ctx->mode = (uint8_t)((ctx->mode + 1u) % 3u);
        snprintf(out, out_size, "OK MODE %u STATE=%s", ctx->mode, ukey_state_name(ctx->state));
        return;
    }
    if (key != UKEY_KEY_CONFIRM) {
        snprintf(out, out_size, "ERR KEY");
        return;
    }

    if (ctx->pending == PENDING_INIT) {
        ukey_status_t st = ukey_set_pin(ctx->pending_pin);
        ctx->pending = PENDING_NONE;
        if (st == UKEY_OK) snprintf(out, out_size, "OK INIT DONE");
        else reply_status(out, out_size, st);
        return;
    }
    if (ctx->pending == PENDING_SIGN) {
        if (crypto_sm2_sign(ctx->store.sm2_pri, ctx->pending_msg, ctx->pending_msg_len, sig) != UKEY_OK ||
            bytes_to_hex(sig, sizeof(sig), hex, sizeof(hex)) != 0) {
            snprintf(out, out_size, "ERR CRYPTO");
        } else {
            snprintf(out, out_size, "OK SIGN %s", hex);
        }
        ctx->pending = PENDING_NONE;
        return;
    }
    if (ctx->pending == PENDING_ERASE) {
        secure_store_factory_default(&ctx->store);
        (void)secure_store_save(&ctx->store);
        ctx->state = UKEY_STATE_UNINIT;
        ctx->pending = PENDING_NONE;
        snprintf(out, out_size, "OK ERASE DONE");
        return;
    }
    snprintf(out, out_size, "ERR NO_PENDING_OP");
}

void ukey_on_command(const char *line, char *out, size_t out_size)
{
    char buf[UKEY_MAX_LINE_LEN];
    char *p = buf;
    char *cmd;
    ukey_context_t *ctx = ukey_ctx();

    snprintf(out, out_size, "ERR");
    if (!line || !out || out_size == 0u) return;
    strncpy(buf, line, sizeof(buf) - 1u);
    buf[sizeof(buf) - 1u] = '\0';
    cmd = next_token(&p);
    if (!cmd) {
        snprintf(out, out_size, "ERR EMPTY");
        return;
    }

    if (strcmp(cmd, "HELP") == 0) {
        snprintf(out, out_size, "OK HELP INFO INIT VERIFY_PIN GET_PUBKEY SIGN SM3 STORE READ ERASE KEY1 KEY2 KEY3");
    } else if (strcmp(cmd, "INFO") == 0) {
        snprintf(out, out_size, "OK INFO VERSION=%s STATE=%s RETRIES=%u DEV=%s",
                 UKEY_VERSION, ukey_state_name(ctx->state), ctx->store.pin_retries, platform_device_id());
    } else if (strcmp(cmd, "INIT") == 0) {
        char *pin = next_token(&p);
        if (!pin || strlen(pin) > UKEY_PIN_MAX_LEN) {
            snprintf(out, out_size, "ERR ARG");
        } else {
            strncpy(ctx->pending_pin, pin, sizeof(ctx->pending_pin) - 1u);
            ctx->pending_pin[sizeof(ctx->pending_pin) - 1u] = '\0';
            ctx->pending = PENDING_INIT;
            snprintf(out, out_size, "WAIT PRESS KEY1 TO INIT, KEY2 TO CANCEL");
        }
    } else if (strcmp(cmd, "VERIFY_PIN") == 0) {
        char *pin = next_token(&p);
        ukey_status_t st = pin ? ukey_verify_pin(pin) : UKEY_ERR_ARG;
        if (st == UKEY_OK) snprintf(out, out_size, "OK PIN VERIFIED");
        else reply_status(out, out_size, st);
    } else if (strcmp(cmd, "GET_PUBKEY") == 0) {
        char hex[CRYPTO_SM2_PUB_LEN * 2u + 1u];
        if (!ctx->store.initialized || bytes_to_hex(ctx->store.sm2_pub, CRYPTO_SM2_PUB_LEN, hex, sizeof(hex)) != 0) {
            snprintf(out, out_size, "ERR STATE");
        } else {
            snprintf(out, out_size, "OK PUBKEY 04%s", hex);
        }
    } else if (strcmp(cmd, "SIGN") == 0) {
        char *hex_msg = next_token(&p);
        if (ctx->state != UKEY_STATE_AUTHED) {
            snprintf(out, out_size, "ERR AUTH");
        } else if (!hex_msg || hex_to_bytes(hex_msg, ctx->pending_msg, sizeof(ctx->pending_msg), &ctx->pending_msg_len) != 0) {
            snprintf(out, out_size, "ERR ARG");
        } else {
            ctx->pending = PENDING_SIGN;
            snprintf(out, out_size, "WAIT PRESS KEY1 TO SIGN, KEY2 TO CANCEL");
        }
    } else if (strcmp(cmd, "SM3") == 0) {
        uint8_t data[UKEY_MAX_DATA_LEN];
        uint8_t digest[CRYPTO_SM3_DIGEST_LEN];
        char hex[CRYPTO_SM3_DIGEST_LEN * 2u + 1u];
        size_t len = 0;
        char *arg = next_token(&p);
        if (!arg || hex_to_bytes(arg, data, sizeof(data), &len) != 0 ||
            crypto_sm3(data, len, digest) != UKEY_OK ||
            bytes_to_hex(digest, sizeof(digest), hex, sizeof(hex)) != 0) {
            snprintf(out, out_size, "ERR ARG");
        } else {
            snprintf(out, out_size, "OK SM3 %s", hex);
        }
    } else if (strcmp(cmd, "STORE") == 0) {
        uint8_t data[UKEY_STORE_VALUE_LEN];
        size_t len = 0;
        char *name = next_token(&p);
        char *hex_data = next_token(&p);
        ukey_status_t st = (!name || !hex_data || hex_to_bytes(hex_data, data, sizeof(data), &len) != 0)
            ? UKEY_ERR_ARG : ukey_store_value(name, data, len);
        if (st == UKEY_OK) snprintf(out, out_size, "OK STORE DONE");
        else reply_status(out, out_size, st);
    } else if (strcmp(cmd, "READ") == 0) {
        uint8_t data[UKEY_STORE_VALUE_LEN];
        size_t len = 0;
        char hex[UKEY_STORE_VALUE_LEN * 2u + 1u];
        char *name = next_token(&p);
        ukey_status_t st = name ? ukey_read_value(name, data, sizeof(data), &len) : UKEY_ERR_ARG;
        if (st == UKEY_OK && bytes_to_hex(data, len, hex, sizeof(hex)) == 0) snprintf(out, out_size, "OK READ %s", hex);
        else reply_status(out, out_size, st);
    } else if (strcmp(cmd, "ERASE") == 0) {
        ctx->pending = PENDING_ERASE;
        snprintf(out, out_size, "WAIT PRESS KEY1 TO ERASE, KEY2 TO CANCEL");
    } else if (strcmp(cmd, "KEY1") == 0) {
        ukey_on_key(UKEY_KEY_CONFIRM, out, out_size);
    } else if (strcmp(cmd, "KEY2") == 0) {
        ukey_on_key(UKEY_KEY_CANCEL, out, out_size);
    } else if (strcmp(cmd, "KEY3") == 0) {
        ukey_on_key(UKEY_KEY_MODE, out, out_size);
    } else {
        snprintf(out, out_size, "ERR UNKNOWN_CMD");
    }
}

