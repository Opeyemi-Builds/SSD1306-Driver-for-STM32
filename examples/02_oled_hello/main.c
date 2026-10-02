/*
 * 02_oled_hello/main.c -- WRITTEN BY ME. Exercises gpio + i2c + ssd1306.
 *
 * Wiring (OLED module -> NUCLEO-F446RE):
 *   VCC -> 3V3,  GND -> GND,  SCL -> PB8 (D15),  SDA -> PB9 (D14)
 *
 * Shows a title, a live seconds counter, and a bar that sweeps across.
 * If any driver call fails, the LED blinks fast and the error is shown on
 * the display when possible.
 */
#include "gpio.h"
#include "i2c.h"
#include "ssd1306.h"
#include "timebase.h"
#include "board.h"

static ssd1306_t s_oled;     /* ~1 KB framebuffer: static, not on the stack */

static void fatal(void)
{
    for (;;) {
        (void)gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
        delay_ms(80u);
    }
}

/* Tiny integer -> text (no printf in a freestanding build). */
static void u32_to_str(uint32_t v, char *out)
{
    char tmp[11];
    uint32_t n = 0;
    do { tmp[n++] = (char)('0' + (v % 10u)); v /= 10u; } while (v != 0u);
    while (n > 0u) { *out++ = tmp[--n]; }
    *out = '\0';
}

int main(void)
{
    timebase_init();

    const gpio_config_t led = { GPIO_MODE_OUTPUT, GPIO_OTYPE_PUSH_PULL,
                                GPIO_SPEED_LOW, GPIO_PULL_NONE, 0u };
    (void)gpio_init(BOARD_LED_PORT, BOARD_LED_PIN, &led);

    if (i2c_init(I2C_SPEED_FAST_400K) != STATUS_OK)               { fatal(); }
    if (ssd1306_init(&s_oled, SSD1306_ADDR_DEFAULT) != STATUS_OK) { fatal(); }

    uint32_t seconds = 0;
    uint8_t  bar = 0;

    for (;;) {
        char num[11];
        u32_to_str(seconds, num);

        ssd1306_clear(&s_oled);
        ssd1306_print(&s_oled, 0, 0, "BARE METAL SSD1306");
        ssd1306_print(&s_oled, 0, 2, "DRIVER IN PURE C");
        ssd1306_print(&s_oled, 0, 4, "UPTIME S:");
        ssd1306_print(&s_oled, 60, 4, num);
        ssd1306_fill_rect(&s_oled, 0, 56, bar, 8, true);          /* sweeping bar */

        if (ssd1306_flush(&s_oled) != STATUS_OK) { fatal(); }

        (void)gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);          /* heartbeat */
        bar = (uint8_t)((bar + 8u) % (SSD1306_WIDTH + 8u));
        delay_ms(250u);
        if (bar == 0u) { seconds++; }
    }
}
