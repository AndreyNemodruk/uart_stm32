#include <stdint.h>
#include <string.h>

#include "main.h"

extern UART_HandleTypeDef huart1;

#define COMMAND_SIZE 64
#define BTN_PORT GPIOB
#define BTN_PIN GPIO_PIN_7
#define LED_PORT GPIOB
#define LED_PIN GPIO_PIN_8
#define DEBOUNCE_MS 50U

static char rxBuffer[COMMAND_SIZE];
static uint8_t rxPos = 0;
static bool ledRequestedOn = false;

static void led_set(bool on)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void uart_send(const char *text)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)text, strlen(text), 100);
}

static int uart_read_bytes(uint8_t *dst, uint16_t len)
{
    return HAL_UART_Receive(&huart1, dst, len, 10) == HAL_OK ? (int)len : 0;
}

static void handle_command(const char *command)
{
    if (strcmp(command, "LED_ON") == 0)
    {
        led_set(true);
    }
    else if (strcmp(command, "LED_OFF") == 0)
    {
        led_set(false);
    }
}

static void on_button_short_press(void)
{
    ledRequestedOn = !ledRequestedOn;
    uart_send(ledRequestedOn ? "LED_ON\r\n" : "LED_OFF\r\n");
}

static void btn_poll(void)
{
    static GPIO_PinState btn_raw = GPIO_PIN_SET;
    static GPIO_PinState btn_stable = GPIO_PIN_SET;
    static uint32_t btn_tick;

    GPIO_PinState btn = HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN);
    if (btn != btn_raw)
    {
        btn_raw = btn;
        btn_tick = HAL_GetTick();
    }
    else if ((HAL_GetTick() - btn_tick) >= DEBOUNCE_MS && btn != btn_stable)
    {
        btn_stable = btn;
        if (btn == GPIO_PIN_RESET)
        {
            on_button_short_press();
        }
    }
}

static void uart_poll(void)
{
    uint8_t rx;
    int len = uart_read_bytes(&rx, 1);

    if (len <= 0)
    {
        return;
    }

    if (rx == '\r' || rx == '\n')
    {
        if (rxPos > 0)
        {
            rxBuffer[rxPos] = '\0';
            handle_command(rxBuffer);
            rxPos = 0;
        }
        return;
    }

    if (rxPos < sizeof(rxBuffer) - 1)
    {
        rxBuffer[rxPos++] = (char)rx;
    }
}

extern "C" void main_cpp()
{
    while (1)
    {
        btn_poll();
        uart_poll();
    }
}
