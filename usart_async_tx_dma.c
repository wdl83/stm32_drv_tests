#include <stddef.h>

#include <drv/stm32_dma.h>
#include <drv/stm32_flash.h>
#include <drv/stm32_gpio.h>
#include <drv/stm32_nvic.h>
#include <drv/stm32_rcc.h>
#include <drv/stm32_usart.h>
#include <printf.h>

#define USART_TX_DMA_CHANNEL                                                   4


char buf[64];
uint32_t tx_cntr = 0;

void tx_complete(void)
{
    const int len =
        snprintf(buf, sizeof(buf), "Hello USART TX DMA %010u\n", tx_cntr);
    ++tx_cntr;

    DMA_CCR_t ccr =
    {
        .bits.EN = 1,
        .bits.TCIE = 1,
        .bits.TEIE = 1,
        .bits.DIR = 1,
        .bits.MINC = 1,
        .bits.PSIZE = DMA_WIDTH8,
        .bits.MSIZE = DMA_WIDTH8,
        .bits.PL = DMA_PRIORITY_LOW
    };

    DMA_PADDR(DMA1_BASE, USART_TX_DMA_CHANNEL, &USART_DR(USART1_BASE));
    DMA_MADDR(DMA1_BASE, USART_TX_DMA_CHANNEL, buf);
    DMA_TRANSFER_NUM(DMA1_BASE, USART_TX_DMA_CHANNEL, len);
    USART_TX_COMPLETE_CLEAR(USART1_BASE);
    DMA_CCR(DMA1_BASE, USART_TX_DMA_CHANNEL) = ccr.u32;
}

void isrDMA1_Channel4(void)
{
    DMA_DISABLE(DMA1_BASE, USART_TX_DMA_CHANNEL);
    DMA_COMPLETE_INT_CLEAR(DMA1_BASE, USART_TX_DMA_CHANNEL);
    DMA_ERROR_INT_CLEAR(DMA1_BASE, USART_TX_DMA_CHANNEL);
    tx_complete();
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

    /* DMA1 */
    DMA1_CLK_ENABLE();

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
    USART_TX_DMA_ENABLE(USART1_BASE);
    USART_TX_ENABLE(USART1_BASE);
    ISR_ENABLE(isrNoDMA1_Channel4);
    tx_complete();

    for(;;) {}
}
