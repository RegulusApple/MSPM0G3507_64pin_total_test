#include "flash.h"

#include <stddef.h>
#include <string.h>

#include <ti/driverlib/dl_flashctl.h>

#define FLASH_RECORD_MAGIC          (0x52454330UL) /* "REC0" */
#define FLASH_RECORD_VERSION        (1U)
#define FLASH_RECORD_STORAGE_BASE   (0x0001FC00UL)
#define FLASH_RECORD_STORAGE_SIZE   (DL_FLASHCTL_SECTOR_SIZE)
#define FLASH_RECORD_HEADER_WORDS   (4U)
#define FLASH_RECORD_HEADER_BYTES   (FLASH_RECORD_HEADER_WORDS * sizeof(uint32_t))

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t count;
    uint32_t image_crc32;
    uint32_t write_sequence;
} FlashRecordHeader;

#define FLASH_RECORD_IMAGE_BYTES \
    (sizeof(FlashRecordHeader) + (sizeof(MeterRecord) * FLASH_RECORD_MAX_COUNT))
#define FLASH_RECORD_IMAGE_WORDS \
    ((FLASH_RECORD_IMAGE_BYTES + sizeof(uint32_t) - 1U) / sizeof(uint32_t))
#define FLASH_RECORD_PROGRAM_WORDS \
    ((FLASH_RECORD_IMAGE_WORDS + 1U) & ~1UL)

static uint32_t gWriteSequence;
static uint32_t gLastImageCrc32;

static const MeterRecord gTestRecords[FLASH_RECORD_TEST_COUNT] = {
    { .timestamp_s = 2UL,  .meter_id = 1U, .meter_addr = 0x0001U, .current_ma = 185U,  .status = FLASH_RECORD_STATUS_UNDERLOAD, .reserved = 0U, .crc16 = 0U },
    { .timestamp_s = 4UL,  .meter_id = 1U, .meter_addr = 0x0001U, .current_ma = 520U,  .status = FLASH_RECORD_STATUS_NORMAL,    .reserved = 0U, .crc16 = 0U },
    { .timestamp_s = 6UL,  .meter_id = 1U, .meter_addr = 0x0001U, .current_ma = 998U,  .status = FLASH_RECORD_STATUS_NORMAL,    .reserved = 0U, .crc16 = 0U },
    { .timestamp_s = 8UL,  .meter_id = 1U, .meter_addr = 0x0001U, .current_ma = 1530U, .status = FLASH_RECORD_STATUS_NORMAL,    .reserved = 0U, .crc16 = 0U },
    { .timestamp_s = 10UL, .meter_id = 1U, .meter_addr = 0x0001U, .current_ma = 2150U, .status = FLASH_RECORD_STATUS_OVERLOAD,  .reserved = 0U, .crc16 = 0U },
};

static uint16_t Flash_Crc16Ccitt(const uint8_t *data, uint32_t length)
{
    uint16_t crc = 0xFFFFU;
    uint32_t i;
    uint8_t bit;

    if (data == NULL) {
        return 0U;
    }

    for (i = 0U; i < length; i++) {
        crc ^= (uint16_t) ((uint16_t) data[i] << 8U);
        for (bit = 0U; bit < 8U; bit++) {
            if ((crc & 0x8000U) != 0U) {
                crc = (uint16_t) ((crc << 1U) ^ 0x1021U);
            } else {
                crc = (uint16_t) (crc << 1U);
            }
        }
    }

    return crc;
}

static uint32_t Flash_Crc32(const uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t i;
    uint8_t bit;

    if (data == NULL) {
        return 0UL;
    }

    for (i = 0U; i < length; i++) {
        crc ^= (uint32_t) data[i];
        for (bit = 0U; bit < 8U; bit++) {
            if ((crc & 1UL) != 0UL) {
                crc = (crc >> 1U) ^ 0xEDB88320UL;
            } else {
                crc >>= 1U;
            }
        }
    }

    return ~crc;
}

static MeterRecord Flash_RecordWithCrc(const MeterRecord *record)
{
    MeterRecord copy = *record;

    copy.reserved = 0U;
    copy.crc16 = 0U;
    copy.crc16 = Flash_Crc16Ccitt((const uint8_t *) &copy, sizeof(copy));

    return copy;
}

static bool Flash_IsRecordCrcValid(const MeterRecord *record)
{
    MeterRecord copy;

    if (record == NULL) {
        return false;
    }

    copy = *record;
    copy.crc16 = 0U;

    return (Flash_Crc16Ccitt((const uint8_t *) &copy, sizeof(copy)) == record->crc16);
}

static const FlashRecordHeader *Flash_GetHeader(void)
{
    return (const FlashRecordHeader *) FLASH_RECORD_STORAGE_BASE;
}

static const MeterRecord *Flash_GetRecordArea(void)
{
    return (const MeterRecord *) (FLASH_RECORD_STORAGE_BASE + sizeof(FlashRecordHeader));
}

void Flash_Init(void)
{
    gWriteSequence = 0UL;
    gLastImageCrc32 = Flash_GetHeader()->image_crc32;
}

bool Flash_SaveRecords(const MeterRecord *records, uint16_t count)
{
    uint32_t image[FLASH_RECORD_PROGRAM_WORDS];
    FlashRecordHeader *header = (FlashRecordHeader *) image;
    MeterRecord *recordArea = (MeterRecord *) ((uint8_t *) image + sizeof(FlashRecordHeader));
    uint32_t imageBytes;
    uint32_t i;
    DL_FLASHCTL_COMMAND_STATUS eraseStatus;
    DL_FLASHCTL_COMMAND_STATUS programStatus;
    bool verifyOk;

    if ((records == NULL) || (count == 0U) || (count > FLASH_RECORD_MAX_COUNT)) {
        return false;
    }

    memset(image, 0xFF, sizeof(image));

    header->magic = FLASH_RECORD_MAGIC;
    header->version = FLASH_RECORD_VERSION;
    header->count = count;
    header->image_crc32 = 0UL;
    header->write_sequence = ++gWriteSequence;

    for (i = 0U; i < count; i++) {
        recordArea[i] = Flash_RecordWithCrc(&records[i]);
    }

    imageBytes = (uint32_t) (sizeof(FlashRecordHeader) + (sizeof(MeterRecord) * count));
    header->image_crc32 = Flash_Crc32(
        ((const uint8_t *) image) + FLASH_RECORD_HEADER_BYTES,
        imageBytes - FLASH_RECORD_HEADER_BYTES);

    __disable_irq();

    DL_FlashCTL_executeClearStatus(FLASHCTL);
    DL_FlashCTL_unprotectSector(
        FLASHCTL, FLASH_RECORD_STORAGE_BASE, DL_FLASHCTL_REGION_SELECT_MAIN);
    eraseStatus = DL_FlashCTL_eraseMemoryFromRAM(
        FLASHCTL, FLASH_RECORD_STORAGE_BASE, DL_FLASHCTL_COMMAND_SIZE_SECTOR);

    if (eraseStatus == DL_FLASHCTL_COMMAND_STATUS_PASSED) {
        DL_FlashCTL_executeClearStatus(FLASHCTL);
        programStatus = DL_FlashCTL_programMemoryBlockingFromRAM64WithECCGenerated(
            FLASHCTL,
            FLASH_RECORD_STORAGE_BASE,
            image,
            FLASH_RECORD_PROGRAM_WORDS,
            DL_FLASHCTL_REGION_SELECT_MAIN);
    } else {
        programStatus = DL_FLASHCTL_COMMAND_STATUS_FAILED;
    }

    __enable_irq();

    if (programStatus != DL_FLASHCTL_COMMAND_STATUS_PASSED) {
        return false;
    }

    verifyOk = (memcmp((const void *) FLASH_RECORD_STORAGE_BASE, image, imageBytes) == 0);
    if (verifyOk) {
        gLastImageCrc32 = header->image_crc32;
    }

    return verifyOk;
}

bool Flash_SaveTestRecords(void)
{
    return Flash_SaveRecords(gTestRecords, FLASH_RECORD_TEST_COUNT);
}

bool Flash_LoadRecords(MeterRecord *records, uint16_t maxCount, uint16_t *outCount)
{
    const FlashRecordHeader *header = Flash_GetHeader();
    const MeterRecord *recordArea = Flash_GetRecordArea();
    uint32_t imageBytes;
    uint32_t crc32;
    uint16_t i;

    if ((records == NULL) || (outCount == NULL) || (maxCount == 0U)) {
        return false;
    }

    *outCount = 0U;

    if ((header->magic != FLASH_RECORD_MAGIC) ||
        (header->version != FLASH_RECORD_VERSION) ||
        (header->count == 0U) ||
        (header->count > FLASH_RECORD_MAX_COUNT) ||
        (header->count > maxCount)) {
        return false;
    }

    imageBytes = (uint32_t) (sizeof(FlashRecordHeader) +
        (sizeof(MeterRecord) * header->count));
    crc32 = Flash_Crc32(
        ((const uint8_t *) FLASH_RECORD_STORAGE_BASE) + FLASH_RECORD_HEADER_BYTES,
        imageBytes - FLASH_RECORD_HEADER_BYTES);

    if (crc32 != header->image_crc32) {
        return false;
    }

    for (i = 0U; i < header->count; i++) {
        if (Flash_IsRecordCrcValid(&recordArea[i]) == false) {
            return false;
        }
        records[i] = recordArea[i];
    }

    *outCount = header->count;
    gLastImageCrc32 = header->image_crc32;

    return true;
}

bool Flash_IsRecordStorageValid(void)
{
    MeterRecord scratch[FLASH_RECORD_MAX_COUNT];
    uint16_t count;

    return Flash_LoadRecords(scratch, FLASH_RECORD_MAX_COUNT, &count);
}

uint32_t Flash_GetLastImageCrc32(void)
{
    return gLastImageCrc32;
}

uint32_t Flash_GetStorageBase(void)
{
    return FLASH_RECORD_STORAGE_BASE;
}

uint32_t Flash_GetStorageSize(void)
{
    return FLASH_RECORD_STORAGE_SIZE;
}

const char *Flash_RecordStatusToString(uint8_t status)
{
    switch (status) {
        case FLASH_RECORD_STATUS_UNDERLOAD:
            return "UNDER";
        case FLASH_RECORD_STATUS_OVERLOAD:
            return "OVER";
        case FLASH_RECORD_STATUS_NORMAL:
        default:
            return "NORMAL";
    }
}
