#ifndef UKEY_PROTOCOL_H
#define UKEY_PROTOCOL_H

#include "ukey_state.h"

void ukey_protocol_init(void);
void ukey_on_command(const char *line, char *out, size_t out_size);
void ukey_on_key(ukey_key_t key, char *out, size_t out_size);

#endif

