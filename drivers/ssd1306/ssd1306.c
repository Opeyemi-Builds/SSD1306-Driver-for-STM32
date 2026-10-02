#include "ssd1306.h"
#include "ssd1306_cmds.h"
#include "ssd1306_font.h"
#include "i2c.h"


static const uint8_t init_cmds[] = {
    SSD1306_CMD_DISPLAY_OFF,                 /* never configure while lit   */
    SSD1306_CMD_CLOCK_DIV,    0x80u,         /* default oscillator/divider  */
    SSD1306_CMD_MUX_RATIO,    0x3Fu,         /* 64 rows (63 = rows - 1)     */
    SSD1306_CMD_DISPLAY_OFFS, 0x00u,         /* no vertical shift           */
    SSD1306_CMD_START_LINE | 0x00u,          /* RAM line 0 at top           */
    SSD1306_CMD_CHARGE_PUMP,  0x14u,         /* enable internal 7.5 V pump  */
    SSD1306_CMD_MEM_MODE,     0x00u,         /* horizontal addressing       */
    SSD1306_CMD_SEG_REMAP,                   /* orientation: not mirrored   */
    SSD1306_CMD_COM_SCAN_DEC,                /* ... and not upside down     */
    SSD1306_CMD_COM_PINS,     0x12u,         /* hardware layout of 128x64   */
    SSD1306_CMD_SET_CONTRAST, 0xCFu,
    SSD1306_CMD_PRECHARGE,    0xF1u,
    SSD1306_CMD_VCOMH,        0x40u,
    SSD1306_CMD_RAM_FOLLOW,                  /* show RAM, not all-pixels-on */
    SSD1306_CMD_NORMAL,
    SSD1306_CMD_DISPLAY_ON
};

/* Window = whole screen: columns 0..127, pages 0..7. After this the chip
 * auto-increments through RAM as data bytes arrive. */
static const uint8_t window_cmds[] = {
    SSD1306_CMD_COL_ADDR,  0u, (uint8_t)(SSD1306_WIDTH - 1u),
    SSD1306_CMD_PAGE_ADDR, 0u, (uint8_t)(SSD1306_PAGES - 1u)
};

/* Send command bytes: control byte 0x00, then all bytes are commands. */
static status_t send_cmds(const ssd1306_t *dev, const uint8_t *cmds, size_t n)
{
    return i2c_mem_write(dev->i2c_addr, SSD1306_CTRL_CMD, cmds, n, SSD1306_TIMEOUT_MS);
}

status_t ssd1306_init(ssd1306_t *dev, uint8_t i2c_addr)
{
    if (dev == NULL || i2c_addr > 0x7Fu) { return STATUS_ERR_PARAM; }
    dev->i2c_addr = i2c_addr;
    ssd1306_clear(dev);

    /* The SSD1306 has no ID register, so "is it there?" = "does it ACK?". */
    status_t st = i2c_probe(i2c_addr, SSD1306_TIMEOUT_MS);
    if (st != STATUS_OK) { return st; }

    st = send_cmds(dev, init_cmds, sizeof init_cmds);
    if (st != STATUS_OK) { return st; }

    return ssd1306_flush(dev);               /* start from a blank screen */
}

status_t ssd1306_display_on(ssd1306_t *dev, bool on)
{
    if (dev == NULL) { return STATUS_ERR_PARAM; }
    const uint8_t c = on ? SSD1306_CMD_DISPLAY_ON : SSD1306_CMD_DISPLAY_OFF;
    return send_cmds(dev, &c, 1u);
}

status_t ssd1306_set_contrast(ssd1306_t *dev, uint8_t contrast)
{
    if (dev == NULL) { return STATUS_ERR_PARAM; }
    const uint8_t c[2] = { SSD1306_CMD_SET_CONTRAST, contrast };
    return send_cmds(dev, c, sizeof c);
}

status_t ssd1306_invert(ssd1306_t *dev, bool invert)
{
    if (dev == NULL) { return STATUS_ERR_PARAM; }
    const uint8_t c = invert ? SSD1306_CMD_INVERT : SSD1306_CMD_NORMAL;
    return send_cmds(dev, &c, 1u);
}


void ssd1306_clear(ssd1306_t *dev)
{
    if (dev == NULL) { return; }
    for (uint32_t i = 0; i < SSD1306_FB_SIZE; i++) { dev->fb[i] = 0u; }
}

void ssd1306_set_pixel(ssd1306_t *dev, uint8_t x, uint8_t y, bool on)
{
    if (dev == NULL || x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) { return; }  /* clip */

    const uint32_t idx = (uint32_t)x + (uint32_t)(y / 8u) * SSD1306_WIDTH;
    const uint8_t  bit = (uint8_t)(1u << (y % 8u));
    if (on) { dev->fb[idx] |= bit; } else { dev->fb[idx] &= (uint8_t)~bit; }
}

void ssd1306_fill_rect(ssd1306_t *dev, uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on)
{
    for (uint32_t yy = y; yy < (uint32_t)y + h; yy++) {
        for (uint32_t xx = x; xx < (uint32_t)x + w; xx++) {
            if (xx < SSD1306_WIDTH && yy < SSD1306_HEIGHT) {
                ssd1306_set_pixel(dev, (uint8_t)xx, (uint8_t)yy, on);
            }
        }
    }
}

static void draw_char(ssd1306_t *dev, uint8_t x, uint8_t row, char c)
{
    if (c >= 'a' && c <= 'z') { c = (char)(c - 'a' + 'A'); }
    if (c < FONT_FIRST_CHAR || c > FONT_LAST_CHAR) { c = '?'; }
    const uint8_t *glyph = font5x7[c - FONT_FIRST_CHAR];

    for (uint32_t col = 0; col < FONT_GLYPH_W; col++) {
        const uint32_t px = (uint32_t)x + col;
        if (px < SSD1306_WIDTH) {
            /* Glyph column bytes are already in page layout*/
            dev->fb[px + (uint32_t)row * SSD1306_WIDTH] = glyph[col];
        }
    }
    /* column 6 of the 6-pixel cell stays blank as letter spacing */
    const uint32_t gap = (uint32_t)x + FONT_GLYPH_W;
    if (gap < SSD1306_WIDTH) { dev->fb[gap + (uint32_t)row * SSD1306_WIDTH] = 0u; }
}

void ssd1306_print(ssd1306_t *dev, uint8_t x, uint8_t row, const char *s)
{
    if (dev == NULL || s == NULL || row >= SSD1306_PAGES) { return; }
    uint32_t cx = x;
    while (*s != '\0' && cx < SSD1306_WIDTH) {
        draw_char(dev, (uint8_t)cx, row, *s++);
        cx += FONT_GLYPH_W + 1u;
    }
}

status_t ssd1306_flush(ssd1306_t *dev)
{
    if (dev == NULL) { return STATUS_ERR_PARAM; }

    status_t st = send_cmds(dev, window_cmds, sizeof window_cmds);
    if (st != STATUS_OK) { return st; }


    return i2c_mem_write(dev->i2c_addr, SSD1306_CTRL_DATA, dev->fb, SSD1306_FB_SIZE,
                         SSD1306_TIMEOUT_MS);
}
