#include "gain_calibration.h"

#include <string.h>

#include "OLED.h"
#include "adc2chSample.h"
#include "key.h"
#include "ti_msp_dl_config.h"

#include <ti/driverlib/dl_flashctl.h>

#define GAIN_FLASH_MAGIC          (0x4741494EUL) /* "GAIN" */
#define GAIN_FLASH_VERSION        (1U)
#define GAIN_FLASH_STORAGE_BASE   (0x0001F800UL)
#define GAIN_FLASH_WORDS          (8U)
#define GAIN_KEY_DELAY_CYCLES     (CPUCLK_FREQ / 20U)

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t gainX100;
    uint32_t crc32;
} GainFlashRecord;

static uint32_t GainCalibration_Crc32(const uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t i;
    uint8_t bit;

    if (data == 0) {
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

static uint32_t GainCalibration_ClampX100(uint32_t gainX100)
{
    if (gainX100 < GAIN_CALIBRATION_MIN_X100) {
        return GAIN_CALIBRATION_MIN_X100;
    }
    if (gainX100 > GAIN_CALIBRATION_MAX_X100) {
        return GAIN_CALIBRATION_MAX_X100;
    }
    return gainX100;
}

static uint32_t GainCalibration_AddX100(uint32_t gainX100, uint32_t step)
{
    if (gainX100 > (GAIN_CALIBRATION_MAX_X100 - step)) {
        return GAIN_CALIBRATION_MAX_X100;
    }
    return gainX100 + step;
}

static uint32_t GainCalibration_SubX100(uint32_t gainX100, uint32_t step)
{
    if (gainX100 < (GAIN_CALIBRATION_MIN_X100 + step)) {
        return GAIN_CALIBRATION_MIN_X100;
    }
    return gainX100 - step;
}

static void GainCalibration_InitKeys(void)
{
    DL_GPIO_initDigitalInputFeatures(KEY_KEY1_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(KEY_KEY2_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(KEY_KEY3_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(KEY_KEY4_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(KEY_KEY5_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
}

static void GainCalibration_DeinitKeys(void)
{
    IOMUX->SECCFG.PINCM[KEY_KEY1_IOMUX] = IOMUX_PINCM_PC_UNCONNECTED;
    IOMUX->SECCFG.PINCM[KEY_KEY2_IOMUX] = IOMUX_PINCM_PC_UNCONNECTED;
    IOMUX->SECCFG.PINCM[KEY_KEY3_IOMUX] = IOMUX_PINCM_PC_UNCONNECTED;
    IOMUX->SECCFG.PINCM[KEY_KEY4_IOMUX] = IOMUX_PINCM_PC_UNCONNECTED;
    IOMUX->SECCFG.PINCM[KEY_KEY5_IOMUX] = IOMUX_PINCM_PC_UNCONNECTED;
}

static bool GainCalibration_IsKeyDown(GPIO_Regs *port, uint32_t pin)
{
    return ((DL_GPIO_readPins(port, pin) & pin) == 0U);
}

static uint8_t GainCalibration_ReadPressedEdges(void)
{
    static uint8_t lastState = 0U;
    uint8_t state = 0U;
    uint8_t edge;

    if (GainCalibration_IsKeyDown(KEY_KEY1_PORT, KEY_KEY1_PIN)) {
        state |= KEY_1;
    }
    if (GainCalibration_IsKeyDown(KEY_KEY2_PORT, KEY_KEY2_PIN)) {
        state |= KEY_2;
    }
    if (GainCalibration_IsKeyDown(KEY_KEY3_PORT, KEY_KEY3_PIN)) {
        state |= KEY_3;
    }
    if (GainCalibration_IsKeyDown(KEY_KEY4_PORT, KEY_KEY4_PIN)) {
        state |= KEY_4;
    }
    if (GainCalibration_IsKeyDown(KEY_KEY5_PORT, KEY_KEY5_PIN)) {
        state |= KEY_5;
    }

    edge = (uint8_t) (state & (uint8_t) (~lastState));
    lastState = state;

    return edge;
}

static void GainCalibration_ShowValue(uint32_t gainX100, bool saved)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "GAIN:");
    OLED_ShowNum(1, 6, gainX100 / 100U, 2);
    OLED_ShowString(1, 8, ".");
    OLED_ShowNum(1, 9, gainX100 % 100U, 2);
    OLED_ShowString(2, 1, "K1:+.01 K2:-.01");
    OLED_ShowString(3, 1, "K3:+.10 K4:-.10");
    if (saved) {
        OLED_ShowString(4, 1, "SAVED");
    } else {
        OLED_ShowString(4, 1, "K5:SAVE");
    }
}

uint32_t GainCalibration_LoadX100(void)
{
    const GainFlashRecord *record = (const GainFlashRecord *) GAIN_FLASH_STORAGE_BASE;
    uint32_t crc;

    if ((record->magic != GAIN_FLASH_MAGIC) ||
        (record->version != GAIN_FLASH_VERSION) ||
        (record->gainX100 < GAIN_CALIBRATION_MIN_X100) ||
        (record->gainX100 > GAIN_CALIBRATION_MAX_X100)) {
        return GAIN_CALIBRATION_DEFAULT_X100;
    }

    crc = GainCalibration_Crc32(
        (const uint8_t *) record,
        sizeof(GainFlashRecord) - sizeof(record->crc32));

    if (crc != record->crc32) {
        return GAIN_CALIBRATION_DEFAULT_X100;
    }

    return record->gainX100;
}

bool GainCalibration_SaveX100(uint32_t gainX100)
{
    uint32_t image[GAIN_FLASH_WORDS];
    GainFlashRecord *record = (GainFlashRecord *) image;
    DL_FLASHCTL_COMMAND_STATUS eraseStatus;
    DL_FLASHCTL_COMMAND_STATUS programStatus;
    bool verifyOk;

    memset(image, 0xFF, sizeof(image));

    record->magic = GAIN_FLASH_MAGIC;
    record->version = GAIN_FLASH_VERSION;
    record->gainX100 = GainCalibration_ClampX100(gainX100);
    record->crc32 = GainCalibration_Crc32(
        (const uint8_t *) record,
        sizeof(GainFlashRecord) - sizeof(record->crc32));

    __disable_irq();

    DL_FlashCTL_executeClearStatus(FLASHCTL);
    DL_FlashCTL_unprotectSector(
        FLASHCTL, GAIN_FLASH_STORAGE_BASE, DL_FLASHCTL_REGION_SELECT_MAIN);
    eraseStatus = DL_FlashCTL_eraseMemoryFromRAM(
        FLASHCTL, GAIN_FLASH_STORAGE_BASE, DL_FLASHCTL_COMMAND_SIZE_SECTOR);

    if (eraseStatus == DL_FLASHCTL_COMMAND_STATUS_PASSED) {
        DL_FlashCTL_executeClearStatus(FLASHCTL);
        programStatus = DL_FlashCTL_programMemoryBlockingFromRAM64WithECCGenerated(
            FLASHCTL,
            GAIN_FLASH_STORAGE_BASE,
            image,
            GAIN_FLASH_WORDS,
            DL_FLASHCTL_REGION_SELECT_MAIN);
    } else {
        programStatus = DL_FLASHCTL_COMMAND_STATUS_FAILED;
    }

    __enable_irq();

    if (programStatus != DL_FLASHCTL_COMMAND_STATUS_PASSED) {
        return false;
    }

    verifyOk = (memcmp((const void *) GAIN_FLASH_STORAGE_BASE,
        image, sizeof(GainFlashRecord)) == 0);

    return verifyOk;
}

float GainCalibration_GetGain(void)
{
    return ((float) GainCalibration_LoadX100()) / 100.0f;
}

void GainCalibration_BootSelect(void)
{
    uint32_t gainX100;
    uint8_t keys;
    bool saved = false;

    GainCalibration_InitKeys();
    delay_cycles(GAIN_KEY_DELAY_CYCLES);

    if (GainCalibration_IsKeyDown(KEY_KEY1_PORT, KEY_KEY1_PIN) == false) {
        gainX100 = GainCalibration_LoadX100();
        ADC12_SetFrontendGain(((float) gainX100) / 100.0f);
        GainCalibration_DeinitKeys();
        return;
    }

    gainX100 = GainCalibration_LoadX100();
    ADC12_SetFrontendGain(((float) gainX100) / 100.0f);
    OLED_DisplayOn();
    GainCalibration_ShowValue(gainX100, saved);
    (void) GainCalibration_ReadPressedEdges();

    while (1) {
        keys = GainCalibration_ReadPressedEdges();

        if ((keys & KEY_1) != 0U) {
            gainX100 = GainCalibration_AddX100(gainX100, 1U);
            saved = false;
        }
        if ((keys & KEY_2) != 0U) {
            gainX100 = GainCalibration_SubX100(gainX100, 1U);
            saved = false;
        }
        if ((keys & KEY_3) != 0U) {
            gainX100 = GainCalibration_AddX100(gainX100, 10U);
            saved = false;
        }
        if ((keys & KEY_4) != 0U) {
            gainX100 = GainCalibration_SubX100(gainX100, 10U);
            saved = false;
        }
        if ((keys & KEY_5) != 0U) {
            saved = GainCalibration_SaveX100(gainX100);
        }

        if (keys != 0U) {
            ADC12_SetFrontendGain(((float) gainX100) / 100.0f);
            GainCalibration_ShowValue(gainX100, saved);
        }

        delay_cycles(GAIN_KEY_DELAY_CYCLES);
    }
}
