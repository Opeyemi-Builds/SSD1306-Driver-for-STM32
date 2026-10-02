/*
 * gpio.c -- GPIO driver for STM32F446 (WRITTEN BY ME).
 * Reference: RM0390, chapter "General-purpose I/Os (GPIO)".
 *
 * Per-pin configuration lives in registers where each pin owns a slice:
 *   MODER, OSPEEDR, PUPDR : 2 bits per pin  -> slice starts at bit (pin * 2)
 *   OTYPER                : 1 bit  per pin  -> bit (pin)
 *   AFR[0] / AFR[1]       : 4 bits per pin  -> AFR[pin / 8], bit ((pin % 8) * 4)
 */
#include "gpio.h"
#include "stm32f446_regs.h"

static gpio_regs_t *const s_ports[GPIO_PORT_COUNT] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH
};

static bool args_valid(gpio_port_t port, uint8_t pin)
{
    return ((unsigned)port < (unsigned)GPIO_PORT_COUNT) && (pin <= 15u);
}

status_t gpio_init(gpio_port_t port, uint8_t pin, const gpio_config_t *cfg)
{
    if (!args_valid(port, pin) || cfg == NULL || cfg->af > 15u) {
        return STATUS_ERR_PARAM;
    }

    gpio_regs_t *g = s_ports[port];

    /* 1. Un-gate the clock for this port (GPIOA..H = AHB1ENR bits 0..7).
     *    Without this, writes to the port's registers are ignored. */
    RCC->AHB1ENR |= BIT((unsigned)port);
    (void)RCC->AHB1ENR;   /* read-back: guarantees the clock is running
                             before the next register access */

    const uint32_t two_bit_shift = (uint32_t)pin * 2u;
    const uint32_t af_shift      = ((uint32_t)pin & 7u) * 4u;

    /* 2. Configure everything EXCEPT the mode first, so when the pin finally
     *    switches mode it already has the right pull / type / speed / AF
     *    (avoids glitches on the wire). */
    reg_modify(&g->PUPDR,   3UL << two_bit_shift, (uint32_t)cfg->pull  << two_bit_shift);
    reg_modify(&g->OTYPER,  1UL << pin,           (uint32_t)cfg->otype << pin);
    reg_modify(&g->OSPEEDR, 3UL << two_bit_shift, (uint32_t)cfg->speed << two_bit_shift);
    reg_modify(&g->AFR[pin >> 3], 0xFUL << af_shift, (uint32_t)cfg->af << af_shift);

    /* 3. Finally switch the mode. */
    reg_modify(&g->MODER, 3UL << two_bit_shift, (uint32_t)cfg->mode << two_bit_shift);

    return STATUS_OK;
}

status_t gpio_write(gpio_port_t port, uint8_t pin, bool level)
{
    if (!args_valid(port, pin)) { return STATUS_ERR_PARAM; }

    /* BSRR: writing 1 to bit n SETS pin n, writing 1 to bit n+16 RESETS it.
     * Writing 0 does nothing. So this is ONE atomic store -- no read-modify-
     * write, no chance an interrupt corrupts a neighbouring pin. Compare to
     * ODR |= ..., which is read, modify, write (three steps). */
    s_ports[port]->BSRR = level ? BIT(pin) : BIT((unsigned)pin + 16u);
    return STATUS_OK;
}

status_t gpio_toggle(gpio_port_t port, uint8_t pin)
{
    if (!args_valid(port, pin)) { return STATUS_ERR_PARAM; }

    gpio_regs_t *g = s_ports[port];
    const uint32_t mask = BIT(pin);
    const uint32_t odr  = g->ODR;

    /* If the pin is currently 1, write its RESET bit; if 0, its SET bit.
     * Still a single atomic BSRR write. */
    g->BSRR = ((odr & mask) << 16) | (~odr & mask);
    return STATUS_OK;
}

status_t gpio_read(gpio_port_t port, uint8_t pin, bool *level)
{
    if (!args_valid(port, pin) || level == NULL) { return STATUS_ERR_PARAM; }

    /* IDR reflects the actual voltage on the pin (sampled), unlike ODR
     * which is only what we asked the output stage to drive. */
    *level = ((s_ports[port]->IDR >> pin) & 1UL) != 0u;
    return STATUS_OK;
}
