#include "pid_controller.h"

#include <stddef.h>

static float PID_Clamp(float value, float minimum, float maximum)
{
    if (value > maximum) {
        return maximum;
    }
    if (value < minimum) {
        return minimum;
    }
    return value;
}

void PID_Init(PID_Controller *pid, float kp, float ki, float kd,
    float outputMin, float outputMax)
{
    if (pid == NULL) {
        return;
    }
    if (outputMin > outputMax) {
        float temporary = outputMin;
        outputMin = outputMax;
        outputMax = temporary;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->outputMin = outputMin;
    pid->outputMax = outputMax;
    PID_Reset(pid);
}

float PID_Update(PID_Controller *pid, float target,
    float feedback, float dtSeconds)
{
    float error;
    float derivative = 0.0f;
    float candidateIntegral;
    float output;

    if ((pid == NULL) || (dtSeconds <= 0.0f)) {
        return 0.0f;
    }

    error = target - feedback;
    candidateIntegral = pid->integral + (error * dtSeconds);
    if (pid->initialized != 0U) {
        derivative = (error - pid->previousError) / dtSeconds;
    }

    output = (pid->kp * error) + (pid->ki * candidateIntegral) +
        (pid->kd * derivative);

    /* Conditional integration prevents wind-up while saturated. */
    if (((output <= pid->outputMax) && (output >= pid->outputMin)) ||
        ((output > pid->outputMax) && (error < 0.0f)) ||
        ((output < pid->outputMin) && (error > 0.0f))) {
        pid->integral = candidateIntegral;
    }

    output = (pid->kp * error) + (pid->ki * pid->integral) +
        (pid->kd * derivative);
    pid->previousError = error;
    pid->initialized = 1U;
    return PID_Clamp(output, pid->outputMin, pid->outputMax);
}

void PID_Reset(PID_Controller *pid)
{
    if (pid == NULL) {
        return;
    }
    pid->integral = 0.0f;
    pid->previousError = 0.0f;
    pid->initialized = 0U;
}

float Control_SlewLimit(float target, float previous, float maxChange)
{
    if (maxChange < 0.0f) {
        maxChange = -maxChange;
    }
    if (target > (previous + maxChange)) {
        return previous + maxChange;
    }
    if (target < (previous - maxChange)) {
        return previous - maxChange;
    }
    return target;
}
