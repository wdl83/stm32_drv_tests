#include <stddef.h>

#include "libc.h"

#include <drv/stm32_flash.h>
#include <drv/stm32_nvic.h>
#include <drv/stm32_rcc.h>
#include <drv/stm32_stk.h>
#include <drv/tlog.h>

#include <modbus_c/crc.h>
#include <modbus_c/rtu.h>
#include <modbus_c/rtu_memory.h>
#include <modbus_c/stm32f103c8/rtu_impl.h>

typedef union
{
    struct
    {
        uint8_t RnW     : 1;
        /* set to request io operation */
        uint8_t PENDING : 1;
        /* set when request was served */
        uint8_t READY   : 1;
        uint8_t         : 1;
        uint8_t OR      : 1;
        uint8_t AND     : 1;
        uint8_t         : 2;
    } bits;

    struct
    {
        uint8_t : 4;
        uint8_t op: 2;
        uint8_t : 2;
    } opcode;
} io_mode_t;

/* RTU ABI */
typedef struct
{
    /*------------------------------------------------------------------------*/
    // offsets are calculated relative to header end               offset (size)
    rtu_memory_header_t header;
    /*------------------------------------------------------------------------*/
    /* calculated CRC16 checksum from current FLASH content
     * recalculated on every boot                                             */
    uint16_t fw_crc16;                                                 //  0 (2)
    uint16_t reserved0;                                                //  2 (2)
    /*------------------------------------------------------------------------*/
    /* I/O on any address
     * io_mode:
     *     RnW: 0    *io_addr  =   io_data
     *     RnW: 1     io_data  =  *io_addr
     *     OR:  1    *io_addr |=   io_data
     *     AND: 1    *io_addr &=   io_data
     * io_addr: address of io operation
     * io_data: data read or to be written */
    uint32_t io_addr;                                                  //  4 (4)
    io_mode_t io_mode;                                                 //  8 (1)
    uint8_t io_data;                                                   //  9 (1)
    uint16_t reserved1;                                                // 10 (2)
    /*------------------------------------------------------------------------*/
    /* Trouble/Trace Log
     * keep it as last member so its size wont affect RTU MEM ABI             */
    char tlog[TLOG_SIZE];                                               // 12 ()
} rtu_memory_fields_t;

/* FLASH layout: [.text|.rodata|.data] */
static
const uint8_t *fw_begin()
{
    extern uintptr_t _text_begin;
    return (const uint8_t *)&_text_begin;
}

static
const uint8_t *fw_end()
{
    extern uintptr_t _text_end;
    extern uintptr_t _data_size;
    return (const uint8_t *)&_text_end +  (uint32_t)&_data_size;
}

uint16_t calc_fw_checksum(void)
{
    uint16_t crc16 = UINT16_C(0xFFFF);
    const uint8_t *begin = fw_begin();
    const uint8_t *const end = fw_end();

    while(begin != end) crc16 = crc16_update(crc16, *begin++);
    return crc16;
}

void rtu_memory_fields_clear(rtu_memory_fields_t *mem)
{
    bzero(mem, sizeof(rtu_memory_fields_t));
}

void rtu_memory_fields_init(rtu_memory_fields_t *mem)
{
    mem->header.addr_begin = RTU_ADDR_BASE;
    mem->header.addr_end =
        RTU_ADDR_BASE + sizeof(rtu_memory_fields_t) - sizeof(rtu_memory_t);
    mem->fw_crc16 = calc_fw_checksum();
}

uint8_t *rtu_pdu_cb(
    modbus_rtu_state_t *state,
    modbus_rtu_addr_t addr,
    modbus_rtu_fcode_t fcode,
    const uint8_t *begin, const uint8_t *end,
    /* curr == begin + sizeof(addr_t) + sizeof(fcode_t) */
    const uint8_t *curr,
    uint8_t *dst_begin, const uint8_t *const dst_end,
    uintptr_t user_data)
{
    rtu_memory_fields_t *mem = (rtu_memory_fields_t *)user_data;

    TLOG_XPRINT16("S|F", ((uint16_t)addr << 8) | fcode);

    /* because crossing rtu_err_reboot_threashold will cause
     * reboot decrese error count if valid PDU received */
    if(state->stats.err_cntr) --state->stats.err_cntr;

    if(modbus_rtu_addr(state) != addr) goto exit;

    *dst_begin++ = addr;

    dst_begin =
        rtu_memory_pdu_cb(
            (rtu_memory_t *)&mem->header,
            fcode,
            begin + sizeof(addr), end,
            curr,
            dst_begin, dst_end);
exit:
    return dst_begin;
}

/* PIT: periodic interrupt */
typedef struct
{
    uint16_t cntr;
    /* main event loop should clear 'updated' bit on every PIT interrupt
     * if its busy with modbus events - count number of skipped updates */
    uint16_t skip_cntr;

    union
    {
        uint8_t value;
        struct
        {
            uint8_t updated : 1;
            uint8_t : 7;
        } bits;
    } status;
} pit_t;

static
void periodic_timer_cb(pit_t *pit)
{
    pit->skip_cntr += pit->status.bits.updated;
    ++pit->cntr;
    pit->status.bits.updated = 1;
    /* log every 10ms * 64K ~ 655s ~11min */
    if(!pit->cntr) TLOG_XPRINT16("PIT", pit->skip_cntr);
}

static
void periodic_timer_init(void)
{
    /* STK freq: 72MHz / 8 = 9MHz, range [0, 2^24) = [0, 16M)
     * 1 / 9MHz * 9^4 = 10ms */
    STK_CLK_SRC_AHB_DIV8();
    STK_SET(UINT32_C(90000));
    STK_INT_ENABLE();
    STK_ENABLE();
}

static
void handle_io(rtu_memory_fields_t *mem)
{
    if(!mem->io_mode.bits.PENDING) return;

    uint8_t *data = (uint8_t *)mem->io_addr;

    if(mem->io_mode.bits.RnW) mem->io_data = *data;
    else if(mem->io_mode.bits.OR) *data |= mem->io_data;
    else if(mem->io_mode.bits.AND) *data &= mem->io_data;
    mem->io_mode.bits.PENDING = 0;
    mem->io_mode.bits.READY = 1;

    const char *mode[] = {"RD", "W|", "W&"};
    TLOG_XPRINT16("IO", (uint32_t)data);
    TLOG_XPRINT8(mode[mem->io_mode.opcode.op], *data);
}

static
void exec(rtu_memory_fields_t *mem, pit_t *pit)
{
    pit->status.bits.updated = 0;
    handle_io(mem);
}

rtu_memory_fields_t mem_;
modbus_rtu_state_t state_;
pit_t pit_;

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

    rtu_memory_fields_clear(&mem_);
    rtu_memory_fields_init(&mem_);
    tlog_init(mem_.tlog);
    TLOG_XPRINT16("TLOG", offsetof(rtu_memory_fields_t, tlog));
    TLOG_XPRINT16("FWCRC", mem_.fw_crc16);

    modbus_rtu_impl(&state_, RTU_ADDR, NULL, NULL, rtu_pdu_cb, (uintptr_t)&mem_);
    periodic_timer_init();

    for(;;)
    {
        INTERRUPT_DISABLE();
        modbus_rtu_event(&state_);
        if(modbus_rtu_idle(&state_)) exec(&mem_, &pit_);
        INTERRUPT_ENABLE();
        WAIT_FOR_INTERRUPT();
    }
}
/*----------------------------------------------------------------------------*/
void isrSysTick(void)
{
    periodic_timer_cb(&pit_);
}
