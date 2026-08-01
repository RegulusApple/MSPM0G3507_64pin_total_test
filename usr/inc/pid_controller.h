#ifndef PID_CONTROLLER_H_
#define PID_CONTROLLER_H_

#include <stdint.h>

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float previousError;
    float outputMin;
    float outputMax;
    uint8_t initialized;
} PID_Controller;

void PID_Init(PID_Controller *pid, float kp, float ki, float kd,
    float outputMin, float outputMax);
float PID_Update(PID_Controller *pid, float target,
    float feedback, float dtSeconds);
void PID_Reset(PID_Controller *pid);
float Control_SlewLimit(float target, float previous, float maxChange);

#endif /* PID_CONTROLLER_H_ */
