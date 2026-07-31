#ifndef USR_UART_H_
#define USR_UART_H_

#include <stdbool.h>
#include <stdint.h>

#include "ti_msp_dl_config.h"

#define USR_UART_RX_BUFFER_SIZE (128U)

void USR_UART_init(void);
void USR_UART_sendByte(uint8_t data);
void USR_UART_sendBytes(const uint8_t *data, uint16_t length);
void USR_UART_sendString(const char *str);

/* 轻量级 UART 格式化输出。
 * 支持：%d %i %u %x %X %08X %s %c %% %f %lf %.Nf
 * 浮点小数位最多 6 位，不支持 %8.3f 这类宽度格式。
 * 这样可以避免拉入标准 printf/vsnprintf，降低代码体积并减少下载调试问题。
 */
int USR_UART_printf(const char *format, ...);
bool USR_UART_readByte(uint8_t *data);
bool USR_UART_readLine(char *line, uint16_t maxLength);
void USR_UART_process(void);
void USR_UART_IRQHandler(void);

#endif /* USR_UART_H_ */


