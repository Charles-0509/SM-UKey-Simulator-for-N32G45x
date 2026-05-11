#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    UKEY_OK = 0,
    UKEY_ERR = -1,
    UKEY_ERR_ARG = -2,
    UKEY_ERR_STATE = -3,
    UKEY_ERR_AUTH = -4,
    UKEY_ERR_LOCKED = -5,
    UKEY_ERR_STORAGE = -6,
    UKEY_ERR_CRYPTO = -7
} ukey_status_t;

typedef enum {
    UKEY_STATE_UNINIT = 0,
    UKEY_STATE_LOCKED,
    UKEY_STATE_AUTHED,
    UKEY_STATE_PIN_LOCKED
} ukey_state_t;

typedef enum {
    UKEY_KEY_CONFIRM = 1,
    UKEY_KEY_CANCEL,
    UKEY_KEY_MODE
} ukey_key_t;

#endif

