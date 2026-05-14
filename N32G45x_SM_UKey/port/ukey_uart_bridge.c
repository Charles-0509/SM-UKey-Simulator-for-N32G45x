#include "ukey_uart_bridge.h"
#include "ukey_protocol.h"
#include "app_config.h"
#include "n32g45x.h"

static char s_line[UKEY_MAX_LINE_LEN];
static uint16_t s_line_len;

static void tx_byte(uint8_t ch)
{
    USART_SendData(USART1, ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXDE) == RESET) {
    }
}

void ukey_uart_send_string(const char *s)
{
    while (s && *s) {
        tx_byte((uint8_t)*s++);
    }
}

void ukey_uart_bridge_init(void)
{
    s_line_len = 0u;
    ukey_uart_send_string("SM UKey UART ready. Type HELP.\r\n");
}

void ukey_uart_receive_byte(uint8_t ch)
{
    char out[UKEY_OUT_LEN];

    if (ch == '\r') {
        return;
    }
    if (ch == '\n') {
        s_line[s_line_len] = '\0';
        if (s_line_len != 0u) {
            ukey_on_command(s_line, out, sizeof(out));
            ukey_uart_send_string(out);
            ukey_uart_send_string("\r\n");
        }
        s_line_len = 0u;
        return;
    }
    if (s_line_len < (UKEY_MAX_LINE_LEN - 1u)) {
        s_line[s_line_len++] = (char)ch;
    } else {
        s_line_len = 0u;
        ukey_uart_send_string("ERR LINE_TOO_LONG\r\n");
    }
}

