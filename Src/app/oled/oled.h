#pragma once

#include "main.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define OLED_I2C_ADDRESS 0x3C

    HAL_StatusTypeDef oled_init(I2C_HandleTypeDef *hi2c);
    void oled_fill(uint8_t value);
    void oled_draw_clock(const char *time_str, const char *date_str);

#ifdef __cplusplus
}
#endif
