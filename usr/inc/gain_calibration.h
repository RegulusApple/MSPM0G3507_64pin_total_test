#ifndef GAIN_CALIBRATION_H_
#define GAIN_CALIBRATION_H_

#include <stdbool.h>
#include <stdint.h>

#define GAIN_CALIBRATION_DEFAULT_X100   (556U)
#define GAIN_CALIBRATION_MIN_X100       (100U)
#define GAIN_CALIBRATION_MAX_X100       (2000U)

void GainCalibration_BootSelect(void);
uint32_t GainCalibration_LoadX100(void);
bool GainCalibration_SaveX100(uint32_t gainX100);
float GainCalibration_GetGain(void);

#endif /* GAIN_CALIBRATION_H_ */
