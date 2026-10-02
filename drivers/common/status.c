#include "status.h"

const char *status_str(status_t s)
{
    switch (s) {
    case STATUS_OK:              return "OK";
    case STATUS_ERR_PARAM:       return "ERR_PARAM";
    case STATUS_ERR_TIMEOUT:     return "ERR_TIMEOUT";
    case STATUS_ERR_BUSY:        return "ERR_BUSY";
    case STATUS_ERR_NACK:        return "ERR_NACK";
    case STATUS_ERR_BUS:         return "ERR_BUS";
    case STATUS_ERR_ARBITRATION: return "ERR_ARBITRATION";
    case STATUS_ERR_OVERRUN:     return "ERR_OVERRUN";
    case STATUS_ERR_FRAMING:     return "ERR_FRAMING";
    case STATUS_ERR_NOISE:       return "ERR_NOISE";
    case STATUS_ERR_PARITY:      return "ERR_PARITY";
    case STATUS_ERR_NO_DATA:     return "ERR_NO_DATA";
    case STATUS_ERR_DEVICE:      return "ERR_DEVICE";
    default:                     return "ERR_UNKNOWN";
    }
}
