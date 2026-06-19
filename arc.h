#ifndef ARC_ETHORNELL_H
#define ARC_ETHORNELL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
    #endif

    struct Arc;

    struct Arc * arc_open(const char * filename);
    void arc_close(struct Arc * arc);
    uint32_t arc_files_count(struct Arc * arc);
    uint8_t * arc_get_file_data(struct Arc * arc, uint32_t idx);
    uint32_t arc_get_file_size(struct Arc * arc, uint32_t idx);
    char * arc_get_file_name(struct Arc * arc, uint32_t idx);

    #ifdef __cplusplus
}
#endif

#endif
