#ifndef USR_FLASH_H_
#define USR_FLASH_H_

#include <stdbool.h>
#include <stdint.h>

#define FLASH_RECORD_MAX_COUNT      (30U)
#define FLASH_RECORD_TEST_COUNT     (5U)

#define FLASH_RECORD_STATUS_NORMAL      (0x00U)
#define FLASH_RECORD_STATUS_UNDERLOAD   (0x01U)
#define FLASH_RECORD_STATUS_OVERLOAD    (0x02U)

typedef struct {
    uint32_t timestamp_s;
    uint16_t meter_id;
    uint16_t meter_addr;
    uint16_t current_ma;
    uint8_t status;
    uint8_t reserved;
    uint16_t crc16;
} MeterRecord;

void Flash_Init(void);
bool Flash_SaveRecords(const MeterRecord *records, uint16_t count);
bool Flash_SaveTestRecords(void);
bool Flash_LoadRecords(MeterRecord *records, uint16_t maxCount, uint16_t *outCount);
bool Flash_IsRecordStorageValid(void);
uint32_t Flash_GetLastImageCrc32(void);
uint32_t Flash_GetStorageBase(void);
uint32_t Flash_GetStorageSize(void);
const char *Flash_RecordStatusToString(uint8_t status);

#endif /* USR_FLASH_H_ */
