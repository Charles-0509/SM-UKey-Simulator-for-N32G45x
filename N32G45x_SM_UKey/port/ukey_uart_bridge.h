#ifndef UKEY_UART_BRIDGE_H
#define UKEY_UART_BRIDGE_H

#include <stdint.h>

void ukey_uart_bridge_init(void);
void ukey_uart_receive_byte(uint8_t ch);
void ukey_uart_send_string(const char *s);

#endif

