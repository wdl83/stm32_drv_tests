#include <stddef.h>

#include <drv/stm32_flash.h>
#include <drv/stm32_gpio.h>
#include <drv/stm32_nvic.h>
#include <drv/stm32_rcc.h>
#include <drv/stm32_tim.h>
#include <drv/stm32_usart.h>
#include <drv/usart_async_tx.h>

#include "libc.h"

#ifndef CPU_CLK_VALUE
#error "Please define CPU_CLK_VALUE"
#endif

#define CPU_CLK                                          UINT32_C(CPU_CLK_VALUE)
#define APB1_CLK                                                  (CPU_CLK >> 1)
#define APB2_CLK                                                         CPU_CLK

usart_tx_ctrl_t tx_ctrl_;

char buf[32] = "Hello world!\n";

void tx_complete(uintptr_t base, usart_tx_ctrl_t *ctrl)
{
    ASSERT(ctrl);
    bzero(ctrl, sizeof(usart_tx_ctrl_t));
}

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
     * enable USART TX/RX
     *
     * 9.1.4 Alternate functions (AF)
     * "For alternate function inputs, the port must be configured in Input mode
     * (floating, pull-up or pull-down) and the input pin must be driven externally */
    GPIO_CFG(GPIOA_BASE, 9, CFN_OUT_ALT_PUSH_PULL, MODE_50MHz);
    GPIO_CFG(GPIOA_BASE, 10, CFN_IN_PULL_UP_DOWN, MODE_INPUT);
    GPIO_PULL_UP(GPIOA_BASE, 10);

    /* USART1 uses APB2 clk as source */
    USART_ENABLE(USART1_BASE);
    USART_BR(USART1_BASE, CALC_BR(APB2_CLK, UINT32_C(9600)));
    USART_TX_ENABLE(USART1_BASE);

    /* TIM2 uses APB1 clk as source */
    /* 36MHz = 36 * 10^6Hz / 65K = 549.3Hz (T = 1.82ms)
     * 36MHz / 36000 = 1kHz (T = 1ms) */
    TIM2_CLK_ENABLE();
    TIM_AUTO_RELOAD_PRELOAD_ENABLE(TIM2_BASE);
    TIM_CLK_DIV(TIM2_BASE, UINT16_C(35999));
    TIM_WR_TARGET(TIM2_BASE, UINT16_C(1000)); // every 1s
    TIM_WR_CNTR(TIM2_BASE, UINT16_C(0));
    TIM_UPDATE_INT_ENABLE(TIM2_BASE);
    TIM_UPDATE_INT_CLEAR(TIM2_BASE);
    TIM_ENABLE(TIM2_BASE);

    ISR_ENABLE(isrNoUSART1);
    ISR_ENABLE(isrNoTIM2);

    for(;;){}
}
/*----------------------------------------------------------------------------*/
void isrUSART1(void)
{
    usart_tx_isr(USART1_BASE, &tx_ctrl_);
}
/*----------------------------------------------------------------------------*/
void isrTIM2(void)
{
    TIM_UPDATE_INT_CLEAR(TIM2_BASE);

    if(NULL != tx_ctrl_.begin) return;

    tx_ctrl_.begin = (uint8_t *)buf;
    tx_ctrl_.end = (const uint8_t *)(buf + sizeof(buf));
    tx_ctrl_.complete_cb = tx_complete;
    usart_async_send(USART1_BASE, &tx_ctrl_);
}
/*----------------------------------------------------------------------------*/
