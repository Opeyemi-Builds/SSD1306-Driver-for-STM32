# Who wrote what

| Code | Origin |
|---|---|
| `drivers/gpio/*`, `drivers/i2c/*`, `drivers/ssd1306/*`, `drivers/common/*` | Mine (the driver logic) |
| `ssd1306_font.h` | 5x7 glyph data: the classic public-domain bitmap font table (data, not code) |
| `platform/stm32f446/stm32f446_regs.h` | Mine, hand-translated from RM0390 (replaces ST's CMSIS header) |
| `platform/stm32f446/startup.c`, `.ld`, `timebase.c`, `libc_min.c` | Mine |
| `examples/*`, `Makefile` | Mine |
| Vendor code (ST HAL/CMSIS) | **None used** |
| Third-party libraries (Adafruit, U8g2, ...) | **None used** |
| Toolchain | GNU Arm Embedded `arm-none-eabi-gcc` |
