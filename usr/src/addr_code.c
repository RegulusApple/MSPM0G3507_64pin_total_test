#include "addr_code.h"
#include "ti_msp_dl_config.h"
#include "usr_uart.h"

uint8_t AddrCode_Read(void)
{
    uint8_t addrCode = 0U;

    if ((DL_GPIO_readPins(ADDR_CODE_ADDR0_PORT, ADDR_CODE_ADDR0_PIN) & ADDR_CODE_ADDR0_PIN) != 0U) {
        addrCode |= ADDR_CODE_BIT_A27;
    }
    if ((DL_GPIO_readPins(ADDR_CODE_ADDR1_PORT, ADDR_CODE_ADDR1_PIN) & ADDR_CODE_ADDR1_PIN) != 0U) {
        addrCode |= ADDR_CODE_BIT_A25;
    }
    if ((DL_GPIO_readPins(ADDR_CODE_ADDR2_PORT, ADDR_CODE_ADDR2_PIN) & ADDR_CODE_ADDR2_PIN) != 0U) {
        addrCode |= ADDR_CODE_BIT_B24;
    }
    if ((DL_GPIO_readPins(ADDR_CODE_ADDR3_PORT, ADDR_CODE_ADDR3_PIN) & ADDR_CODE_ADDR3_PIN) != 0U) {
        addrCode |= ADDR_CODE_BIT_B18;
    }

    return addrCode;
}

void AddrCode_Print(const char *tag, uint8_t addrCode)
{
    if (tag == 0) {
        tag = "ADDR_CODE";
    }

    USR_UART_printf("%s,ADDR=0x%02X,A27=%u,A25=%u,B24=%u,B18=%u\r\n",
        tag,
        addrCode,
        (addrCode & ADDR_CODE_BIT_A27) ? 1U : 0U,
        (addrCode & ADDR_CODE_BIT_A25) ? 1U : 0U,
        (addrCode & ADDR_CODE_BIT_B24) ? 1U : 0U,
        (addrCode & ADDR_CODE_BIT_B18) ? 1U : 0U);
}
