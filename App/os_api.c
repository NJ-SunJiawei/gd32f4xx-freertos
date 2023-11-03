#include "os_api.h"

OS_STATUS memory_compare(uint8_t *src, uint8_t *dst, uint16_t length)
{
    while(length --) {
        if(*src++ != *dst++) {
            return OS_ERROR;
        }
    }
    return OS_SUCCESS;
}
