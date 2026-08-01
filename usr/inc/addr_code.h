#ifndef ADDR_CODE_H_
#define ADDR_CODE_H_

#include <stdint.h>

#define ADDR_CODE_BIT_A27  (0x01U)
#define ADDR_CODE_BIT_A25  (0x02U)
#define ADDR_CODE_BIT_B24  (0x04U)
#define ADDR_CODE_BIT_B18  (0x08U)

uint8_t AddrCode_Read(void);
void AddrCode_Print(const char *tag, uint8_t addrCode);

#endif
