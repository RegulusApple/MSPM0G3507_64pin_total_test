/*
 * MSPM0G3507 四项独立测试入口
 *
 * 板载按键 S2 (PB21)：
 *   单击     : 切换测试项 (0..3)
 *   长按>0.8s: 执行当前测试项
 * UART0 (PA10, 115200) 输出结果；OLED 第 0 行显示当前测试项
 *
 * 测试项：
 *   0 FREQ   - 测频精度：TIMG12 PERIOD 捕获（PB13 输入），对比信号源
 *   1 RTC    - RTC 时间走时（LFXT 32.768kHz），每秒打印，秒数应递增
 *   2 PSU    - CV/CC 控制节拍：TIMG6 20kHz 中断计数，tick 应快速递增
 *   3 ADC    - ADC 高速采样：单帧 @100kHz，打印 RMS/频率，验证高采样率链路
 */

#include "sysconfig.h"

#include <stdbool.h>
#include <stdint.h>

#include "OLED.h"
#include "adc2chSample.h"
#include "analog_measure.h"
#include "freq_meter.h"
#include "psu_control.h"
#include "system_time.h"
#include "usr_rtc.h"
#include "usr_uart.h"

#define TEST_COUNT         (4U)
#define TEST_LOOP_MS       (20U)
#define TEST_LONG_PRESS_MS (800U)

typedef enum {
    TEST_FREQ = 0,
    TEST_RTC,
    TEST_PSU,
    TEST_ADC
} TestItem;

static const char *const gTestNames[TEST_COUNT] = {
    "FREQ", "RTC ", "PSU ", "ADC "
};

static TestItem gTest = TEST_FREQ;
static uint32_t gLoopCount = 0U;

/* S2 (PB21) scan state */
static bool gBtnPressed;
static uint32_t gBtnHoldMs;
static bool gBtnLongSent;

static void Test_ShowName(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "TEST:");
    OLED_ShowString(1, 7, (char *) gTestNames[gTest]);
    OLED_ShowString(2, 1, "CLK=next");
    OLED_ShowString(3, 1, "LONG=run");
    OLED_ShowString(4, 1, "KEY:");
    /* 第 4 行显示按键实时电平：未按=1，按下=0 */
    OLED_ShowChar(4, 5,
        ((DL_GPIO_readPins(KEY_LED_PORT, KEY_LED_BUTTON_PIN) &
          KEY_LED_BUTTON_PIN) != 0U) ? '1' : '0');
}

static void Test_Enter(TestItem item)
{
    /* Cleanup of the previous test. */
    switch (gTest) {
        case TEST_FREQ:
            FreqMeter_Stop();          /* stop capture + its interrupt */
            break;
        case TEST_PSU:
            Psu_Stop();
            break;
        default:
            break;
    }

    gTest = item;
    Test_ShowName();

    switch (gTest) {
        case TEST_FREQ:
            FreqMeter_Init();          /* prepare only; started in Test_RunFreq */
            FreqMeter_Reset();
            USR_UART_sendString("TEST=FREQ, feed signal to PB13, long-press to run\r\n");
            break;
        case TEST_RTC:
            UsrRtc_Init();
            USR_UART_sendString("TEST=RTC, long-press to run\r\n");
            break;
        case TEST_PSU:
        {
            PsuConfig cfg = {
                .vrefTargetV = 30.0f,
                .irefTargetA = 2.2f,
                .curveMode   = CURVE_PV,
                .vKp = 0.5f, .vKi = 0.1f, .vKd = 0.0f,
                .iKp = 0.3f, .iKi = 0.05f, .iKd = 0.0f,
                .dacBase = 0.0f,
                .dacVoltPerCode = 30.0f / 4095.0f,
                .dacAmpPerCode  = 2.2f / 4095.0f,
            };
            Psu_Init(&cfg);
            USR_UART_sendString("TEST=PSU, long-press to run 20kHz tick\r\n");
            break;
        }
        case TEST_ADC:
            ADC12_Init();
            USR_UART_sendString("TEST=ADC, long-press to capture @100kHz\r\n");
            break;
        default:
            break;
    }
}

static void Test_RunFreq(void)
{
    FreqMeterResult fm;
    uint32_t i;

    FreqMeter_Start();      /* start capture only during the measurement */
    FreqMeter_Reset();
    SystemTime_DelayMs(500U);   /* let several edges accumulate */

    for (i = 0U; i < 5U; i++) {
        if (FreqMeter_Read(&fm) && fm.valid) {
            USR_UART_printf("FREQ: F=%.3f Hz, period=%u ticks, edges=%u\r\n",
                fm.frequencyHz, fm.lastPeriodTicks, fm.edgeCount);
            OLED_ShowString(2, 1, "F=");
            OLED_ShowFloat(2, 3, fm.frequencyHz, 1);
        } else {
            USR_UART_printf("FREQ: no valid capture (lost=%d)\r\n",
                FreqMeter_WasLost());
        }
        SystemTime_DelayMs(200U);
    }

    FreqMeter_Stop();       /* stop capture so the interrupt cannot stall OLED */
}

static void Test_RunRtc(void)
{
    UsrRtcTime t0, t1;
    uint32_t i;

    if (!UsrRtc_IsClockReady()) {
        USR_UART_sendString("RTC: LFXT not ready (crystal?)\r\n");
        return;
    }
    if (!UsrRtc_GetTime(&t0)) {
        USR_UART_sendString("RTC: read failed\r\n");
        return;
    }
    for (i = 0U; i < 5U; i++) {
        if (UsrRtc_GetTime(&t1)) {
            USR_UART_printf("RTC: %04u-%02u-%02u %02u:%02u:%02u s2000=%u\r\n",
                t1.year, t1.month, t1.dayOfMonth,
                t1.hours, t1.minutes, t1.seconds,
                UsrRtc_GetUnixSeconds2000());
            OLED_ShowString(2, 1, "TIME:");
            OLED_ShowNum(2, 7, t1.hours, 2);
            OLED_ShowChar(2, 9, ':');
            OLED_ShowNum(2, 10, t1.minutes, 2);
        }
        SystemTime_DelayMs(1000U);
    }
    /* t1 should be >= t0 + ~4s (5 iterations, 1s each). */
    if (t1.seconds != t0.seconds) {
        USR_UART_printf("RTC: seconds advanced (t0=%u -> t1=%u)\r\n",
            t0.seconds, t1.seconds);
    } else {
        USR_UART_sendString("RTC: WARNING seconds did not advance\r\n");
    }
}

static void Test_RunPsu(void)
{
    PsuState st;
    uint32_t startTick;
    uint32_t i;

    Psu_Start();
    SystemTime_DelayMs(100U);
    st = Psu_GetState();
    startTick = st.tickCount;
    USR_UART_printf("PSU: tick after 100ms = %u\r\n", startTick);

    for (i = 0U; i < 5U; i++) {
        Psu_Update(28.0f, 0.5f);
        st = Psu_GetState();
        USR_UART_printf("PSU: tick=%u dac=%u cv=%d cc=%d\r\n",
            st.tickCount, st.dacCode, st.inCvMode, st.inCcMode);
        SystemTime_DelayMs(100U);
    }

    /* 500ms elapsed => expect ~10000 ticks at 20kHz. */
    st = Psu_GetState();
    USR_UART_printf("PSU: delta=%u ticks in 500ms (expect ~10000)\r\n",
        st.tickCount - startTick);
    Psu_Stop();
}

static void Test_RunAdc(void)
{
    uint16_t validCount;
    ADC12_RmsResult rms;
    float freqHz;
    bool ok;

    ok = ADC12_CaptureFrameHighRate(ADC12_CHANNEL_0, 100000U, &validCount);
    USR_UART_printf("ADC: high-rate capture %s, valid=%u\r\n",
        ok ? "OK" : "FAIL", validCount);
    if (!ok) {
        return;
    }

    if (ADC12_CalcRms(ADC12_CHANNEL_0, &rms)) {
        USR_UART_printf("ADC: rms=%u code, rmsMv=%.3f, dc=%.3f\r\n",
            (uint32_t) rms.rmsCode, rms.rmsMv, rms.dcCode);
    }
    if (ADC12_CalcFrequency(ADC12_CHANNEL_0, &freqHz)) {
        USR_UART_printf("ADC: freq=%.3f Hz\r\n", freqHz);
        OLED_ShowString(2, 1, "F=");
        OLED_ShowFloat(2, 3, freqHz, 1);
    } else {
        USR_UART_sendString("ADC: freq estimate failed (no signal?)\r\n");
    }
    USR_UART_printf("ADC: fs set=%u actual=", ADC12_GetSampleRateHz());
    ADC12_CheckSampleRate(&freqHz);
    USR_UART_printf("%.0f\r\n", freqHz);
}

static void Test_ScanButton(void)
{
    bool pressed = ((DL_GPIO_readPins(KEY_LED_PORT, KEY_LED_BUTTON_PIN) &
        KEY_LED_BUTTON_PIN) == 0U);

    if (pressed && !gBtnPressed) {
        gBtnHoldMs = 0U;
        gBtnLongSent = false;
    } else if (pressed && gBtnPressed) {
        gBtnHoldMs += TEST_LOOP_MS;
        if (!gBtnLongSent && (gBtnHoldMs >= TEST_LONG_PRESS_MS)) {
            gBtnLongSent = true;
            switch (gTest) {
                case TEST_FREQ: Test_RunFreq(); break;
                case TEST_RTC:  Test_RunRtc();  break;
                case TEST_PSU:  Test_RunPsu();  break;
                case TEST_ADC:  Test_RunAdc();  break;
                default: break;
            }
            Test_ShowName();
        }
    } else if (!pressed && gBtnPressed) {
        if (!gBtnLongSent) {
            Test_Enter((TestItem) ((gTest + 1U) % TEST_COUNT));
        }
    }
    gBtnPressed = pressed;
}

int main(void)
{
    Sysclk_Init();
    USR_UART_init();
    OLED_Init();
    UsrRtc_Init();

    USR_UART_sendString("\r\n=== MSPM0G3507 4-item test ===\r\n");
    USR_UART_printf("FCL=%s, CPUCLK=%u\r\n",
        Sysclk_IsFclActive() ? "ON" : "OFF", CPUCLK_FREQ);
    USR_UART_printf("OLED=%s\r\n", OLED_IsPresent() ? "present" : "absent");

    Test_Enter(TEST_FREQ);

    while (1) {
        Test_ScanButton();

        /* 心跳：每 500ms 打印一次按键电平与当前测试项。
         * 只要串口打开就能持续看到数据，用于验证串口链路和按键读取。 */
        gLoopCount++;
        if ((gLoopCount % (500U / TEST_LOOP_MS)) == 0U) {
            bool keyLevel = ((DL_GPIO_readPins(KEY_LED_PORT, KEY_LED_BUTTON_PIN) &
                KEY_LED_BUTTON_PIN) != 0U);
            USR_UART_printf("HB:test=%s key=%d btn=%d\r\n",
                gTestNames[gTest], keyLevel, gBtnPressed);
            OLED_ShowChar(4, 5, keyLevel ? '1' : '0');
        }

        SystemTime_DelayMs(TEST_LOOP_MS);
    }
}
