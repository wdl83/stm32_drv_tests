#include <stddef.h>

#include <drv/stm32_flash.h>
#include <drv/stm32_gpio.h>
#include <drv/stm32_rcc.h>
#include <drv/stm32_usart.h>
#include <drv/usart_tx.h>


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

    for(;;)
    {
        usart_send_str(USART1_BASE, "Hello world!\n");
    }
}
