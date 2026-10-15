#pragma once

#include "main.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define WATCH_I2C_ADDRESS 0x68

    typedef struct
    {
        uint8_t year;
        uint8_t month;
        uint8_t date;
        uint8_t wday;
        uint8_t hour;
        uint8_t min;
        uint8_t sec;
    } WatchTime;

    HAL_StatusTypeDef watch_init(I2C_HandleTypeDef *hi2c);
    bool watch_read(WatchTime *time);
    void watch_format_time(const WatchTime *time, char *buf, size_t len);
    void watch_format_date(const WatchTime *time, char *buf, size_t len);

#ifdef __cplusplus
}
#endif
