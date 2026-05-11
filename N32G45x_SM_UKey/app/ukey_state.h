#ifndef UKEY_STATE_H
#define UKEY_STATE_H

#include "secure_store.h"

typedef enum {
    PENDING_NONE = 0,
    PENDING_INIT,
    PENDING_SIGN,
    PENDING_ERASE
} pending_op_t;

typedef struct {
    secure_store_plain_t store;
    ukey_state_t state;
    pending_op_t pending;
    char pending_pin[UKEY_PIN_MAX_LEN + 1u];
    uint8_t pending_msg[UKEY_MAX_DATA_LEN];
    size_t pending_msg_len;
    uint8_t mode;
} ukey_context_t;

void ukey_state_init(void);
ukey_context_t *ukey_ctx(void);
const char *ukey_state_name(ukey_state_t state);
ukey_status_t ukey_set_pin(const char *pin);
ukey_status_t ukey_verify_pin(const char *pin);
ukey_status_t ukey_store_value(const char *name, const uint8_t *data, size_t len);
ukey_status_t ukey_read_value(const char *name, uint8_t *out, size_t out_max, size_t *out_len);

#endif

