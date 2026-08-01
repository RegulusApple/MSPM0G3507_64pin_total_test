#include "contest_selftest.h"

#include <math.h>
#include <stdbool.h>

#include "analog_calibration.h"
#include "pid_controller.h"
#include "serial_protocol.h"
#include "signalProcess.h"
#include "usr_uart.h"

#define SELFTEST_SAMPLE_COUNT       (1024U)
#define SELFTEST_SAMPLE_RATE_HZ     (12800.0f)
#define SELFTEST_SIGNAL_HZ          (50.0f)
#define SELFTEST_PHASE_DEG          (30.0f)
#define SELFTEST_OFFSET_CODE        (2048.0f)
#define SELFTEST_AMPLITUDE_A_CODE   (1000.0f)
#define SELFTEST_AMPLITUDE_B_CODE   (800.0f)

static uint16_t gSelfTestChannelA[SELFTEST_SAMPLE_COUNT];
static uint16_t gSelfTestChannelB[SELFTEST_SAMPLE_COUNT];

static bool SelfTest_IsNear(float actual, float expected, float tolerance)
{
    return fabsf(actual - expected) <= tolerance;
}

static void SelfTest_GenerateSignals(void)
{
    uint16_t i;
    float phaseRad = SELFTEST_PHASE_DEG * PI / 180.0f;

    for (i = 0U; i < SELFTEST_SAMPLE_COUNT; i++) {
        float angle = 2.0f * PI * SELFTEST_SIGNAL_HZ *
            (float) i / SELFTEST_SAMPLE_RATE_HZ;
        float a = SELFTEST_OFFSET_CODE +
            SELFTEST_AMPLITUDE_A_CODE * sinf(angle);
        float b = SELFTEST_OFFSET_CODE +
            SELFTEST_AMPLITUDE_B_CODE * sinf(angle + phaseRad);

        gSelfTestChannelA[i] = (uint16_t) (a + 0.5f);
        gSelfTestChannelB[i] = (uint16_t) (b + 0.5f);
    }
}

uint32_t ContestSelfTest_Run(void)
{
    static const uint8_t crcText[] = "123456789";
    SP_BasicStats stats;
    PID_Controller pid;
    float frequencyHz = 0.0f;
    float phaseDeg = 0.0f;
    float magnitude;
    float calibrated;
    float pidOutput;
    uint16_t crc;
    uint8_t frame[SERIAL_PROTOCOL_MEASUREMENT_FRAME_LEN];
    AnalogResult measurement = {0};
    SerialProtocol_MeasurementFrame decoded = {0};
    uint32_t failures = 0U;
    bool pass;

    SelfTest_GenerateSignals();
    USR_UART_printf("SELFTEST,BEGIN,FS=12800,N=1024,F=50\r\n");

    pass = SP_CalcBasicStatsU16(gSelfTestChannelA,
        SELFTEST_SAMPLE_COUNT, &stats) &&
        SelfTest_IsNear(stats.mean, SELFTEST_OFFSET_CODE, 1.0f) &&
        SelfTest_IsNear(stats.rmsAc, 707.1068f, 2.0f) &&
        SelfTest_IsNear(stats.peakToPeak, 2000.0f, 2.0f);
    if (!pass) {
        failures |= CONTEST_SELFTEST_FAIL_STATS;
    }
    USR_UART_printf("SELFTEST,STATS,%s,MEAN=%.3f,RMS=%.3f,VPP=%.3f\r\n",
        pass ? "PASS" : "FAIL", stats.mean, stats.rmsAc,
        stats.peakToPeak);

    pass = SP_EstimateFrequencyU16(gSelfTestChannelA,
        SELFTEST_SAMPLE_COUNT, SELFTEST_SAMPLE_RATE_HZ, &frequencyHz) &&
        SelfTest_IsNear(frequencyHz, SELFTEST_SIGNAL_HZ, 0.10f);
    if (!pass) {
        failures |= CONTEST_SELFTEST_FAIL_FREQUENCY;
    }
    USR_UART_printf("SELFTEST,FREQ,%s,HZ=%.4f\r\n",
        pass ? "PASS" : "FAIL", frequencyHz);

    pass = SP_EstimatePhaseU16(gSelfTestChannelA, gSelfTestChannelB,
        SELFTEST_SAMPLE_COUNT, SELFTEST_SIGNAL_HZ,
        SELFTEST_SAMPLE_RATE_HZ, &phaseDeg) &&
        SelfTest_IsNear(phaseDeg, SELFTEST_PHASE_DEG, 0.50f);
    if (!pass) {
        failures |= CONTEST_SELFTEST_FAIL_PHASE;
    }
    USR_UART_printf("SELFTEST,PHASE,%s,DEG=%.3f\r\n",
        pass ? "PASS" : "FAIL", phaseDeg);

    magnitude = SP_GoertzelMagnitudeU16(gSelfTestChannelA,
        SELFTEST_SAMPLE_COUNT, SELFTEST_SAMPLE_RATE_HZ,
        SELFTEST_SIGNAL_HZ);
    pass = SelfTest_IsNear(magnitude, SELFTEST_AMPLITUDE_A_CODE, 2.0f);
    if (!pass) {
        failures |= CONTEST_SELFTEST_FAIL_GOERTZEL;
    }
    USR_UART_printf("SELFTEST,GOERTZEL,%s,MAG=%.3f\r\n",
        pass ? "PASS" : "FAIL", magnitude);

    Calibration_ResetDefault();
    pass = Calibration_TwoPoint(0U, 0.0f, 100.0f,
        1000.0f, 1100.0f);
    calibrated = Calibration_Apply(0U, 600.0f);
    pass = pass && SelfTest_IsNear(calibrated, 500.0f, 0.01f);
    if (!pass) {
        failures |= CONTEST_SELFTEST_FAIL_CALIBRATION;
    }
    USR_UART_printf("SELFTEST,CAL,%s,VALUE=%.3f\r\n",
        pass ? "PASS" : "FAIL", calibrated);

    PID_Init(&pid, 2.0f, 0.0f, 0.0f, -10.0f, 10.0f);
    pidOutput = PID_Update(&pid, 10.0f, 8.0f, 0.01f);
    pass = SelfTest_IsNear(pidOutput, 4.0f, 0.001f) &&
        SelfTest_IsNear(Control_SlewLimit(10.0f, 0.0f, 1.5f),
            1.5f, 0.001f);
    if (!pass) {
        failures |= CONTEST_SELFTEST_FAIL_PID;
    }
    USR_UART_printf("SELFTEST,PID,%s,OUT=%.3f\r\n",
        pass ? "PASS" : "FAIL", pidOutput);

    crc = SerialProtocol_Crc16Ccitt(crcText,
        (uint16_t) (sizeof(crcText) - 1U));
    pass = crc == 0x29B1U;
    if (!pass) {
        failures |= CONTEST_SELFTEST_FAIL_CRC;
    }
    USR_UART_printf("SELFTEST,CRC,%s,CRC=0x%04X\r\n",
        pass ? "PASS" : "FAIL", crc);

    measurement.dcMv = 700.0f;
    measurement.rmsMv = 70.7107f;
    measurement.peakMv = 100.0f;
    measurement.peakToPeakMv = 200.0f;
    measurement.frequencyHz = 50.0f;
    measurement.phaseDeg = 30.0f;
    measurement.thdPercent = 1.25f;
    measurement.frequencyValid = true;
    measurement.phaseValid = true;
    measurement.thdValid = true;
    pass = SerialProtocol_PackMeasurementFrame(7U, 1234U,
        &measurement, frame, sizeof(frame)) &&
        SerialProtocol_UnpackMeasurementFrame(frame, sizeof(frame),
            &decoded) &&
        (decoded.meter_id == 7U) && (decoded.sequence == 1234U) &&
        SelfTest_IsNear(decoded.result.rmsMv, measurement.rmsMv, 0.001f) &&
        SelfTest_IsNear(decoded.result.frequencyHz,
            measurement.frequencyHz, 0.001f) &&
        decoded.result.frequencyValid && decoded.result.phaseValid &&
        decoded.result.thdValid;
    frame[9] ^= 0x01U;
    pass = pass && !SerialProtocol_UnpackMeasurementFrame(frame,
        sizeof(frame), &decoded);
    if (!pass) {
        failures |= CONTEST_SELFTEST_FAIL_PROTOCOL;
    }
    USR_UART_printf("SELFTEST,PROTOCOL,%s,LEN=%u\r\n",
        pass ? "PASS" : "FAIL",
        (uint16_t) SERIAL_PROTOCOL_MEASUREMENT_FRAME_LEN);

    USR_UART_printf("SELFTEST,SUMMARY,%s,MASK=0x%08X\r\n",
        (failures == 0U) ? "PASS" : "FAIL", failures);
    return failures;
}
