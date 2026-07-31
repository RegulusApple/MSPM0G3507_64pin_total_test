#include "ad9833.h"
#include "ti_msp_dl_config.h"

#define AD9833_MCLK_HZ (25000000UL)
#define AD9833_FREQ_SCALE (268435456UL)

static void AD9833_WaitSpiIdle(void)
{
    while (!DL_SPI_isTXFIFOEmpty(AD9833_SPI_INST) ||
           DL_SPI_isBusy(AD9833_SPI_INST)) {
    }
}

static void AD9833_DrainRx(void)
{
    uint16_t discard;

    while (DL_SPI_receiveDataCheck16(AD9833_SPI_INST, &discard)) {
    }
}

void AD9833_Init(void)
{
    DL_SPI_Config config = {
        .mode = DL_SPI_MODE_CONTROLLER,
        .frameFormat = DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA0,
        .parity = DL_SPI_PARITY_NONE,
        .dataSize = DL_SPI_DATA_SIZE_16,
        .bitOrder = DL_SPI_BIT_ORDER_MSB_FIRST,
        .chipSelectPin = DL_SPI_CHIP_SELECT_0,
    };

    DL_GPIO_setPins(AD9833_FSYNC_PORT, AD9833_FSYNC_PIN);
    DL_GPIO_enableOutput(AD9833_FSYNC_PORT, AD9833_FSYNC_PIN);

    DL_SPI_disable(AD9833_SPI_INST);
    DL_SPI_init(AD9833_SPI_INST, &config);
    DL_SPI_setBitRateSerialClockDivider(AD9833_SPI_INST, 3U);
    DL_SPI_enable(AD9833_SPI_INST);
    AD9833_DrainRx();
}

void AD9833_Send(uint16_t data)
{
    AD9833_WaitSpiIdle();
    AD9833_DrainRx();
    DL_GPIO_clearPins(AD9833_FSYNC_PORT, AD9833_FSYNC_PIN);
    DL_SPI_transmitDataBlocking16(AD9833_SPI_INST, data);
    AD9833_WaitSpiIdle();
    DL_GPIO_setPins(AD9833_FSYNC_PORT, AD9833_FSYNC_PIN);
    AD9833_DrainRx();
}

void AD9833_WaveOut(uint8_t mode, uint32_t frequencyHz, uint16_t phaseDeg)
{
    uint32_t frequencyWord;
    uint16_t phaseWord;
    uint16_t control = 0x2000U;

    if (phaseDeg > 359U) phaseDeg = 359U;
    frequencyWord = (uint32_t) (((uint64_t) frequencyHz * AD9833_FREQ_SCALE) / AD9833_MCLK_HZ);
    phaseWord = (uint16_t) (((uint32_t) phaseDeg * 4096U) / 360U);

    AD9833_Send(0x0100U);
    AD9833_Send(0x2100U);
    AD9833_Send((uint16_t) (0x4000U | (frequencyWord & 0x3FFFU)));
    AD9833_Send((uint16_t) (0x4000U | ((frequencyWord >> 14) & 0x3FFFU)));
    AD9833_Send((uint16_t) (0xC000U | phaseWord));
    if (mode == TRIANGLE_WAVE) control |= 0x0002U;
    if (mode == SQUARE_WAVE) control |= 0x0028U;
    AD9833_Send(control);
}
