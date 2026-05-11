#include "utils_hex.h"

static int hex_val(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int hex_to_bytes(const char *hex, uint8_t *out, size_t out_max, size_t *out_len)
{
    size_t n = 0;
    size_t i;
    int hi;
    int lo;

    if (!hex || !out || !out_len) return -1;
    while (*hex == ' ' || *hex == '\t') hex++;
    while (hex[n] != '\0' && hex[n] != '\r' && hex[n] != '\n') n++;
    if ((n % 2u) != 0u || (n / 2u) > out_max) return -1;

    *out_len = n / 2u;
    for (i = 0; i < *out_len; i++) {
        hi = hex_val(hex[i * 2u]);
        lo = hex_val(hex[i * 2u + 1u]);
        if (hi < 0 || lo < 0) return -1;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return 0;
}

int bytes_to_hex(const uint8_t *data, size_t len, char *out, size_t out_size)
{
    static const char *digits = "0123456789ABCDEF";
    size_t i;
    if (!data || !out || out_size < (len * 2u + 1u)) return -1;
    for (i = 0; i < len; i++) {
        out[i * 2u] = digits[(data[i] >> 4) & 0x0Fu];
        out[i * 2u + 1u] = digits[data[i] & 0x0Fu];
    }
    out[len * 2u] = '\0';
    return 0;
}
