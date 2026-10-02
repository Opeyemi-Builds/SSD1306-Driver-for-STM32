/*
 * timebase.c -- WRITTEN BY ME.
 * Uses the Cortex-M SysTick down-counter: it counts from LOAD to 0, raises
 * the SysTick exception, reloads, repeats. Every 16000 clocks @16 MHz = 1 ms.
 */
#include "timebase.h"
#include "stm32f446_regs.h"
#include "board.h"

/* Written by the ISR, read by main code -> must be volatile. A 32-bit
 * aligned read is a single instruction on Cortex-M, so no tearing. */
static volatile uint32_t s_ms;

void SysTick_Handler(void)
{
    s_ms++;
}

void timebase_init(void)
{
    SYSTICK->LOAD = (BOARD_SYSCLK_HZ / 1000UL) - 1UL;  /* N+1 clocks per period */
    SYSTICK->VAL  = 0;
    SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_ENABLE;
}

uint32_t millis(void)
{
    return s_ms;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while ((uint32_t)(millis() - start) < ms) { /* spin */ }
}
