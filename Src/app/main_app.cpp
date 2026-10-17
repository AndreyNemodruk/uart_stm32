#include <stdio.h>

#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;

#define ADC_RANK_COUNT 1

static uint32_t adc_dma_buf[ADC_RANK_COUNT];
static char uart_tx_buf[48];

static void uart_send_adc(uint32_t raw, uint32_t mv)
{
    int n;

    if (huart1.gState != HAL_UART_STATE_READY)
    {
        return;
    }

    n = snprintf(uart_tx_buf, sizeof(uart_tx_buf), "ADC %lu  %lu mV\r\n", raw, mv);
    if (n <= 0)
    {
        return;
    }

    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)uart_tx_buf, (uint16_t)n);
}

extern "C" void main_cpp()
{
    HAL_Delay(2000);

    if (huart1.hdmatx == NULL)
    {
        printf("[UART] TX DMA not linked. Add USART1_TX DMA in Cube.\r\n");
        return;
    }

    if (HAL_ADC_Start_DMA(&hadc1, adc_dma_buf, ADC_RANK_COUNT) != HAL_OK)
    {
        printf("[ADC] DMA start failed\r\n");
        return;
    }

    while (1)
    {
        uint32_t raw = adc_dma_buf[0] & 0xFFFu;
        uint32_t mv = (raw * 3300u) / 4095u;

        uart_send_adc(raw, mv);
        HAL_Delay(200);
    }
}
