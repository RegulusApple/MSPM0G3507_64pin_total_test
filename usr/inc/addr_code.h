#ifndef ADDR_CODE_H_
#define ADDR_CODE_H_

#include <stdint.h>

#define ADDR_CODE_BIT_A27  ADDR_CODE_ADDR0_BIT
#define ADDR_CODE_BIT_A25  ADDR_CODE_ADDR1_BIT
#define ADDR_CODE_BIT_B24  ADDR_CODE_ADDR2_BIT
#define ADDR_CODE_BIT_B18  ADDR_CODE_ADDR3_BIT

uint8_t AddrCode_Read(void);
void AddrCode_Print(const char *tag, uint8_t addrCode);

#endif
