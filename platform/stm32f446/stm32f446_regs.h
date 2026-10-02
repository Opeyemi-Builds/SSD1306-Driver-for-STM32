/*
 * stm32f446_regs.h  --  WRITTEN BY ME (hand-translated from RM0390)
 *
 * This file is our own tiny replacement for ST's CMSIS device header.
 * Every struct below mirrors a register table in the STM32F446 reference
 * manual (RM0390). The _Static_assert lines at the bottom make the COMPILER
 * check that our struct layout matches the "address offset" column of the
 * manual -- if you mistype a register, the build fails instead of the
 * hardware silently misbehaving.
 */
#ifndef STM32F446_REGS_H
#define STM32F446_REGS_H

#include <stdint.h>
#include <stddef.h>

#define BIT(n)  (1UL << (n))

/* A peripheral register can change without the CPU writing it (hardware sets
 * flags, receives data...). 'volatile' tells the compiler: every access in my
 * source must really happen, in this order. Never cache, merge or drop it. */
typedef volatile uint32_t reg32_t;

/* Read-modify-write of a multi-bit field: clear 'mask', then OR in 'value'. */
static inline void reg_modify(reg32_t *reg, uint32_t mask, uint32_t value)
{
    *reg = (*reg & ~mask) | (value & mask);
}

/* ---------------------------------------------------------------- *
 * Bus base addresses (RM0390, "Memory map" table)
 * ---------------------------------------------------------------- */
#define PERIPH_BASE   0x40000000UL
#define APB1_BASE     (PERIPH_BASE + 0x00000UL)
#define APB2_BASE     (PERIPH_BASE + 0x10000UL)
#define AHB1_BASE     (PERIPH_BASE + 0x20000UL)

/* ---------------------------------------------------------------- *
 * RCC: Reset and Clock Control  (AHB1 + 0x3800)
 * Peripherals are clock-gated OFF at reset to save power. Before you can
 * touch a peripheral's registers you must enable its clock here.
 * ---------------------------------------------------------------- */
typedef struct {
    reg32_t CR;          /* 0x00 */
    reg32_t PLLCFGR;     /* 0x04 */
    reg32_t CFGR;        /* 0x08 */
    reg32_t CIR;         /* 0x0C */
    reg32_t AHB1RSTR;    /* 0x10 */
    reg32_t AHB2RSTR;    /* 0x14 */
    reg32_t AHB3RSTR;    /* 0x18 */
    uint32_t _rsv0;      /* 0x1C */
    reg32_t APB1RSTR;    /* 0x20 */
    reg32_t APB2RSTR;    /* 0x24 */
    uint32_t _rsv1[2];   /* 0x28, 0x2C */
    reg32_t AHB1ENR;     /* 0x30 */
    reg32_t AHB2ENR;     /* 0x34 */
    reg32_t AHB3ENR;     /* 0x38 */
    uint32_t _rsv2;      /* 0x3C */
    reg32_t APB1ENR;     /* 0x40 */
    reg32_t APB2ENR;     /* 0x44 */
} rcc_regs_t;

#define RCC_BASE      (AHB1_BASE + 0x3800UL)
#define RCC           ((rcc_regs_t *)RCC_BASE)

#define RCC_APB1ENR_USART2EN   BIT(17)
#define RCC_APB1ENR_I2C1EN     BIT(21)
#define RCC_APB1RSTR_I2C1RST   BIT(21)

/* ---------------------------------------------------------------- *
 * GPIO  (AHB1 + 0x0000, one block every 0x400 bytes: A, B, C ... H)
 * ---------------------------------------------------------------- */
typedef struct {
    reg32_t MODER;       /* 0x00  mode: 2 bits per pin            */
    reg32_t OTYPER;      /* 0x04  output type: 1 bit per pin      */
    reg32_t OSPEEDR;     /* 0x08  speed: 2 bits per pin           */
    reg32_t PUPDR;       /* 0x0C  pull-up/down: 2 bits per pin    */
    reg32_t IDR;         /* 0x10  input data (read-only)          */
    reg32_t ODR;         /* 0x14  output data                     */
    reg32_t BSRR;        /* 0x18  bit set/reset (write-only)      */
    reg32_t LCKR;        /* 0x1C  configuration lock              */
    reg32_t AFR[2];      /* 0x20 AFRL (pins 0-7), 0x24 AFRH (8-15) */
} gpio_regs_t;

#define GPIOA_BASE    (AHB1_BASE + 0x0000UL)
#define GPIOB_BASE    (AHB1_BASE + 0x0400UL)
#define GPIOC_BASE    (AHB1_BASE + 0x0800UL)
#define GPIOD_BASE    (AHB1_BASE + 0x0C00UL)
#define GPIOE_BASE    (AHB1_BASE + 0x1000UL)
#define GPIOF_BASE    (AHB1_BASE + 0x1400UL)
#define GPIOG_BASE    (AHB1_BASE + 0x1800UL)
#define GPIOH_BASE    (AHB1_BASE + 0x1C00UL)

#define GPIOA         ((gpio_regs_t *)GPIOA_BASE)
#define GPIOB         ((gpio_regs_t *)GPIOB_BASE)
#define GPIOC         ((gpio_regs_t *)GPIOC_BASE)
#define GPIOD         ((gpio_regs_t *)GPIOD_BASE)
#define GPIOE         ((gpio_regs_t *)GPIOE_BASE)
#define GPIOF         ((gpio_regs_t *)GPIOF_BASE)
#define GPIOG         ((gpio_regs_t *)GPIOG_BASE)
#define GPIOH         ((gpio_regs_t *)GPIOH_BASE)

/* ---------------------------------------------------------------- *
 * USART  (USART2 is on APB1 at 0x4000 4400)
 * ---------------------------------------------------------------- */
typedef struct {
    reg32_t SR;          /* 0x00 status                  */
    reg32_t DR;          /* 0x04 data (TX and RX share)  */
    reg32_t BRR;         /* 0x08 baud rate               */
    reg32_t CR1;         /* 0x0C                         */
    reg32_t CR2;         /* 0x10                         */
    reg32_t CR3;         /* 0x14                         */
    reg32_t GTPR;        /* 0x18                         */
} usart_regs_t;

#define USART2_BASE   (APB1_BASE + 0x4400UL)
#define USART2        ((usart_regs_t *)USART2_BASE)

#define USART_SR_PE     BIT(0)   /* parity error            */
#define USART_SR_FE     BIT(1)   /* framing error           */
#define USART_SR_NF     BIT(2)   /* noise detected          */
#define USART_SR_ORE    BIT(3)   /* overrun error           */
#define USART_SR_RXNE   BIT(5)   /* read data register not empty */
#define USART_SR_TC     BIT(6)   /* transmission complete   */
#define USART_SR_TXE    BIT(7)   /* transmit data register empty */

#define USART_CR1_RE      BIT(2)
#define USART_CR1_TE      BIT(3)
#define USART_CR1_RXNEIE  BIT(5)
#define USART_CR1_PS      BIT(9)   /* 0 = even, 1 = odd */
#define USART_CR1_PCE     BIT(10)  /* parity control enable */
#define USART_CR1_M       BIT(12)  /* word length: 1 = 9 bits */
#define USART_CR1_UE      BIT(13)  /* USART enable */

/* ---------------------------------------------------------------- *
 * I2C  (I2C1 is on APB1 at 0x4000 5400)   -- STM32F4-style I2C block
 * ---------------------------------------------------------------- */
typedef struct {
    reg32_t CR1;         /* 0x00 */
    reg32_t CR2;         /* 0x04 */
    reg32_t OAR1;        /* 0x08 */
    reg32_t OAR2;        /* 0x0C */
    reg32_t DR;          /* 0x10 */
    reg32_t SR1;         /* 0x14 */
    reg32_t SR2;         /* 0x18 */
    reg32_t CCR;         /* 0x1C */
    reg32_t TRISE;       /* 0x20 */
    reg32_t FLTR;        /* 0x24 */
} i2c_regs_t;

#define I2C1_BASE     (APB1_BASE + 0x5400UL)
#define I2C1          ((i2c_regs_t *)I2C1_BASE)

#define I2C_CR1_PE      BIT(0)
#define I2C_CR1_START   BIT(8)
#define I2C_CR1_STOP    BIT(9)
#define I2C_CR1_ACK     BIT(10)
#define I2C_CR1_POS     BIT(11)
#define I2C_CR1_SWRST   BIT(15)

#define I2C_SR1_SB      BIT(0)   /* start bit generated      */
#define I2C_SR1_ADDR    BIT(1)   /* address sent / matched   */
#define I2C_SR1_BTF     BIT(2)   /* byte transfer finished   */
#define I2C_SR1_RXNE    BIT(6)
#define I2C_SR1_TXE     BIT(7)
#define I2C_SR1_BERR    BIT(8)   /* bus error                */
#define I2C_SR1_ARLO    BIT(9)   /* arbitration lost         */
#define I2C_SR1_AF      BIT(10)  /* acknowledge failure (NACK) */
#define I2C_SR1_OVR     BIT(11)

#define I2C_SR2_MSL     BIT(0)
#define I2C_SR2_BUSY    BIT(1)

#define I2C_CCR_FS      BIT(15)  /* 1 = fast mode */

/* ---------------------------------------------------------------- *
 * Cortex-M4 core peripherals (ARM, not ST): SysTick and NVIC.
 * These live in the "private peripheral bus" at 0xE000_0000.
 * ---------------------------------------------------------------- */
typedef struct {
    reg32_t CTRL;        /* 0x00 */
    reg32_t LOAD;        /* 0x04 */
    reg32_t VAL;         /* 0x08 */
    reg32_t CALIB;       /* 0x0C */
} systick_regs_t;

#define SYSTICK_BASE  0xE000E010UL
#define SYSTICK       ((systick_regs_t *)SYSTICK_BASE)

#define SYSTICK_CTRL_ENABLE     BIT(0)
#define SYSTICK_CTRL_TICKINT    BIT(1)
#define SYSTICK_CTRL_CLKSOURCE  BIT(2)   /* 1 = processor clock */

#define NVIC_ISER     ((reg32_t *)0xE000E100UL)   /* 8 words: 1 bit per IRQ */
#define USART2_IRQn   38u

/* ---------------------------------------------------------------- *
 * Compile-time proof that our structs match the manual.
 * ---------------------------------------------------------------- */
_Static_assert(offsetof(rcc_regs_t, AHB1ENR)  == 0x30, "RCC AHB1ENR offset");
_Static_assert(offsetof(rcc_regs_t, APB1ENR)  == 0x40, "RCC APB1ENR offset");
_Static_assert(offsetof(rcc_regs_t, APB1RSTR) == 0x20, "RCC APB1RSTR offset");
_Static_assert(offsetof(gpio_regs_t, IDR)     == 0x10, "GPIO IDR offset");
_Static_assert(offsetof(gpio_regs_t, ODR)     == 0x14, "GPIO ODR offset");
_Static_assert(offsetof(gpio_regs_t, BSRR)    == 0x18, "GPIO BSRR offset");
_Static_assert(offsetof(gpio_regs_t, AFR)     == 0x20, "GPIO AFR offset");
_Static_assert(offsetof(usart_regs_t, BRR)    == 0x08, "USART BRR offset");
_Static_assert(offsetof(usart_regs_t, CR1)    == 0x0C, "USART CR1 offset");
_Static_assert(offsetof(i2c_regs_t, DR)       == 0x10, "I2C DR offset");
_Static_assert(offsetof(i2c_regs_t, SR1)      == 0x14, "I2C SR1 offset");
_Static_assert(offsetof(i2c_regs_t, CCR)      == 0x1C, "I2C CCR offset");
_Static_assert(offsetof(i2c_regs_t, TRISE)    == 0x20, "I2C TRISE offset");

#endif /* STM32F446_REGS_H */
