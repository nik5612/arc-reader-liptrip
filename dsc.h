#ifndef DSC_ETHORNELL_H
#define DSC_ETHORNELL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
    #endif

    int dsc_is_valid(uint8_t * data, uint32_t size);
    uint8_t * dsc_decrypt(uint8_t * crypted, uint32_t crypted_size, uint32_t * decrypted_size);
    int dsc_save(uint8_t * data, uint32_t size, const char * filename);

    #ifdef __cplusplus
}
#endif

#endif
