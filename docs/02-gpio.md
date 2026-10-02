# 02 - The GPIO driver, from datasheet to C

Documents: **RM0390** (reference manual, chapter *General-purpose I/Os (GPIO)* and *RCC*), **DS10693** (datasheet: pinout + alternate function table), **UM1724** (Nucleo-64 user manual: which pin goes to the LED/button).

## Step 1 - Which pin?
UM1724: LD2 = PA5, B1 = PC13 (active low). -> `board.h`.

## Step 2 - Is the peripheral clocked?
RCC register `AHB1ENR` (offset 0x30): bit 0 = GPIOAEN ... bit 7 = GPIOHEN. Reset value 0 = peripheral off. Writes to a gated peripheral are silently ignored, the #1 beginner bug. -> first line of `gpio_init()`.

## Step 3 - The GPIO registers (offsets from port base)
| Reg | Off | Layout | Purpose |
|---|---|---|---|
| MODER | 0x00 | 2 bits/pin | 00 input, 01 output, 10 alternate, 11 analog |
| OTYPER | 0x04 | 1 bit/pin | 0 push-pull, 1 open-drain |
| OSPEEDR | 0x08 | 2 bits/pin | edge speed |
| PUPDR | 0x0C | 2 bits/pin | 00 none, 01 pull-up, 10 pull-down |
| IDR | 0x10 | 1 bit/pin | pin level (read) |
| ODR | 0x14 | 1 bit/pin | output latch |
| BSRR | 0x18 | 16 set + 16 reset | atomic set/clear |
| AFR[2] | 0x20/24 | 4 bits/pin | which peripheral owns the pin |

Check the reset values in the manual: e.g. GPIOA_MODER resets to `0xA800 0000` because PA13/PA14/PA15 default to debug functions. **Never reconfigure PA13/PA14 or you lose the debugger.**

## Step 4 - Bit arithmetic
For a 2-bit field of pin `n`: shift = `n*2`, mask = `3 << shift`.
Make PA5 an output (01):
```
MODER = (MODER & ~(3 << 10)) | (1 << 10);
```
`& ~mask` clears the old field (without touching other pins), `| value` writes the new one. This is `reg_modify()`.

## Step 5 - Why BSRR instead of ODR
`ODR |= bit` is read-modify-write: 3 steps; an interrupt in the middle can corrupt another pin. `BSRR = bit` is one store; writing 0s does nothing, so it is atomic. Bits 0..15 set, bits 16..31 reset.

## Step 6 - API design
```c
gpio_init(port, pin, &cfg);  gpio_write(port, pin, level);
gpio_toggle(port, pin);      gpio_read(port, pin, &level);
```
All return `status_t`; invalid port/pin/null -> `STATUS_ERR_PARAM`. Config order in `gpio_init`: pull/type/speed/AF first, MODER last, so the pin never glitches.

## Verify it
1. LED test: `make flash` -> LD2 blinks 1 Hz (toggle every 500 ms); hold B1 -> 5 Hz.
2. Register inspection (OpenOCD/GDB): `mdw 0x40020000 6` shows MODER; bits 11:10 should read `01`. `mdw 0x40020014` ODR bit 5 flips each blink.
3. Oscilloscope/logic analyzer on PA5 (CN10 / Arduino D13): expect a 1 Hz square wave, 50 % duty. Measure it: period should be 1000 ms, +-1 ms.
4. Fault tests: call `gpio_write(GPIO_PORT_A, 16, 1)` -> must return `STATUS_ERR_PARAM`. Comment out the RCC enable line -> LED never lights (proves the clock gate is real).
5. Speed test: loop `gpio_toggle` with no delay and measure frequency on a scope; compare with `-O0` vs `-Os`.

## Questions to answer before moving on
1. Which address is GPIOB_BSRR? (base + offset)
2. What value do you write to BSRR to turn PB3 off?
3. Why must MODER be written *after* AFR/OTYPER?
4. Why is `ODR |= x` unsafe if an ISR also writes ODR?
5. What does `volatile` stop the compiler from doing?
