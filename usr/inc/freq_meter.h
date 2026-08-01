#ifndef FREQ_METER_H_
#define FREQ_METER_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * Frequency meter using TIMG12 (32-bit) edge-time capture on PB13.
 *
 * Principle: the timer runs down from a 32-bit load value at BUSCLK
 * (32 MHz).  On every rising edge of the input signal the current counter
 * value is latched into the capture register (CC0_DN interrupt).  The
 * difference between two consecutive captures equals the signal period in
 * timer ticks, so  f = BUSCLK / (periodTicks).
 *
 * The meter reports the most recent period measured between two edges and
 * the frequency derived from it.  Poll FreqMeter_Read() at a low rate
 * (e.g. every 100 ms); the IRQ only records samples.
 */

typedef struct {
    bool valid;
    uint32_t lastPeriodTicks;   /* period in 32 MHz ticks (0 if invalid) */
    float frequencyHz;          /* last derived frequency */
    uint32_t edgeCount;         /* total rising edges captured */
} FreqMeterResult;

void FreqMeter_Init(void);
/* Start/stop the capture counter and its NVIC interrupt.  Init only prepares
 * the peripheral; capture stays stopped until Start() so a floating input
 * cannot flood the CPU with edge interrupts. */
void FreqMeter_Start(void);
void FreqMeter_Stop(void);
bool FreqMeter_Read(FreqMeterResult *result);
void FreqMeter_Reset(void);
/* True if the capture counter wrapped (input lost / below ~1 Hz). */
bool FreqMeter_WasLost(void);

/* ISR entry; do not define your own TIMG12_IRQHandler when using this lib. */
void FREQ_CAPTURE_INST_IRQHandler(void);

#endif /* FREQ_METER_H_ */
