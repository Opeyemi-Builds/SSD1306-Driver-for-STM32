#ifndef TIMEBASE_H
#define TIMEBASE_H
#include <stdint.h>

/* Start the SysTick timer so it fires an interrupt every 1 ms. */
void     timebase_init(void);
/* Milliseconds since timebase_init(). Wraps after ~49 days; always compare
 * with (uint32_t)(now - start) >= duration, which is wrap-safe. */
uint32_t millis(void);
/* Busy-wait for 'ms' milliseconds. */
void     delay_ms(uint32_t ms);

#endif
