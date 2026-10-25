#pragma once

#include "main.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LOGGER_SLOT_SIZE 32U
#define LOGGER_SLOT_COUNT 128U
#define LOGGER_SEQ_MOD 1000U

    bool logger_init(void);
    bool logger_log(const char *message);
    bool logger_read(uint16_t number, char *buffer);
    bool logger_read_range(uint16_t first, uint16_t last);
    uint16_t logger_get_count(void);
    uint16_t logger_get_first(void);
    uint16_t logger_get_last(void);

#ifdef __cplusplus
}
#endif
