#include "uart_debug.h"

static void uart_debugPutChar(uint8_t ch)
{
    DL_UART_transmitDataBlocking(UART_DEBUG_INST, ch);
}

int fputc(int ch, FILE *f)
{
    (void) f;

    if (ch == '\n') {
        uart_debugPutChar((uint8_t) '\r');
    }
    uart_debugPutChar((uint8_t) ch);

    return ch;
}

int fputs(const char *str, FILE *stream)
{
    uint16_t i;
    uint16_t len;

    (void) stream;

    len = (uint16_t) strlen(str);
    for (i = 0U; i < len; i++) {
        (void) fputc(str[i], stream);
    }

    return len;
}

void uart_debugInit(void)
{
    /* UART0 is initialized by SYSCFG_DL_init via Sysclk_Init. */
}
