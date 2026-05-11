#include "ukey_usb_bridge.h"
#include "ukey_protocol.h"
#include "app_config.h"
#include "hw_config.h"
#include <string.h>

extern uint8_t USART_Rx_Buffer[USART_RX_DATA_SIZE];
extern uint32_t USART_Rx_ptr_in;
extern uint32_t USART_Rx_ptr_out;

static char s_line[UKEY_MAX_LINE_LEN];
static uint16_t s_line_len;

static void tx_byte(uint8_t ch)
{
    uint32_t next = USART_Rx_ptr_in + 1u;
    if (next == USART_RX_DATA_SIZE) {
        next = 0u;
    }
    if (next == USART_Rx_ptr_out) {
        return;
    }
    USART_Rx_Buffer[USART_Rx_ptr_in] = ch;
    USART_Rx_ptr_in = next;
}

void ukey_usb_send_string(const char *s)
{
    while (s && *s) {
        tx_byte((uint8_t)*s++);
    }
}

void ukey_usb_bridge_init(void)
{
    s_line_len = 0u;
    ukey_protocol_init();
    ukey_usb_send_string("SM UKey ready. Type HELP.\r\n");
}

void ukey_usb_receive_bytes(const uint8_t *data, uint16_t len)
{
    char out[UKEY_OUT_LEN];
    uint16_t i;

    for (i = 0u; i < len; i++) {
        uint8_t ch = data[i];
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            s_line[s_line_len] = '\0';
            if (s_line_len != 0u) {
                ukey_on_command(s_line, out, sizeof(out));
                ukey_usb_send_string(out);
                ukey_usb_send_string("\r\n");
            }
            s_line_len = 0u;
            continue;
        }
        if (s_line_len < (UKEY_MAX_LINE_LEN - 1u)) {
            s_line[s_line_len++] = (char)ch;
        } else {
            s_line_len = 0u;
            ukey_usb_send_string("ERR LINE_TOO_LONG\r\n");
        }
    }
}
