/*
 * 01_gpio_blink/main.c -- WRITTEN BY ME. First test of the GPIO driver.
 *
 *  - LD2 (PA5) is an OUTPUT: we toggle it.
 *  - B1  (PC13) is an INPUT: while pressed, the LED blinks fast.
 */
#include "gpio.h"
#include "timebase.h"
#include "board.h"

int main(void)
{
    timebase_init();

    const gpio_config_t led_cfg = {
        .mode  = GPIO_MODE_OUTPUT,
        .otype = GPIO_OTYPE_PUSH_PULL,
        .speed = GPIO_SPEED_LOW,
        .pull  = GPIO_PULL_NONE,
        .af    = 0u,
    };
    const gpio_config_t btn_cfg = {
        .mode  = GPIO_MODE_INPUT,
        .otype = GPIO_OTYPE_PUSH_PULL,   /* ignored for inputs */
        .speed = GPIO_SPEED_LOW,         /* ignored for inputs */
        .pull  = GPIO_PULL_NONE,         /* board already has an external pull-up */
        .af    = 0u,
    };

    if (gpio_init(BOARD_LED_PORT, BOARD_LED_PIN, &led_cfg) != STATUS_OK) { for (;;) { } }
    if (gpio_init(BOARD_BUTTON_PORT, BOARD_BUTTON_PIN, &btn_cfg) != STATUS_OK) { for (;;) { } }

    for (;;) {
        bool level = true;
        (void)gpio_read(BOARD_BUTTON_PORT, BOARD_BUTTON_PIN, &level);
        const bool pressed = !level;               /* active LOW */

        (void)gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
        delay_ms(pressed ? 100u : 500u);
    }
}
