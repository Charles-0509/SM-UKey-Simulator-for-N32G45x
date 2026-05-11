#ifndef PLATFORM_PORT_H
#define PLATFORM_PORT_H

#include "app_types.h"

#define PLATFORM_SECURE_AREA_SIZE 1024u

ukey_status_t platform_flash_load_secure_area(uint8_t *buf, size_t len);
ukey_status_t platform_flash_save_secure_area(const uint8_t *buf, size_t len);
ukey_status_t platform_random_bytes(uint8_t *buf, size_t len);
const char *platform_device_id(void);

#endif

