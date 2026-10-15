#include "watch.h"

#include <stdio.h>

#define WATCH_I2C_TIMEOUT_MS 100
#define DS1307_REG_START 0x00

static I2C_HandleTypeDef *watch_i2c;

static const char *const k_wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static uint8_t bcd2bin(uint8_t v)
{
    return (uint8_t)((v >> 4) * 10u + (v & 0x0Fu));
}

static HAL_StatusTypeDef ds1307_read_regs(uint8_t reg, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Read(watch_i2c, (uint16_t)(WATCH_I2C_ADDRESS << 1), reg,
                            I2C_MEMADD_SIZE_8BIT, data, len, WATCH_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef ds1307_write_regs(uint8_t reg, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Write(watch_i2c, (uint16_t)(WATCH_I2C_ADDRESS << 1), reg,
                             I2C_MEMADD_SIZE_8BIT, data, len, WATCH_I2C_TIMEOUT_MS);
}

static const char *watch_wday_str(uint8_t wday)
{
    if (wday >= 1u && wday <= 7u)
    {
        return k_wday[wday - 1u];
    }
    return "---";
}

static HAL_StatusTypeDef ds1307_start(void)
{
    uint8_t sec = 0;

    if (ds1307_read_regs(0x00, &sec, 1) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if ((sec & 0x80u) == 0u)
    {
        return HAL_OK;
    }

    sec &= 0x7Fu;
    return ds1307_write_regs(0x00, &sec, 1);
}

HAL_StatusTypeDef watch_init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
    {
        return HAL_ERROR;
    }

    watch_i2c = hi2c;
    return ds1307_start();
}

bool watch_read(WatchTime *time)
{
    uint8_t regs[7];
    uint8_t hour_reg;
    uint8_t hour;

    if (time == NULL || watch_i2c == NULL)
    {
        return false;
    }

    if (ds1307_read_regs(DS1307_REG_START, regs, sizeof(regs)) != HAL_OK)
    {
        return false;
    }

    if (regs[0] & 0x80u)
    {
        return false;
    }

    hour_reg = regs[2];
    if (hour_reg & 0x40u)
    {
        hour = bcd2bin(hour_reg & 0x1Fu);
        if ((hour_reg & 0x20u) && hour != 12u)
        {
            hour = (uint8_t)(hour + 12u);
        }
        else if (((hour_reg & 0x20u) == 0u) && hour == 12u)
        {
            hour = 0;
        }
    }
    else
    {
        hour = bcd2bin(hour_reg & 0x3Fu);
    }

    time->sec = bcd2bin(regs[0] & 0x7Fu);
    time->min = bcd2bin(regs[1] & 0x7Fu);
    time->hour = hour;
    time->wday = (uint8_t)(regs[3] & 0x07u);
    time->date = bcd2bin(regs[4] & 0x3Fu);
    time->month = bcd2bin(regs[5] & 0x1Fu);
    time->year = bcd2bin(regs[6]);
    return true;
}

void watch_format_time(const WatchTime *time, char *buf, size_t len)
{
    snprintf(buf, len, "%02u:%02u:%02u", time->hour, time->min, time->sec);
}

void watch_format_date(const WatchTime *time, char *buf, size_t len)
{
    snprintf(buf, len, "%s %02u.%02u.20%02u",
             watch_wday_str(time->wday), time->date, time->month, time->year);
}
