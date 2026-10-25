#include <stdio.h>

#include "main.h"
#include "at24c32/at24c32.h"
#include "logger/logger.h"
#include "oled/oled.h"
#include "watch/watch.h"

extern I2C_HandleTypeDef hi2c1;

extern "C" void main_cpp()
{
    char buf[LOGGER_SLOT_SIZE];
    bool oled_ok = false;
    bool watch_ok = false;
    uint32_t rtc_tick = 0;

    HAL_Delay(2000);

    if (at24c32_init(&hi2c1) && logger_init())
    {
        printf("[EEPROM] AT24C32 0x50 ready\r\n");
        logger_log("Info: boot");
        printf("[LOG] count=%u first=#%u last=#%u\r\n",
               logger_get_count(), logger_get_first(), logger_get_last());
        logger_read(logger_get_last(), buf);
    }
    else
    {
        printf("[EEPROM] AT24C32 not ready\r\n");
    }

    oled_ok = (oled_init(&hi2c1) == HAL_OK);
    if (oled_ok)
    {
        oled_fill(0x00);
        printf("[OLED] init OK\r\n");
    }
    else
    {
        printf("[OLED] init failed\r\n");
    }

    watch_ok = (watch_init(&hi2c1) == HAL_OK);
    if (watch_ok)
    {
        printf("[RTC] init OK\r\n");
    }
    else
    {
        printf("[RTC] init failed\r\n");
    }

    while (1)
    {
        char time_str[16];
        char date_str[20];
        WatchTime time;

        if (!watch_ok)
        {
            continue;
        }

        uint32_t now = HAL_GetTick();
        if ((now - rtc_tick) < 1000u)
        {
            continue;
        }
        rtc_tick = now;

        if (!watch_read(&time))
        {
            printf("[RTC] Read failed\r\n");
            continue;
        }

        watch_format_time(&time, time_str, sizeof(time_str));
        watch_format_date(&time, date_str, sizeof(date_str));

        // printf("%s %s\r\n", date_str, time_str);

        char log_msg[32];
        snprintf(log_msg, sizeof(log_msg), "%s %s", date_str, time_str);
        logger_log(log_msg);

        logger_read(logger_get_last(), buf);

        if (oled_ok)
        {
            oled_draw_clock(time_str, date_str);
        }
    }
}
