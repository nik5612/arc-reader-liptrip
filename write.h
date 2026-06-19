#ifndef WRITE_ETHORNELL_H
#define WRITE_ETHORNELL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
    #endif

    int write_RGBA_to_png(uint16_t width, uint16_t height, uint8_t * array, const char * filename);

    #ifdef __cplusplus
}
#endif

#endif
