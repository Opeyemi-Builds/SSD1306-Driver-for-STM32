/*
 * startup.c -- WRITTEN BY ME.
 *
 * What runs between "power on" and main():
 *   1. The core loads SP from vector[0] and jumps to vector[1] (Reset_Handler).
 *   2. Reset_Handler copies .data from flash to RAM, zeroes .bss, calls main().
 *
 * The vector table here is intentionally SHORT: it stops after USART2 (IRQ 38).
 * Interrupts we never enable can never fire, so later entries are not needed.
 * If you add a peripheral with a higher IRQ number, extend the table.
 */
#include <stdint.h>

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

#define WEAK_ALIAS __attribute__((weak, alias("Default_Handler")))
void NMI_Handler(void)        WEAK_ALIAS;
void HardFault_Handler(void)  WEAK_ALIAS;
void MemManage_Handler(void)  WEAK_ALIAS;
void BusFault_Handler(void)   WEAK_ALIAS;
void UsageFault_Handler(void) WEAK_ALIAS;
void SVC_Handler(void)        WEAK_ALIAS;
void DebugMon_Handler(void)   WEAK_ALIAS;
void PendSV_Handler(void)     WEAK_ALIAS;
void SysTick_Handler(void)    WEAK_ALIAS;   /* overridden in timebase.c   */
void USART2_IRQHandler(void)  WEAK_ALIAS;   /* overridden in uart.c       */

typedef void (*vector_t)(void);

__attribute__((section(".isr_vector"), used))
const vector_t g_vector_table[55] = {
    [0]  = (vector_t)&_estack,     /* initial stack pointer */
    [1]  = Reset_Handler,
    [2]  = NMI_Handler,
    [3]  = HardFault_Handler,
    [4]  = MemManage_Handler,
    [5]  = BusFault_Handler,
    [6]  = UsageFault_Handler,
    [11] = SVC_Handler,
    [12] = DebugMon_Handler,
    [14] = PendSV_Handler,
    [15] = SysTick_Handler,
    /* device interrupts start at index 16 (IRQ n -> index 16 + n) */
    [16 ... 53] = Default_Handler,
    [54] = USART2_IRQHandler,      /* IRQ 38 */
};

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) { *dst++ = *src++; }

    for (dst = &_sbss; dst < &_ebss; ) { *dst++ = 0u; }

    (void)main();
    for (;;) { }
}

/* Any unexpected exception lands here: park so a debugger can inspect it. */
void Default_Handler(void)
{
    for (;;) { }
}
