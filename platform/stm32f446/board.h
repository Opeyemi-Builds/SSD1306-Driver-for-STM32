/*
 * board.h -- WRITTEN BY ME. Facts about the NUCLEO-F446RE board.
 * Keep board-specific choices here so drivers stay reusable.
 */
#ifndef BOARD_H
#define BOARD_H

/* After reset the MCU runs from the internal 16 MHz RC oscillator (HSI).
 * We never touch the PLL, so every bus runs at 16 MHz. Simple on purpose:
 * all timing maths in the drivers can be checked by hand. */
#define BOARD_SYSCLK_HZ   16000000UL
#define BOARD_APB1_HZ     16000000UL
#define BOARD_APB2_HZ     16000000UL

/* LD2 (green user LED) is wired to PA5 on Nucleo-64 boards. */
#define BOARD_LED_PORT       GPIO_PORT_A
#define BOARD_LED_PIN        5u

/* B1 (blue user button) is wired to PC13, active LOW (external pull-up). */
#define BOARD_BUTTON_PORT    GPIO_PORT_C
#define BOARD_BUTTON_PIN     13u

#endif
