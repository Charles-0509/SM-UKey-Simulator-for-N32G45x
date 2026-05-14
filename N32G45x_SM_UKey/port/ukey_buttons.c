#include "ukey_buttons.h"
#include "ukey_protocol.h"
#include "ukey_uart_bridge.h"
#include "app_config.h"
#if UKEY_ENABLE_USB_CDC
#include "ukey_usb_bridge.h"
#endif
#include "n32g45x.h"

#ifndef UKEY_BUTTON_GPIO
#define UKEY_BUTTON_GPIO              GPIOA
#endif

#ifndef UKEY_BUTTON_GPIO_CLK
#define UKEY_BUTTON_GPIO_CLK          RCC_APB2_PERIPH_GPIOA
#endif

#ifndef UKEY_BUTTON1_PIN
#define UKEY_BUTTON1_PIN              GPIO_PIN_4
#endif

#ifndef UKEY_BUTTON2_PIN
#define UKEY_BUTTON2_PIN              GPIO_PIN_5
#endif

#ifndef UKEY_BUTTON_ACTIVE_LOW
#define UKEY_BUTTON_ACTIVE_LOW        1u
#endif

#ifndef UKEY_BUTTON_DEBOUNCE_POLLS
#define UKEY_BUTTON_DEBOUNCE_POLLS    5000u
#endif

typedef struct {
    uint16_t pin;
    ukey_key_t key;
    uint16_t counter;
    uint8_t latched;
} ukey_button_t;

static ukey_button_t s_buttons[] = {
    { UKEY_BUTTON1_PIN, UKEY_KEY_CONFIRM, 0u, 0u },
    { UKEY_BUTTON2_PIN, UKEY_KEY_CANCEL,  0u, 0u }
};

static uint8_t button_is_pressed(uint16_t pin)
{
    uint8_t level;

    level = GPIO_ReadInputDataBit(UKEY_BUTTON_GPIO, pin);
#if UKEY_BUTTON_ACTIVE_LOW
    return (level == 0u) ? 1u : 0u;
#else
    return (level != 0u) ? 1u : 0u;
#endif
}

static void emit_key_result(ukey_key_t key)
{
    char out[UKEY_OUT_LEN];

    ukey_on_key(key, out, sizeof(out));
    ukey_uart_send_string(out);
    ukey_uart_send_string("\r\n");
#if UKEY_ENABLE_USB_CDC
    ukey_usb_send_string(out);
    ukey_usb_send_string("\r\n");
#endif
}

void ukey_buttons_init(void)
{
    GPIO_InitType gpio;

    RCC_EnableAPB2PeriphClk(UKEY_BUTTON_GPIO_CLK, ENABLE);

    GPIO_InitStruct(&gpio);
    gpio.Pin        = UKEY_BUTTON1_PIN | UKEY_BUTTON2_PIN;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
#if UKEY_BUTTON_ACTIVE_LOW
    gpio.GPIO_Mode  = GPIO_Mode_IPU;
#else
    gpio.GPIO_Mode  = GPIO_Mode_IPD;
#endif
    GPIO_InitPeripheral(UKEY_BUTTON_GPIO, &gpio);
}

void ukey_buttons_poll(void)
{
    uint32_t i;

    for (i = 0u; i < (sizeof(s_buttons) / sizeof(s_buttons[0])); i++) {
        ukey_button_t *button;

        button = &s_buttons[i];
        if (button_is_pressed(button->pin)) {
            if (button->counter < UKEY_BUTTON_DEBOUNCE_POLLS) {
                button->counter++;
            } else if (button->latched == 0u) {
                button->latched = 1u;
                emit_key_result(button->key);
            }
        } else {
            button->counter = 0u;
            button->latched = 0u;
        }
    }
}
