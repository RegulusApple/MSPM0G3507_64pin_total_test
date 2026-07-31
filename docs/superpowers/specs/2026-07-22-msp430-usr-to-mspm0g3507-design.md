# MSP430 usr to MSPM0G3507 Migration Design

Date: 2026-07-22

## Goal

Refactor the existing MSP430-oriented `usr` code so it can build and run on MSPM0G3507 while preserving the upper-level signal-processing API where practical.

## Confirmed Pin Mapping

- ADC channel 0 input: `PA27`, `ADC0 A0_0`.
- ADC channel 1 input: `PA16`, `ADC1 A1_1`.
- DAC is reserved for later use on `PA15`, `DAC_OUT`; ADC must not use `PA15`.
- Debug UART: `UART0`, `PA10` as TX and `PA11` as RX.
- External keyboard inputs: `PB1`, `PB2`, `PB3`, `PB4`.
- OLED I2C is out of scope for this step.

## Architecture

Use MSPM0 SysConfig-generated initialization and MSPM0 DriverLib APIs as the hardware boundary. Remove MSP430 DriverLib dependencies such as `PMM`, `UCS`, `USCI_A_UART`, `Timer_A`, `ADC12_A`, `#pragma vector`, `__interrupt`, and MSP430 port register macros from the migrated `usr` code.

`signalProcess.c` remains the algorithm layer and should only receive minimal include or type fixes.

`adc2chSample.c/.h` becomes the MSPM0 ADC sampling adapter. It should retain the existing public data buffers and `ADC12_FrameSampleOnce()` style API where possible, so callers do not need a broad rewrite. Sampling should be designed around hardware triggering and DMA rather than per-sample timer interrupts.

`uart_debug.c/.h` becomes the MSPM0 UART0 debug adapter and keeps `printf` redirection.

Add `key.c/.h` as a small keyboard input module for `PB1` to `PB4`, with initialization, raw read, and debounced read interfaces.

## ADC Sampling Notes

MSPM0G3507 ADC modules can reach high sample rates, but reliable near-4MSPS operation depends on clock configuration, sample time, DMA setup, memory bandwidth, and analog source impedance. The migration should provide an MSPM0-compatible high-rate sampling structure, then verify the actual configured rate in SysConfig and build output.

The two confirmed analog inputs are on separate ADC modules, allowing simultaneous dual-channel sampling in principle.

## Error Handling and Constraints

- Do not enable `DAC_OUT` while using `PA15` for any external ADC input; `PA15` is reserved.
- Do not use `PA19` or `PA20` because they are SWD pins.
- Avoid assigning `PB1` to UART0 RX, and avoid assigning `PB2/PB3` to I2C1 while they are reserved for the keyboard.
- Keep OLED changes out of scope unless needed only to make the build pass.

## Verification

- Update the Keil project and source so the migrated files compile with ArmClang.
- Build the `project/mspm0_damo.uvprojx` target from command line where possible.
- Confirm there are no remaining MSP430-only symbols in migrated source files.
- Confirm the SysConfig-generated declarations match the selected ADC, UART, DMA, and GPIO pins.
