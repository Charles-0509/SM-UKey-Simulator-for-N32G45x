#ifndef UTILS_HEX_H
#define UTILS_HEX_H

#include "app_types.h"

int hex_to_bytes(const char *hex, uint8_t *out, size_t out_max, size_t *out_len);
int bytes_to_hex(const uint8_t *data, size_t len, char *out, size_t out_size);

#endif

