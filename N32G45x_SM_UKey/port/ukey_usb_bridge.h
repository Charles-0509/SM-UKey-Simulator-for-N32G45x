#ifndef UKEY_USB_BRIDGE_H
#define UKEY_USB_BRIDGE_H

#include <stdint.h>

void ukey_usb_bridge_init(void);
void ukey_usb_receive_bytes(const uint8_t *data, uint16_t len);
void ukey_usb_send_string(const char *s);

#endif

