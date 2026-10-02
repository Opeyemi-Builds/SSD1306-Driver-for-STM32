/*
 * i2c.h -- I2C master driver API (WRITTEN BY ME). Drives I2C1 on PB8 (SCL) /
 * PB9 (SDA), Arduino connector pins D15 / D14 on the NUCLEO-F446RE.
 *
 * Addresses are 7-bit (0x00..0x7F), WITHOUT the R/W bit; the driver adds it.
 * 'timeout_ms' bounds EACH individual wait inside a transaction (not the
 * whole transaction), so long transfers do not need huge timeouts.
 */
#ifndef I2C_H
#define I2C_H

#include <stddef.h>
#include <stdint.h>
#include "status.h"

#define I2C_DEFAULT_TIMEOUT_MS  50u

typedef enum {
    I2C_SPEED_STANDARD_100K = 100000,
    I2C_SPEED_FAST_400K     = 400000
} i2c_speed_t;

status_t i2c_init(i2c_speed_t speed);

/* Raw transactions */
status_t i2c_write(uint8_t addr7, const uint8_t *data, size_t len, uint32_t timeout_ms);
status_t i2c_read (uint8_t addr7, uint8_t *data, size_t len, uint32_t timeout_ms);

/* "Register" style transactions used by most sensors/displays:
 *   mem_write: START | addr+W | reg | data... | STOP
 *   mem_read : START | addr+W | reg | RESTART | addr+R | data... | STOP  */
status_t i2c_mem_write(uint8_t addr7, uint8_t reg, const uint8_t *data, size_t len, uint32_t timeout_ms);
status_t i2c_mem_read (uint8_t addr7, uint8_t reg, uint8_t *data, size_t len, uint32_t timeout_ms);

/* Address-only transaction: STATUS_OK if a device ACKs, STATUS_ERR_NACK if not. */
status_t i2c_probe(uint8_t addr7, uint32_t timeout_ms);

#endif
