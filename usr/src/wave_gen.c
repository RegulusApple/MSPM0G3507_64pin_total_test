#include "wave_gen.h"

#include <stddef.h>

#include "ad9833.h"
#include "ad9850.h"
#include "board_config.h"
#include "dac.h"

static WaveConfig gWaveConfig;
static bool gWaveRunning;

static uint8_t WaveGen_ToAd9833Mode(WaveType type)
{
    if (type == WAVE_TRIANGLE) {
        return TRIANGLE_WAVE;
    }
    if (type == WAVE_SQUARE) {
        return SQUARE_WAVE;
    }
    return SINE_WAVE;
}

bool WaveGen_Start(const WaveConfig *config)
{
    if ((config == NULL) || (config->frequencyHz < 0.0f) ||
        (config->amplitudeMv < 0.0f) || (config->offsetMv < 0.0f) ||
        (config->phaseDeg < 0.0f) || (config->phaseDeg > 360.0f)) {
        return false;
    }

    if (config->source == WAVE_SOURCE_DAC) {
        DAC_Init();
        if (config->type == WAVE_DC) {
            if (config->offsetMv > 65535.0f) {
                return false;
            }
            DAC_SetVoltageMv((uint16_t) config->offsetMv);
        } else if (config->type == WAVE_SINE) {
            if ((config->frequencyHz < 1.0f) ||
                (config->frequencyHz > 4294967295.0f) ||
                (config->amplitudeMv > 65535.0f)) {
                return false;
            }
            DAC_SetSineAmplitudeMv((uint16_t) config->amplitudeMv);
            if (DAC_StartSineHz((uint32_t) config->frequencyHz) == false) {
                return false;
            }
        } else {
            return false;
        }
    } else if (config->source == WAVE_SOURCE_AD9833) {
        if ((config->type == WAVE_DC) || (config->frequencyHz < 1.0f) ||
            (config->frequencyHz > 4294967295.0f)) {
            return false;
        }
        /* AD9833 amplitude and offset require external analog hardware. */
        if ((config->amplitudeMv != 0.0f) || (config->offsetMv != 0.0f)) {
            return false;
        }
        if ((AD9833_Init() == false) ||
            (AD9833_WaveOut(WaveGen_ToAd9833Mode(config->type),
                (uint32_t) config->frequencyHz,
                (uint16_t) config->phaseDeg) == false)) {
            return false;
        }
    } else if (config->source == WAVE_SOURCE_AD9850) {
        if (BOARD_AD9850_AVAILABLE == 0U) {
            return false;
        }
        /* AD9850 outputs sine (IOUT) and square (QOUT via comparator);
         * triangle needs external shaping.  Amplitude/offset are analog. */
        if ((config->type != WAVE_SINE) && (config->type != WAVE_SQUARE)) {
            return false;
        }
        if ((config->frequencyHz < 1.0f) ||
            (config->frequencyHz > 4294967295.0f) ||
            (config->phaseDeg < 0.0f) || (config->phaseDeg > 360.0f)) {
            return false;
        }
        if ((config->amplitudeMv != 0.0f) || (config->offsetMv != 0.0f)) {
            return false;
        }
        if ((AD9850_Init() == false) ||
            (AD9850_SetFrequencyPhaseHz((uint32_t) config->frequencyHz,
                (uint16_t) config->phaseDeg) == false)) {
            return false;
        }
    } else {
        return false;
    }

    gWaveConfig = *config;
    gWaveRunning = true;
    return true;
}

bool WaveGen_SetFrequency(float frequencyHz)
{
    WaveConfig config;

    if ((gWaveRunning == false) || (frequencyHz < 1.0f)) {
        return false;
    }
    config = gWaveConfig;
    config.frequencyHz = frequencyHz;
    return WaveGen_Start(&config);
}

bool WaveGen_SetAmplitude(float amplitudeMv)
{
    WaveConfig config;

    if ((gWaveRunning == false) ||
        (gWaveConfig.source != WAVE_SOURCE_DAC) ||
        (gWaveConfig.type != WAVE_SINE) || (amplitudeMv < 0.0f)) {
        return false;
    }
    config = gWaveConfig;
    config.amplitudeMv = amplitudeMv;
    return WaveGen_Start(&config);
}

bool WaveGen_SetOffset(float offsetMv)
{
    WaveConfig config;

    if ((gWaveRunning == false) ||
        (gWaveConfig.source != WAVE_SOURCE_DAC) ||
        (gWaveConfig.type != WAVE_DC) || (offsetMv < 0.0f)) {
        return false;
    }
    config = gWaveConfig;
    config.offsetMv = offsetMv;
    return WaveGen_Start(&config);
}

void WaveGen_Stop(void)
{
    if (gWaveRunning && (gWaveConfig.source == WAVE_SOURCE_DAC)) {
        DAC_StopSine();
        DAC_SetCode12(0U);
    } else if (gWaveRunning && (gWaveConfig.source == WAVE_SOURCE_AD9850)) {
        (void) AD9850_PowerDown();
    }
    gWaveRunning = false;
}

bool WaveGen_IsRunning(void)
{
    return gWaveRunning;
}

bool WaveGen_GetConfig(WaveConfig *config)
{
    if (config == NULL) {
        return false;
    }
    *config = gWaveConfig;
    return true;
}
