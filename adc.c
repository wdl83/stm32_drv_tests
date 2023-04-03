#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

#include <drv/stm32_adc.h>
#include <drv/stm32_flash.h>
#include <drv/stm32_gpio.h>
#include <drv/stm32_rcc.h>
#include <drv/stm32_usart.h>
#include <drv/usart_tx.h>
#include <drv/util.h>
#include "printf/printf.h"


__attribute__((noreturn))
void main(void)
{
    /* Bluepill STM32F103C8T6:
     *  - core max_freq: 72MHz
     *  - ext. crystal :  8MHz
     *  - APB2 max freq: 72MHz (high-speed APB)
     *  - APB1 max freq: 36MHz (low-speed APB)
     *  - PoR clock src:  8MHz (HSI)
     *  - USART TX1    : PA9
     *  - USART RX1    : PA10 */

    HSE_ENABLE();
    /* wait for HSE ready */
    while(!HSE_READY()) {}
    /* PLL multiplier 72MHz / 8MHz = 9 */
    PLL_MUL_9();
    /* HSI as PLL source */
    PLL_SRC_HSE();
    /* prescale APB1 source freq. 72MHz / 2 = 36MHz (max) */
    APB1_CLK_DIV2();
    /* flash wait state */
    FLASH_SYSCLK_48_72MHz();
    PLL_ENABLE();
    /* wait for PLL ready */
    while(!PLL_READY()) {}
    /* switch SYSCLK source to PLL */
    SYSCLK_SRC_PLL();
    /* wait for SYSCLK to switch */
    while(!IS_SYSCLK_SRC_PLL()) {}

    /* USART1 config */

    PORTA_CLK_ENABLE();
    USART1_CLK_ENABLE();
    /* PoR: alternate functions are not active on I/O ports
     * enable USART TX/RX  */
    GPIO_CFG(GPIOA_BASE, 9, CFN_OUT_ALT_PUSH_PULL, MODE_50MHz);
    GPIO_CFG(GPIOA_BASE, 10, CFN_IN_PULL_UP_DOWN, MODE_INPUT);

    /* USART1 uses APB2 clk as source */
    USART_ENABLE(USART1_BASE);
    USART_BR(USART1_BASE, CALC_BR(APB2_CLK, UINT32_C(9600)));
    USART_TX_ENABLE(USART1_BASE);

    /* ADC1 config
     * ADC_CLK: 14MHz max
     * prescaler: APB2_CLK / PRESCALER (72MHz / 6 = 12MHz ~83.33ns) */
    ADC_CLK_DIV6();
    ADC1_CLK_ENABLE();
    ADC_ENABLE(ADC1_BASE);
    ADC_CALIBRATION(ADC1_BASE);
    while(!ADC_CALIBRATION_DONE(ADC1_BASE)) {}

    /* enable temp. sensor (IN16) and Vrefint (IN17) */
    ADC_IN16_IN17_ENABLE(ADC1_BASE);
    /* read internal temp. sensor (recommended sampling time 17.1us)
     * 17100ns / 83.33ns ~= 205
     * 205 - 12.5 (HW const) ~= 192.5 (closest available 239.5) */
    ADC_CH16_SAMPLE_TIME(ADC1_BASE, ADC_SAMPLE_TIME_239t5);
    ADC_SEQ1(ADC1_BASE, 16);

    char buf[32];

    for(;;)
    {
        ADC_TRIGGER(ADC1_BASE);
        while(!ADC_COMPLETE(ADC1_BASE)) {}

        /* tempC = ((V25 - Vsense) / Avg_slope) + 25
         * V25 = 1.43, Avg_Slope = 4.3 */
        const uint16_t value = ADC_DR(ADC1_BASE);
        const uint32_t value_mV = (value * UINT32_C(3300)) / (UINT32_C(1) << 12);
        snprintf(buf, sizeof(buf), "%5u %5umV\n", value, value_mV);
        usart_send_str(USART1_BASE, buf);
    }
}
