/*
 * ssd1306_cmds.h -- WRITTEN BY ME, from the SSD1306 datasheet
 * ("Command Table" section). Every constant below is one command byte
 * (or the first byte of a command that takes parameter bytes).
 */
#ifndef SSD1306_CMDS_H
#define SSD1306_CMDS_H

/* I2C control byte: first byte after the address tells the chip what follows.
 *   bit7 Co  : 0 = all remaining bytes are of the same kind
 *   bit6 D/C#: 0 = command bytes, 1 = display data (RAM) bytes */
#define SSD1306_CTRL_CMD          0x00u
#define SSD1306_CTRL_DATA         0x40u

#define SSD1306_CMD_SET_CONTRAST  0x81u  /* + 1 byte: 0..255             */
#define SSD1306_CMD_RAM_FOLLOW    0xA4u  /* show RAM contents            */
#define SSD1306_CMD_NORMAL        0xA6u  /* 1 = pixel on                 */
#define SSD1306_CMD_INVERT        0xA7u
#define SSD1306_CMD_DISPLAY_OFF   0xAEu  /* sleep                        */
#define SSD1306_CMD_DISPLAY_ON    0xAFu
#define SSD1306_CMD_MEM_MODE      0x20u  /* + 1 byte: 0 = horizontal     */
#define SSD1306_CMD_COL_ADDR      0x21u  /* + start, end (0..127)        */
#define SSD1306_CMD_PAGE_ADDR     0x22u  /* + start, end (0..7)          */
#define SSD1306_CMD_START_LINE    0x40u  /* | line (0..63)               */
#define SSD1306_CMD_SEG_REMAP     0xA1u  /* column 127 -> SEG0 (mirror X) */
#define SSD1306_CMD_MUX_RATIO     0xA8u  /* + 1 byte: rows-1             */
#define SSD1306_CMD_COM_SCAN_DEC  0xC8u  /* scan rows bottom-up (flip Y) */
#define SSD1306_CMD_DISPLAY_OFFS  0xD3u  /* + 1 byte                     */
#define SSD1306_CMD_CLOCK_DIV     0xD5u  /* + 1 byte                     */
#define SSD1306_CMD_PRECHARGE     0xD9u  /* + 1 byte                     */
#define SSD1306_CMD_COM_PINS      0xDAu  /* + 1 byte                     */
#define SSD1306_CMD_VCOMH         0xDBu  /* + 1 byte                     */
#define SSD1306_CMD_CHARGE_PUMP   0x8Du  /* + 1 byte: 0x14 = enable      */

#endif
