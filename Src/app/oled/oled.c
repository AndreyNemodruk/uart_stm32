#include "oled.h"

#include "ssd1306.h"
#include "ssd1306_fonts.h"

#include <string.h>

HAL_StatusTypeDef oled_init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
    {
        return HAL_ERROR;
    }

    ssd1306_Init();
    return HAL_OK;
}

void oled_fill(uint8_t value)
{
    ssd1306_Fill(value ? White : Black);
    ssd1306_UpdateScreen();
}

static uint8_t str_x_centered(const char *s, SSD1306_Font_t font)
{
    int width = (int)strlen(s) * (int)font.width;
    int x = ((int)SSD1306_WIDTH - width) / 2;
    if (x < 0)
    {
        x = 0;
    }
    return (uint8_t)x;
}

void oled_draw_clock(const char *time_str, const char *date_str)
{
    ssd1306_Fill(Black);

    ssd1306_SetCursor(str_x_centered(time_str, Font_11x18), 12);
    ssd1306_WriteString((char *)time_str, Font_11x18, White);

    ssd1306_SetCursor(str_x_centered(date_str, Font_7x10), 44);
    ssd1306_WriteString((char *)date_str, Font_7x10, White);

    ssd1306_UpdateScreen();
}
