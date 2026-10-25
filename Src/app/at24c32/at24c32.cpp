#include "at24c32.h"

#define AT24C32_I2C_TIMEOUT_MS 100
#define AT24C32_READY_TIMEOUT_MS 20

static I2C_HandleTypeDef *at24_i2c;

bool at24c32_wait_ready(void)
{
    uint32_t start;

    if (at24_i2c == NULL)
    {
        return false;
    }

    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < AT24C32_READY_TIMEOUT_MS)
    {
        if (HAL_I2C_IsDeviceReady(at24_i2c, (uint16_t)(AT24C32_ADDRESS << 1), 1,
                                  AT24C32_I2C_TIMEOUT_MS) == HAL_OK)
        {
            return true;
        }
    }

    return false;
}

bool at24c32_init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
    {
        return false;
    }

    at24_i2c = hi2c;
    return at24c32_wait_ready();
}

bool at24c32_write(uint16_t address, const uint8_t *data, size_t length)
{
    if ((at24_i2c == NULL) || (data == NULL) || (length == 0U) ||
        (length > AT24C32_PAGE_SIZE) ||
        ((address % AT24C32_PAGE_SIZE) != 0U) ||
        ((uint32_t)address + length > AT24C32_SIZE))
    {
        return false;
    }

    if (HAL_I2C_Mem_Write(at24_i2c, (uint16_t)(AT24C32_ADDRESS << 1), address,
                          I2C_MEMADD_SIZE_16BIT, (uint8_t *)data, (uint16_t)length,
                          AT24C32_I2C_TIMEOUT_MS) != HAL_OK)
    {
        return false;
    }

    return at24c32_wait_ready();
}

bool at24c32_read(uint16_t address, uint8_t *data, size_t length)
{
    if ((at24_i2c == NULL) || (data == NULL) || (length == 0U) ||
        ((uint32_t)address + length > AT24C32_SIZE))
    {
        return false;
    }

    return HAL_I2C_Mem_Read(at24_i2c, (uint16_t)(AT24C32_ADDRESS << 1), address,
                            I2C_MEMADD_SIZE_16BIT, data, (uint16_t)length,
                            AT24C32_I2C_TIMEOUT_MS) == HAL_OK;
}
