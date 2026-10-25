#pragma once

#include "main.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define AT24C32_ADDRESS 0x50
#define AT24C32_PAGE_SIZE 32U
#define AT24C32_SIZE 4096U

    bool at24c32_init(I2C_HandleTypeDef *hi2c);
    bool at24c32_wait_ready(void);
    bool at24c32_write(uint16_t address, const uint8_t *data, size_t length);
    bool at24c32_read(uint16_t address, uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif
