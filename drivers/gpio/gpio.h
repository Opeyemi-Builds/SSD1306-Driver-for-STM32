/*
 * gpio.h -- public API of the GPIO driver (WRITTEN BY ME).
 *
 * A pin is identified by (port, pin number). The caller never sees a register.
 */
#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include <stdint.h>
#include "status.h"

typedef enum {
    GPIO_PORT_A = 0, GPIO_PORT_B, GPIO_PORT_C, GPIO_PORT_D,
    GPIO_PORT_E,     GPIO_PORT_F, GPIO_PORT_G, GPIO_PORT_H,
    GPIO_PORT_COUNT
} gpio_port_t;

/* Values equal the bit patterns the MODER register expects. */
typedef enum {
    GPIO_MODE_INPUT     = 0,
    GPIO_MODE_OUTPUT    = 1,
    GPIO_MODE_ALTERNATE = 2,   /* pin driven by a peripheral (UART, I2C...) */
    GPIO_MODE_ANALOG    = 3
} gpio_mode_t;

typedef enum { GPIO_OTYPE_PUSH_PULL = 0, GPIO_OTYPE_OPEN_DRAIN = 1 } gpio_otype_t;
typedef enum { GPIO_SPEED_LOW = 0, GPIO_SPEED_MEDIUM, GPIO_SPEED_FAST, GPIO_SPEED_HIGH } gpio_speed_t;
typedef enum { GPIO_PULL_NONE = 0, GPIO_PULL_UP = 1, GPIO_PULL_DOWN = 2 } gpio_pull_t;

typedef struct {
    gpio_mode_t  mode;
    gpio_otype_t otype;   /* only meaningful for OUTPUT / ALTERNATE */
    gpio_speed_t speed;   /* only meaningful for OUTPUT / ALTERNATE */
    gpio_pull_t  pull;
    uint8_t      af;      /* alternate function number 0..15, ALTERNATE only */
} gpio_config_t;

status_t gpio_init(gpio_port_t port, uint8_t pin, const gpio_config_t *cfg);
status_t gpio_write(gpio_port_t port, uint8_t pin, bool level);
status_t gpio_toggle(gpio_port_t port, uint8_t pin);
status_t gpio_read(gpio_port_t port, uint8_t pin, bool *level);

#endif
