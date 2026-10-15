#include <stdio.h>

#include "main.h"
#include "oled/oled.h"
#include "watch/watch.h"

extern I2C_HandleTypeDef hi2c1;

extern "C" void main_cpp()
{
    HAL_Delay(2000);

    bool oled_ok = (oled_init(&hi2c1) == HAL_OK);
    
    if (oled_ok)
    {
        oled_fill(0x00);
        printf("[OLED] init OK\r\n");
    }
    else
    {
        printf("[OLED] init failed\r\n");
    }

    bool watch_ok = (watch_init(&hi2c1) == HAL_OK);
    if (watch_ok)
    {
        printf("[RTC] init OK\r\n");
    }
    else
    {
        printf("[RTC] init failed\r\n");
    }

    uint32_t rtc_tick = 0;

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

        printf("%s %s\r\n", date_str, time_str);

        if (oled_ok)
        {
            oled_draw_clock(time_str, date_str);
        }
    }
}
