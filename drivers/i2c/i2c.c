
#include "i2c.h"
#include "gpio.h"
#include "timebase.h"
#include "stm32f446_regs.h"
#include "board.h"

#define I2C_SCL_PORT  GPIO_PORT_B
#define I2C_SCL_PIN   8u
#define I2C_SDA_PORT  GPIO_PORT_B
#define I2C_SDA_PIN   9u
#define I2C_AF        4u          /* DS10693 AF table: I2C1_SCL/SDA = AF4 */

static i2c_speed_t s_speed;

/* ----------------------- peripheral configuration ----------------------- */

static void apply_config(void)
{
    const uint32_t pclk_mhz = BOARD_APB1_HZ / 1000000UL;
    const uint32_t f_scl    = (uint32_t)s_speed;

    I2C1->CR1 &= ~I2C_CR1_PE;            /* timing registers need PE = 0 */
    I2C1->CR2 = pclk_mhz & 0x3FUL;       /* FREQ field: APB1 clock in MHz */

    if (s_speed == I2C_SPEED_STANDARD_100K) {
        /* Standard mode: SCL low time = high time = CCR * T_pclk, so
         *   f_scl = f_pclk / (2 * CCR)  =>  CCR = f_pclk / (2 * f_scl)
         * 16 MHz / (2 * 100 kHz) = 80. Round UP so we never exceed f_scl. */
        const uint32_t ccr = (BOARD_APB1_HZ + 2UL * f_scl - 1UL) / (2UL * f_scl);
        I2C1->CCR   = ccr;
        /* Max rise time 1000 ns => TRISE = 1000ns / T_pclk + 1 = MHz + 1 */
        I2C1->TRISE = pclk_mhz + 1UL;
    } else {
        /* Fast mode, DUTY = 0: low = 2 * high, period = 3 * CCR * T_pclk, so
         *   CCR = f_pclk / (3 * f_scl) = 16 MHz / 1.2 MHz = 13.3 -> 14
         * (rounded up => ~380 kHz actual, safely under the 400 kHz limit). */
        const uint32_t ccr = (BOARD_APB1_HZ + 3UL * f_scl - 1UL) / (3UL * f_scl);
        I2C1->CCR   = I2C_CCR_FS | ccr;
        /* Max rise time 300 ns => TRISE = 300ns / T_pclk + 1 */
        I2C1->TRISE = (pclk_mhz * 300UL) / 1000UL + 1UL;
    }

    I2C1->CR1 |= I2C_CR1_PE;
}

static void soft_reset(void)
{
    I2C1->CR1 |=  I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;
    apply_config();
}

status_t i2c_init(i2c_speed_t speed)
{
    if (speed != I2C_SPEED_STANDARD_100K && speed != I2C_SPEED_FAST_400K) {
        return STATUS_ERR_PARAM;
    }
    s_speed = speed;

    /* I2C pins MUST be open-drain: devices only pull the line LOW; pull-up
     * resistors make it HIGH. A push-pull pin would short against a device
     * pulling low. The internal pull-up (~40 kohm) is too weak for reliable
     * 400 kHz; OLED/IMU breakout boards normally have their own 4.7k-10k. */
    const gpio_config_t pin = { GPIO_MODE_ALTERNATE, GPIO_OTYPE_OPEN_DRAIN,
                                GPIO_SPEED_HIGH, GPIO_PULL_UP, I2C_AF };
    status_t st = gpio_init(I2C_SCL_PORT, I2C_SCL_PIN, &pin);
    if (st != STATUS_OK) { return st; }
    st = gpio_init(I2C_SDA_PORT, I2C_SDA_PIN, &pin);
    if (st != STATUS_OK) { return st; }

    RCC->APB1ENR  |= RCC_APB1ENR_I2C1EN;
    (void)RCC->APB1ENR;
    RCC->APB1RSTR |=  RCC_APB1RSTR_I2C1RST;     /* pulse reset: known state */
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    apply_config();
    return STATUS_OK;
}

/* ------------------------- low-level building blocks -------------------- */

/* Wait until any bit in 'mask' is set in SR1, or an error shows up. */
static status_t wait_sr1(uint32_t mask, uint32_t timeout_ms)
{
    const uint32_t start = millis();
    for (;;) {
        const uint32_t sr1 = I2C1->SR1;
        if (sr1 & I2C_SR1_AF)   { return STATUS_ERR_NACK; }
        if (sr1 & I2C_SR1_ARLO) { return STATUS_ERR_ARBITRATION; }
        if (sr1 & I2C_SR1_BERR) { return STATUS_ERR_BUS; }
        if (sr1 & mask)         { return STATUS_OK; }
        if ((uint32_t)(millis() - start) >= timeout_ms) { return STATUS_ERR_TIMEOUT; }
    }
}

/* Common error exit: release the bus, clear sticky flags, restore defaults. */
static status_t fail(status_t st)
{
    I2C1->CR1 |= I2C_CR1_STOP;
    I2C1->SR1 &= ~(I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR | I2C_SR1_OVR); /* rc_w0 bits */
    I2C1->CR1 &= ~I2C_CR1_POS;
    I2C1->CR1 |= I2C_CR1_ACK;
    if (st == STATUS_ERR_TIMEOUT) { soft_reset(); }   /* peripheral may be wedged */
    return st;
}

/* The bus must be idle before we may start. */
static status_t begin_transaction(uint32_t timeout_ms)
{
    const uint32_t start = millis();
    while (I2C1->SR2 & I2C_SR2_BUSY) {
        if ((uint32_t)(millis() - start) >= timeout_ms) {
            soft_reset();
            return STATUS_ERR_BUSY;
        }
    }
    return STATUS_OK;
}

/* Generate START (or repeated START) and wait for SB ("start bit sent"). */
static status_t start_condition(uint32_t timeout_ms)
{
    I2C1->CR1 |= I2C_CR1_START;
    return wait_sr1(I2C_SR1_SB, timeout_ms);
}

/* Clearing ADDR requires reading SR1 and then SR2. */
static void clear_addr(void)
{
    (void)I2C1->SR1;
    (void)I2C1->SR2;
}

static status_t send_byte(uint8_t b, uint32_t timeout_ms)
{
    status_t st = wait_sr1(I2C_SR1_TXE, timeout_ms);   /* DR empty? */
    if (st != STATUS_OK) { return st; }
    I2C1->DR = b;
    return STATUS_OK;
}

/* ------------------------------ transmit path --------------------------- */

static status_t write_common(uint8_t addr7, bool has_reg, uint8_t reg,
                             const uint8_t *data, size_t len, uint32_t t)
{
    status_t st = begin_transaction(t);
    if (st != STATUS_OK) { return st; }

    st = start_condition(t);
    if (st != STATUS_OK) { return fail(st); }

    I2C1->DR = (uint32_t)(addr7 << 1);      /* 7-bit address + R/W = 0 (write) */
    st = wait_sr1(I2C_SR1_ADDR, t);         /* slave ACKed the address */
    if (st != STATUS_OK) { return fail(st); }
    clear_addr();

    if (has_reg) {
        st = send_byte(reg, t);
        if (st != STATUS_OK) { return fail(st); }
    }
    for (size_t i = 0; i < len; i++) {
        st = send_byte(data[i], t);
        if (st != STATUS_OK) { return fail(st); }
    }

    if (has_reg || len > 0u) {
        st = wait_sr1(I2C_SR1_BTF, t);      /* last byte fully shifted out */
        if (st != STATUS_OK) { return fail(st); }
    }

    I2C1->CR1 |= I2C_CR1_STOP;
    return STATUS_OK;
}



/*receive*/
static status_t receive_common(uint8_t addr7, uint8_t *data, size_t len, uint32_t t)
{
    status_t st;

    if (len == 2u) { I2C1->CR1 |= I2C_CR1_POS; }  /* ACK bit controls the NEXT byte */
    I2C1->CR1 |= I2C_CR1_ACK;

    st = start_condition(t);                       /* START or repeated START */
    if (st != STATUS_OK) { return fail(st); }

    I2C1->DR = (uint32_t)((addr7 << 1) | 1u);      /* R/W = 1 (read) */
    st = wait_sr1(I2C_SR1_ADDR, t);
    if (st != STATUS_OK) { return fail(st); }

    if (len == 1u) {
        I2C1->CR1 &= ~I2C_CR1_ACK;                 /* NACK the only byte  */
        clear_addr();
        I2C1->CR1 |= I2C_CR1_STOP;                
        st = wait_sr1(I2C_SR1_RXNE, t);
        if (st != STATUS_OK) { return fail(st); }
        data[0] = (uint8_t)I2C1->DR;

    } else if (len == 2u) {
        clear_addr();
        I2C1->CR1 &= ~I2C_CR1_ACK;                 
        st = wait_sr1(I2C_SR1_BTF, t);            
        if (st != STATUS_OK) { return fail(st); }
        I2C1->CR1 |= I2C_CR1_STOP;
        data[0] = (uint8_t)I2C1->DR;
        data[1] = (uint8_t)I2C1->DR;

    } else {
        clear_addr();
        while (len > 3u) {                        
            st = wait_sr1(I2C_SR1_RXNE, t);
            if (st != STATUS_OK) { return fail(st); }
            *data++ = (uint8_t)I2C1->DR;
            len--;
        }
        
        st = wait_sr1(I2C_SR1_BTF, t);
        if (st != STATUS_OK) { return fail(st); }
        I2C1->CR1 &= ~I2C_CR1_ACK;                 
        *data++ = (uint8_t)I2C1->DR;             
        st = wait_sr1(I2C_SR1_BTF, t);             
        if (st != STATUS_OK) { return fail(st); }
        I2C1->CR1 |= I2C_CR1_STOP;
        *data++ = (uint8_t)I2C1->DR;            
        st = wait_sr1(I2C_SR1_RXNE, t);
        if (st != STATUS_OK) { return fail(st); }
        *data = (uint8_t)I2C1->DR;                 
    }

    I2C1->CR1 &= ~I2C_CR1_POS;                    
    I2C1->CR1 |= I2C_CR1_ACK;
    return STATUS_OK;
}



static bool addr_ok(uint8_t addr7) { return addr7 <= 0x7Fu; }

status_t i2c_write(uint8_t addr7, const uint8_t *data, size_t len, uint32_t t)
{
    if (!addr_ok(addr7) || (data == NULL && len > 0u)) { return STATUS_ERR_PARAM; }
    return write_common(addr7, false, 0u, data, len, t);
}

status_t i2c_mem_write(uint8_t addr7, uint8_t reg, const uint8_t *data, size_t len, uint32_t t)
{
    if (!addr_ok(addr7) || (data == NULL && len > 0u)) { return STATUS_ERR_PARAM; }
    return write_common(addr7, true, reg, data, len, t);
}

status_t i2c_probe(uint8_t addr7, uint32_t t)
{
    if (!addr_ok(addr7)) { return STATUS_ERR_PARAM; }
    return write_common(addr7, false, 0u, NULL, 0u, t);
}

status_t i2c_read(uint8_t addr7, uint8_t *data, size_t len, uint32_t t)
{
    if (!addr_ok(addr7) || data == NULL || len == 0u) { return STATUS_ERR_PARAM; }
    status_t st = begin_transaction(t);
    if (st != STATUS_OK) { return st; }
    return receive_common(addr7, data, len, t);
}

status_t i2c_mem_read(uint8_t addr7, uint8_t reg, uint8_t *data, size_t len, uint32_t t)
{
    if (!addr_ok(addr7) || data == NULL || len == 0u) { return STATUS_ERR_PARAM; }

    status_t st = begin_transaction(t);
    if (st != STATUS_OK) { return st; }

    /* Phase 1: write the register index to the device. */
    st = start_condition(t);
    if (st != STATUS_OK) { return fail(st); }
    I2C1->DR = (uint32_t)(addr7 << 1);
    st = wait_sr1(I2C_SR1_ADDR, t);
    if (st != STATUS_OK) { return fail(st); }
    clear_addr();
    st = send_byte(reg, t);
    if (st != STATUS_OK) { return fail(st); }
    st = wait_sr1(I2C_SR1_BTF, t);
    if (st != STATUS_OK) { return fail(st); }

    /* Phase 2: repeated START (no STOP in between!) and read. The repeated
     * start keeps the bus ours so the device keeps its register pointer. */
    return receive_common(addr7, data, len, t);
}
