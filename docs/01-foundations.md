# 01 - Foundations: what is a driver?

## The layers
```
Application (main.c)        "blink the LED"
      |
Driver (gpio.c)             "configure pin PA5 as output, set it high"
      |
Registers (memory-mapped)   *(uint32_t*)0x40020014 = ...
      |
Peripheral (GPIO block)     digital logic inside the MCU
      |
Pin -> resistor -> LED      physical world
```

| Term | Meaning |
|---|---|
| **Application** | Code that decides *what* to do. Should not know register addresses. |
| **Driver** | Code that knows *how* to operate one piece of hardware, and exposes a clean API. |
| **HAL** | Vendor-written driver library covering all peripherals (STM32 HAL). We are NOT using it. |
| **BSP** | Board Support Package: facts about one board (LED on PA5). Ours is `board.h`. |
| **Firmware** | The program living in the MCU's flash. Everything we build is firmware. |
| **OS / kernel** | Optional scheduler + driver framework. Bare-metal means none: our code is the only thing running. |

## Peripheral, register, memory-mapped I/O
- A **peripheral** is a hardware block inside the MCU (GPIO, UART, I2C...) that does a job on its own, independent of the CPU.
- A **register** is a 32-bit storage location inside a peripheral. Some bits are *settings* (write), some are *status* (read), some are *data*.
- **Memory-mapped I/O**: registers are given addresses in the same address space as RAM. A normal load/store instruction to that address talks to hardware instead of memory.

`GPIOA->ODR |= (1 << 5);` means: *load* the 32-bit word at 0x40020014, set bit 5, *store* it back. The GPIO block sees that bit change and drives PA5 high.

## Why `volatile`
Hardware changes registers by itself, and writes have side effects. `volatile` forbids the compiler from caching, reordering, merging or deleting accesses. See `reg32_t` in `stm32f446_regs.h`.

## How an address is found
Reference manual (RM0390) memory map: GPIOA base = `0x4002 0000`. Register map table for a peripheral: each register has an *offset*. Address = base + offset. ODR offset is 0x14, so GPIOA_ODR = `0x4002 0014`. Our struct in `stm32f446_regs.h` lets the compiler compute that, and `_Static_assert` proves we typed the offsets right.

## Why C
Direct memory access via pointers, no runtime or garbage collector, predictable timing, tiny footprint, every toolchain supports it, and vendors ship C headers.
