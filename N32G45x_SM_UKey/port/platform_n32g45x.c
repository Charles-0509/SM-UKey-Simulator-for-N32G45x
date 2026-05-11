#include "platform_port.h"
#include "n32g45x.h"
#include "n32g45x_flash.h"
#include "n32g45x_rng.h"
#include <string.h>

#define UKEY_FLASH_PAGE_SIZE       2048u
#define UKEY_FLASH_BASE_ADDR       0x08000000u
#define UKEY_FLASH_SIZE_BYTES      (256u * 1024u)
#define UKEY_FLASH_SECURE_ADDR     (UKEY_FLASH_BASE_ADDR + UKEY_FLASH_SIZE_BYTES - UKEY_FLASH_PAGE_SIZE)

#ifndef UKEY_UID_BASE
#define UKEY_UID_BASE              0x1FFFF7E8u
#endif

ukey_status_t platform_flash_load_secure_area(uint8_t *buf, size_t len)
{
    if (!buf || len > PLATFORM_SECURE_AREA_SIZE || len > UKEY_FLASH_PAGE_SIZE) {
        return UKEY_ERR_ARG;
    }
    memcpy(buf, (const void *)UKEY_FLASH_SECURE_ADDR, len);
    return UKEY_OK;
}

ukey_status_t platform_flash_save_secure_area(const uint8_t *buf, size_t len)
{
    uint32_t addr = UKEY_FLASH_SECURE_ADDR;
    const uint32_t *word = (const uint32_t *)buf;
    size_t off;

    if (!buf || len > PLATFORM_SECURE_AREA_SIZE || len > UKEY_FLASH_PAGE_SIZE || (len % 4u) != 0u) {
        return UKEY_ERR_ARG;
    }

    FLASH_Unlock();
    FLASH_EraseOnePage(UKEY_FLASH_SECURE_ADDR);
    for (off = 0u; off < len; off += 4u) {
        if (FLASH_COMPL != FLASH_ProgramWord(addr + (uint32_t)off, *word++)) {
            FLASH_Lock();
            return UKEY_ERR_STORAGE;
        }
    }
    FLASH_Lock();
    return UKEY_OK;
}

ukey_status_t platform_random_bytes(uint8_t *buf, size_t len)
{
    uint32_t rnd[8];
    size_t done = 0u;

    if (!buf) {
        return UKEY_ERR_ARG;
    }
    while (done < len) {
        size_t chunk = len - done;
        if (chunk > sizeof(rnd)) {
            chunk = sizeof(rnd);
        }
        if (GetTrueRand_U32(rnd, (uint32_t)((chunk + 3u) / 4u)) != RNG_OK) {
            return UKEY_ERR;
        }
        memcpy(buf + done, rnd, chunk);
        done += chunk;
    }
    return UKEY_OK;
}

const char *platform_device_id(void)
{
    static char id[32];
    const uint32_t *uid = (const uint32_t *)UKEY_UID_BASE;
    static const char hex[] = "0123456789ABCDEF";
    uint32_t v[3];
    uint8_t bytes[12];
    uint32_t pos = 0u;
    uint32_t i;

    v[0] = uid[0];
    v[1] = uid[1];
    v[2] = uid[2];
    memcpy(bytes, v, sizeof(bytes));
    id[pos++] = 'N';
    id[pos++] = '3';
    id[pos++] = '2';
    id[pos++] = '-';
    for (i = 0u; i < sizeof(bytes) && pos < sizeof(id) - 1u; i++) {
        id[pos++] = hex[(bytes[i] >> 4) & 0x0Fu];
        if (pos >= sizeof(id) - 1u) {
            break;
        }
        id[pos++] = hex[bytes[i] & 0x0Fu];
    }
    id[pos] = '\0';
    return id;
}
