# bare-metal-drivers

Hardware drivers written from scratch in C, register by register, from the datasheet - no HAL, no Arduino, no vendor libraries.

**Current status:** GPIO driver, I2C master driver, SSD1306 OLED driver.

## Why I built this
I wanted to understand what actually happens between `led_on()` and a voltage change on a pin: addresses, registers, clock gating, bit fields, and how to read a reference manual like an embedded engineer.

## Supported hardware
NUCLEO-F446RE (STM32F446RE, Cortex-M4). Runs on the default 16 MHz internal oscillator.

## Architecture
```
examples/02_oled_hello/main.c   application
        |
drivers/ssd1306/ssd1306.c       device driver (framebuffer + commands)
        |
drivers/i2c/i2c.c               bus driver (I2C1 master)
        |
drivers/gpio/gpio.c             pin driver (open-drain AF4 on PB8/PB9)
        |
platform/stm32f446/             register map, startup, linker script, SysTick timebase
        |
STM32F446 GPIO/I2C/RCC registers (memory-mapped I/O)
        |
I2C bus -> SSD1306 OLED
```
```
bare-metal-drivers/
  drivers/common/   status codes
  drivers/gpio/     GPIO driver
  drivers/i2c/      I2C master driver
  drivers/ssd1306/  SSD1306 OLED driver
  platform/stm32f446/   MCU-specific plumbing (all hand-written)
  examples/         runnable demos
  docs/             learning notes, datasheet walkthrough, provenance
```

## Register-level implementation
See [docs/02-gpio.md](docs/02-gpio.md), [docs/03-i2c.md](docs/03-i2c.md), [docs/04-ssd1306.md](docs/04-ssd1306.md): how each line of `gpio.c` comes from RM0390. Highlights: RCC clock gating, 2-bit field masking, atomic BSRR set/reset, glitch-free configuration order, compile-time register offset checks (`_Static_assert`).

## Build and run
```
sudo apt install gcc-arm-none-eabi openocd
make                          # builds 02_oled_hello (default)
make EXAMPLE=01_gpio_blink    # builds the LED example
make flash        # flash via ST-LINK (Nucleo plugged in over USB)
```
Wiring: OLED VCC->3V3, GND->GND, SCL->PB8 (D15), SDA->PB9 (D14).
Expected: OLED shows text, an uptime counter and a sweeping bar; LD2 heartbeats. Failure = fast LED blink.
`01_gpio_blink`: LD2 blinks at 1 Hz, 5 Hz while the blue button is held.

## Testing methodology
LED test, register inspection through OpenOCD/GDB, scope/logic-analyzer frequency measurement, and negative tests for invalid arguments. Details in docs/02-gpio.md.

## Lessons learned
(fill in as you go: e.g. "peripheral clocks are off at reset", "BSRR vs ODR", "why volatile")

## Limitations
Single clock config (16 MHz HSI). Polling I2C only (no IRQ/DMA), single master, no bus-recovery pulses. 5x7 font covers space..Z only. Not hardware-verified until you flash it - fill in your results.

## Future improvements
UART, MPU6050, I2C interrupts/DMA, GPIO interrupts, SD/SPI.

## Provenance
See [docs/provenance.md](docs/provenance.md).
