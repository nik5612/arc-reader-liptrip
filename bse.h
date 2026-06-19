#ifndef BSE_ETHORNELL_H
#define BSE_ETHORNELL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
    #endif

    int bse_is_valid(uint8_t * data, uint32_t size);
    int bse_decrypt(uint8_t * crypted);

    #ifdef __cplusplus
}
#endif

#endif
