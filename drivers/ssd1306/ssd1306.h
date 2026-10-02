/*
 * ssd1306.h -- SSD1306 128x64 monochrome OLED driver API (WRITTEN BY ME).
 *
 * This is a DEVICE driver: it knows the SSD1306's commands and memory layout
 * but nothing about STM32 registers. It talks to the chip only through the
 * bus driver (i2c.h). That separation is the point:
 *
 *      application -> ssd1306.c -> i2c.c -> STM32 I2C registers -> chip
 *
 * Usage model: draw into a RAM framebuffer (fast, no I2C traffic), then call
 * ssd1306_flush() to send the whole buffer to the display in one transfer.
 */
#ifndef SSD1306_H
#define SSD1306_H

#include <stdbool.h>
#include <stdint.h>
#include "status.h"

#define SSD1306_WIDTH      128u
#define SSD1306_HEIGHT     64u
#define SSD1306_PAGES      (SSD1306_HEIGHT / 8u)       /* 8 rows of 8 pixels */
#define SSD1306_FB_SIZE    (SSD1306_WIDTH * SSD1306_PAGES)   /* 1024 bytes */

#define SSD1306_ADDR_DEFAULT  0x3Cu    /* 7-bit; some modules use 0x3D */
#define SSD1306_TIMEOUT_MS    50u

typedef struct {
    uint8_t i2c_addr;
    uint8_t fb[SSD1306_FB_SIZE];       /* 1 bit per pixel, page-organised */
} ssd1306_t;

status_t ssd1306_init(ssd1306_t *dev, uint8_t i2c_addr);
status_t ssd1306_display_on(ssd1306_t *dev, bool on);
status_t ssd1306_set_contrast(ssd1306_t *dev, uint8_t contrast);
status_t ssd1306_invert(ssd1306_t *dev, bool invert);

/* Drawing: framebuffer only. Nothing appears until ssd1306_flush(). */
void     ssd1306_clear(ssd1306_t *dev);
void     ssd1306_set_pixel(ssd1306_t *dev, uint8_t x, uint8_t y, bool on);
void     ssd1306_fill_rect(ssd1306_t *dev, uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on);

/* Text: 5x7 font in a 6x8 cell. 'row' is a text row 0..7 (pixel y = row*8),
 * 'x' is a pixel column. Lowercase is drawn as uppercase; unknown -> '?'. */
void     ssd1306_print(ssd1306_t *dev, uint8_t x, uint8_t row, const char *s);

/* Send the framebuffer to the display. */
status_t ssd1306_flush(ssd1306_t *dev);

#endif
