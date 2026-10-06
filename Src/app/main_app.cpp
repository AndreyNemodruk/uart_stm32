// #include <stdio.h>
// #include <stdbool.h>
// #include <stdlib.h>

// #include "main.h"
// #include "printf/usb_printf.h"

// extern "C" ADC_HandleTypeDef hadc1;
// extern "C" TIM_HandleTypeDef htim2;

// // ==================== ADC ====================

// static uint32_t Read_ADC_Raw()
// {
//     uint32_t raw = 0;

//     HAL_ADC_Start(&hadc1);

//     if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
//     {
//         raw = HAL_ADC_GetValue(&hadc1);
//     }

//     HAL_ADC_Stop(&hadc1);

//     return raw;
// }

// // ==================== MAIN ====================

// extern "C" void main_cpp()
// {
//     // Запускаем PWM TIM2 CH1 → PA0
//     HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

//     printf("\r\n");
//     printf("ADC -> PWM TEST\r\n");
//     printf("PA4 -> ADC -> PWM PA0\r\n");
//     printf("| RAW  | Duty |\r\n");
//     printf("|------|------|\r\n");

//     while (1)
//     {
//         // Читаем потенциометр с PA4
//         uint32_t raw = Read_ADC_Raw();

//         // RAW 0..4095 преобразуем в CCR 0..999
//         uint32_t pulse = (raw * 999UL) / 4095UL;

//         // Меняем заполнение PWM
//         __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
//         if (raw < 50)
//         {
//             raw = 0;
//         }

//         if (raw > 4080)
//         {
//             raw = 4095;
//         }
//         // Рассчитываем заполнение в процентах
//         uint32_t duty = (raw * 100UL) / 4095UL;

//         printf(
//             "| %4lu | %3lu%% |\r\n",
//             raw,
//             duty);

//         HAL_Delay(100);
//     }
// }

#include <stdint.h>

#include "main.h"

extern "C" TIM_HandleTypeDef htim2;

static const uint32_t sineTable[] =
    {
        25, 33, 40, 45, 48,
        49, 48, 45, 40, 33,
        25, 17, 10, 5, 2,
        0, 2, 5, 10, 17};

constexpr uint32_t SINE_POINTS = 20;

extern "C" void main_cpp()
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim2);

    while (1)
    {
        for (uint32_t i = 0; i < SINE_POINTS; i++)
        {
            uint32_t pulse = (sineTable[i] * arr) / 49;

            __HAL_TIM_SET_COMPARE(
                &htim2,
                TIM_CHANNEL_1,
                pulse);

            HAL_Delay(0);
        }
    }
}
