#include "main.h"

#define LED_PIN (1u << 8)
#define BTN_PIN (1u << 7)

static const uint32_t k_periods_ms[] = {100u, 200u, 500u, 1000u};

extern "C" void main_cpp()
{
    uint32_t period_i = 0;
    uint32_t last_blink = 0;
    uint32_t last_btn = 0;
    bool btn_was = false;

    while (1)
    {
        uint32_t now = uwTick;
        bool btn = ((GPIOB->IDR & BTN_PIN) == 0u);

        if (btn && !btn_was && ((now - last_btn) > 20u))
        {
            period_i++;
            if (period_i >= 4u)
            {
                period_i = 0;
            }
            last_btn = now;
        }
        btn_was = btn;

        if ((now - last_blink) >= k_periods_ms[period_i])
        {
            last_blink = now;
            uint32_t odr = GPIOB->ODR;
            GPIOB->BSRR = ((odr & LED_PIN) << 16u) | (~odr & LED_PIN);
        }
    }
}
