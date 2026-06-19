#ifndef CBG_ETHORNELL_H
#define CBG_ETHORNELL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
    #endif

    int cbg_is_valid(uint8_t * data, uint32_t size);
    uint8_t * cbg_decrypt(uint8_t * crypted, uint16_t * pwidth, uint16_t * pheight);
    int cbg_save(uint8_t * data, uint32_t width, uint32_t height, const char * filename);

    #ifdef __cplusplus
}
#endif

#endif
