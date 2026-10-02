/* status.h -- WRITTEN BY ME. One error vocabulary shared by every driver. */
#ifndef STATUS_H
#define STATUS_H

typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_PARAM,        /* caller passed something invalid          */
    STATUS_ERR_TIMEOUT,      /* hardware did not respond in time         */
    STATUS_ERR_BUSY,         /* peripheral/bus busy or stuck             */
    STATUS_ERR_NACK,         /* I2C: nobody acknowledged                 */
    STATUS_ERR_BUS,          /* I2C: misplaced START/STOP on the bus     */
    STATUS_ERR_ARBITRATION,  /* I2C: lost arbitration to another master  */
    STATUS_ERR_OVERRUN,      /* UART: data arrived before we read last   */
    STATUS_ERR_FRAMING,      /* UART: stop bit not seen where expected   */
    STATUS_ERR_NOISE,        /* UART: noise detected on line             */
    STATUS_ERR_PARITY,       /* UART: parity mismatch                    */
    STATUS_ERR_NO_DATA,      /* nothing available (non-blocking read)    */
    STATUS_ERR_DEVICE        /* device answered, but not as expected     */
} status_t;

/* Human-readable name, handy for printing over UART. */
const char *status_str(status_t s);

#endif
