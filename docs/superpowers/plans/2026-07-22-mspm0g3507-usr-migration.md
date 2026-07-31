# MSPM0G3507 usr Migration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Migrate the MSP430-oriented `usr` modules to MSPM0G3507 using `PA27` and `PA16` for dual ADC sampling, `UART0 PA10/PA11` for debug output, and `PB1` to `PB4` for an external keyboard.

**Architecture:** MSPM0 SysConfig and DriverLib form the hardware boundary. The user modules keep stable application-facing APIs where practical, while MSP430-only peripheral code is replaced by MSPM0-compatible ADC, UART, and GPIO implementations. ADC sampling exposes the configured sample rate explicitly through setter/getter functions.

**Tech Stack:** Keil uVision ArmClang, MSPM0 DriverLib, SysConfig-generated `ti_msp_dl_config.c/.h`, C99-style embedded C.

## Global Constraints

- ADC channel 0 input is `PA27`, `ADC0 A0_0`.
- ADC channel 1 input is `PA16`, `ADC1 A1_1`.
- `PA15` is reserved for future `DAC_OUT`; ADC must not use `PA15`.
- Debug UART is `UART0`, `PA10` TX and `PA11` RX.
- External keyboard inputs are `PB1`, `PB2`, `PB3`, `PB4`.
- OLED I2C is out of scope except for build compatibility.
- ADC sampling frequency must be explicit in code through an API and a stored value.
- Do not use MSP430-only APIs or interrupt syntax in migrated source files.
- This workspace is not a git repository, so commit steps are replaced by build and diff checks.

---

## File Structure

- Modify `ti_msp_dl_config.h`: add MSPM0 pin/peripheral macros for ADC0, ADC1, UART0, and key GPIO.
- Modify `ti_msp_dl_config.c`: initialize power, GPIO, ADC0, ADC1, DMA where used, UART0, and keyboard pins.
- Modify `usr/inc/sysconfig.h` and `usr/src/sysconfig.c`: replace MSP430 clock initialization with a thin wrapper around MSPM0 SysConfig.
- Modify `usr/inc/adc2chSample.h` and `usr/src/adc2chSample.c`: implement dual ADC sample buffers and explicit sample-rate API.
- Modify `usr/inc/uart_debug.h` and `usr/src/uart_debug.c`: implement `printf` redirection through `UART0`.
- Create `usr/inc/key.h` and `usr/src/key.c`: implement keyboard input scan and debounce.
- Modify `usr/inc/OLED.h` and `usr/src/OLED.c` only enough to remove MSP430-only includes/macros from the build while leaving OLED behavior out of scope.
- Modify `main.c`: replace the ADC DMA demo with a small migrated application skeleton that initializes SysConfig, UART, keys, FFT tables, and one ADC frame sample.
- Modify `project/mspm0_damo.uvprojx`: add `key.c` to the `usr` group if not auto-added by the IDE.

---

### Task 1: MSPM0 Config Surface

**Files:**
- Modify: `ti_msp_dl_config.h`
- Modify: `ti_msp_dl_config.c`
- Modify: `usr/inc/sysconfig.h`
- Modify: `usr/src/sysconfig.c`

**Interfaces:**
- Produces: `SYSCFG_DL_init(void)`, `Sysclk_Init(void)`, and peripheral macros used by later tasks.
- Consumes: existing MSPM0 DriverLib headers under `source/ti/driverlib`.

- [ ] **Step 1: Replace MSP430 driver includes**

In `usr/inc/sysconfig.h`, replace `#include "driverlib.h"` with:

```c
#include "ti_msp_dl_config.h"
```

- [ ] **Step 2: Replace `Sysclk_Init` implementation**

In `usr/src/sysconfig.c`, keep `ALLOCATE_DATA_BUFF` and replace MSP430 clock setup with:

```c
void Sysclk_Init(void)
{
    SYSCFG_DL_init();
}
```

- [ ] **Step 3: Add pin/peripheral macros to `ti_msp_dl_config.h`**

Add macros for:

```c
#define ADC12_0_INST ADC0
#define ADC12_0_ADCMEM_0 DL_ADC12_MEM_IDX_0
#define GPIO_ADC12_0_A0_PORT GPIOA
#define GPIO_ADC12_0_A0_PIN DL_GPIO_PIN_27

#define ADC12_1_INST ADC1
#define ADC12_1_ADCMEM_0 DL_ADC12_MEM_IDX_0
#define GPIO_ADC12_1_A1_PORT GPIOA
#define GPIO_ADC12_1_A1_PIN DL_GPIO_PIN_16

#define UART_DEBUG_INST UART0
#define UART_DEBUG_INST_IRQHandler UART0_IRQHandler
#define UART_DEBUG_INST_INT_IRQN UART0_INT_IRQn
#define GPIO_UART_DEBUG_TX_PORT GPIOA
#define GPIO_UART_DEBUG_TX_PIN DL_GPIO_PIN_10
#define GPIO_UART_DEBUG_RX_PORT GPIOA
#define GPIO_UART_DEBUG_RX_PIN DL_GPIO_PIN_11

#define KEY_PORT GPIOB
#define KEY1_PIN DL_GPIO_PIN_1
#define KEY2_PIN DL_GPIO_PIN_2
#define KEY3_PIN DL_GPIO_PIN_3
#define KEY4_PIN DL_GPIO_PIN_4
#define KEY_ALL_PINS (KEY1_PIN | KEY2_PIN | KEY3_PIN | KEY4_PIN)
```

- [ ] **Step 4: Update SysConfig init declarations**

Ensure `ti_msp_dl_config.h` declares:

```c
void SYSCFG_DL_UART_DEBUG_init(void);
void SYSCFG_DL_ADC12_1_init(void);
void SYSCFG_DL_KEY_init(void);
```

- [ ] **Step 5: Update `SYSCFG_DL_init` order**

In `ti_msp_dl_config.c`, initialize power and GPIO before module setup, then call:

```c
SYSCFG_DL_SYSCTL_init();
SYSCFG_DL_ADC12_0_init();
SYSCFG_DL_ADC12_1_init();
SYSCFG_DL_UART_DEBUG_init();
SYSCFG_DL_KEY_init();
```

- [ ] **Step 6: Verify no MSP430 clock symbols remain**

Run:

```powershell
rg -n "PMM_|UCS_|GPIO_PORT_P5|driverlib\.h" usr ti_msp_dl_config.c ti_msp_dl_config.h
```

Expected: no matches for `PMM_`, `UCS_`, `GPIO_PORT_P5`, or MSP430 `driverlib.h`.

---

### Task 2: Dual ADC Sampling Adapter With Explicit Sample Rate

**Files:**
- Modify: `usr/inc/adc2chSample.h`
- Modify: `usr/src/adc2chSample.c`

**Interfaces:**
- Consumes: `ADC12_0_INST`, `ADC12_1_INST`, `ADC12_0_ADCMEM_0`, `ADC12_1_ADCMEM_0`.
- Produces:
  - `void ADC12_SetSampleRateHz(uint32_t sampleRateHz);`
  - `uint32_t ADC12_GetSampleRateHz(void);`
  - `void ADC12_FrameSampleOnce(void);`
  - `float ADC12_GetAVCC_Voltage(void);`
  - `uint16_t *pADC_Buffer0;`
  - `uint16_t *pADC_Buffer1;`
  - `volatile bool ADC_Finished;`

- [ ] **Step 1: Add explicit sample-rate API to header**

In `usr/inc/adc2chSample.h`, add:

```c
#define ADC_SAMPLE_RATE_DEFAULT_HZ (1600U)

void ADC12_SetSampleRateHz(uint32_t sampleRateHz);
uint32_t ADC12_GetSampleRateHz(void);
```

Keep `ADC_SAMPLE_RATE` only if existing signal-processing formulas still require a floating value, and define it from the default:

```c
#define ADC_SAMPLE_RATE ((float) ADC_SAMPLE_RATE_DEFAULT_HZ)
```

- [ ] **Step 2: Replace MSP430 ADC implementation**

In `usr/src/adc2chSample.c`, remove `Timer_A`, `#pragma vector`, `__interrupt`, and `ADC12_A` usage. Add a file-scope variable:

```c
static uint32_t gADCSampleRateHz = ADC_SAMPLE_RATE_DEFAULT_HZ;
```

- [ ] **Step 3: Implement sample-rate setter/getter**

Implement:

```c
void ADC12_SetSampleRateHz(uint32_t sampleRateHz)
{
    if (sampleRateHz == 0U) {
        sampleRateHz = ADC_SAMPLE_RATE_DEFAULT_HZ;
    }
    gADCSampleRateHz = sampleRateHz;
}

uint32_t ADC12_GetSampleRateHz(void)
{
    return gADCSampleRateHz;
}
```

- [ ] **Step 4: Implement frame sampling loop**

Implement a first-pass compatible frame sampler that fills both buffers with `ADC_BUFFER_LEN` samples from `ADC0 MEM0` and `ADC1 MEM0`. Use explicit comments that the loop is a correctness baseline and the configured sample rate is stored by `ADC12_SetSampleRateHz`; high-rate timer/DMA triggering is the next optimization point if command-line verification shows the baseline compiles.

The loop body must call MSPM0 DriverLib APIs:

```c
DL_ADC12_startConversion(ADC12_0_INST);
DL_ADC12_startConversion(ADC12_1_INST);
while (DL_ADC12_isConversionStarted(ADC12_0_INST)) {}
while (DL_ADC12_isConversionStarted(ADC12_1_INST)) {}
pADC_Buffer0[i] = DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_0);
pADC_Buffer1[i] = DL_ADC12_getMemResult(ADC12_1_INST, ADC12_1_ADCMEM_0);
```

If these exact status APIs differ in the local DriverLib, inspect `source/ti/driverlib/dl_adc12.h` and use the local equivalent.

- [ ] **Step 5: Keep AVCC query explicit but conservative**

Implement `ADC12_GetAVCC_Voltage` as:

```c
float ADC12_GetAVCC_Voltage(void)
{
    return 3.3f;
}
```

This preserves the old API without pretending the MSP430 battery-monitor path has been ported.

- [ ] **Step 6: Verify no MSP430 ADC symbols remain**

Run:

```powershell
rg -n "Timer_A|ADC12_A|TIMER1_A1_VECTOR|__interrupt|TA1IV|ADC12IFG" usr/src/adc2chSample.c usr/inc/adc2chSample.h
```

Expected: no matches.

---

### Task 3: UART0 Debug Adapter

**Files:**
- Modify: `usr/inc/uart_debug.h`
- Modify: `usr/src/uart_debug.c`

**Interfaces:**
- Consumes: `UART_DEBUG_INST`, `SYSCFG_DL_UART_DEBUG_init()`.
- Produces: `void uart_debugInit(void);`, `fputc`, `fputs`.

- [ ] **Step 1: Replace include**

In `usr/inc/uart_debug.h`, replace `#include "driverlib.h"` with:

```c
#include "ti_msp_dl_config.h"
```

- [ ] **Step 2: Implement UART transmit helper**

In `usr/src/uart_debug.c`, add:

```c
static void uart_debugPutChar(uint8_t ch)
{
    while (DL_UART_isBusy(UART_DEBUG_INST)) {}
    DL_UART_transmitData(UART_DEBUG_INST, ch);
}
```

If the local DriverLib uses a different function name, inspect `source/ti/driverlib/dl_uart.h` and use the local equivalent.

- [ ] **Step 3: Rewrite `fputc` and `fputs`**

Use `uart_debugPutChar` for every byte, including `\r` before `\n` if desired for serial terminals.

- [ ] **Step 4: Make init idempotent**

Implement:

```c
void uart_debugInit(void)
{
    SYSCFG_DL_UART_DEBUG_init();
}
```

- [ ] **Step 5: Verify no MSP430 UART symbols remain**

Run:

```powershell
rg -n "USCI_A|UCTXIFG|GPIO_PORT_P4|driverlib\.h" usr/src/uart_debug.c usr/inc/uart_debug.h
```

Expected: no matches.

---

### Task 4: Keyboard Module

**Files:**
- Create: `usr/inc/key.h`
- Create: `usr/src/key.c`
- Modify: `project/mspm0_damo.uvprojx`

**Interfaces:**
- Consumes: `KEY_PORT`, `KEY1_PIN`, `KEY2_PIN`, `KEY3_PIN`, `KEY4_PIN`, `KEY_ALL_PINS`.
- Produces:
  - `void Key_Init(void);`
  - `uint8_t Key_ReadRaw(void);`
  - `uint8_t Key_ReadDebounced(void);`
  - bit masks `KEY_1`, `KEY_2`, `KEY_3`, `KEY_4`.

- [ ] **Step 1: Create `key.h`**

Create:

```c
#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include "ti_msp_dl_config.h"

#define KEY_1 (0x01U)
#define KEY_2 (0x02U)
#define KEY_3 (0x04U)
#define KEY_4 (0x08U)

void Key_Init(void);
uint8_t Key_ReadRaw(void);
uint8_t Key_ReadDebounced(void);

#endif
```

- [ ] **Step 2: Create `key.c`**

Create a GPIO input implementation that treats pressed keys as low if pull-ups are enabled:

```c
#include "key.h"

void Key_Init(void)
{
    SYSCFG_DL_KEY_init();
}

uint8_t Key_ReadRaw(void)
{
    uint32_t pins = DL_GPIO_readPins(KEY_PORT, KEY_ALL_PINS);
    uint8_t value = 0U;

    if ((pins & KEY1_PIN) == 0U) { value |= KEY_1; }
    if ((pins & KEY2_PIN) == 0U) { value |= KEY_2; }
    if ((pins & KEY3_PIN) == 0U) { value |= KEY_3; }
    if ((pins & KEY4_PIN) == 0U) { value |= KEY_4; }

    return value;
}

uint8_t Key_ReadDebounced(void)
{
    uint8_t first = Key_ReadRaw();
    delay_cycles(32000U);
    if (first == Key_ReadRaw()) {
        return first;
    }
    return 0U;
}
```

- [ ] **Step 3: Add `key.c` to Keil project**

Add a file entry under the existing `usr` group:

```xml
<File>
  <FileName>key.c</FileName>
  <FileType>1</FileType>
  <FilePath>..\usr\src\key.c</FilePath>
</File>
```

- [ ] **Step 4: Verify project entry**

Run:

```powershell
Select-String -Path 'project/mspm0_damo.uvprojx' -Pattern 'key.c'
```

Expected: one `key.c` file entry.

---

### Task 5: OLED Build Compatibility

**Files:**
- Modify: `usr/inc/OLED.h`
- Modify: `usr/src/OLED.c`

**Interfaces:**
- Produces the existing OLED public functions so code links.
- OLED behavior is out of scope.

- [ ] **Step 1: Remove MSP430 GPIO include and register macros**

In `usr/inc/OLED.h`, replace `#include "gpio.h"` with:

```c
#include "ti_msp_dl_config.h"
```

Replace MSP430 `HWREG16` macros with compile-safe stubs if OLED is not used:

```c
#define OLED_SCL_1 do {} while (0)
#define OLED_SDA_1 do {} while (0)
#define OLED_SCL_0 do {} while (0)
#define OLED_SDA_0 do {} while (0)
```

- [ ] **Step 2: Make `OLED_I2C_Init` compile**

In `usr/src/OLED.c`, replace MSP430 GPIO calls in `OLED_I2C_Init` with an empty body:

```c
void OLED_I2C_Init(void)
{
}
```

- [ ] **Step 3: Verify no MSP430 OLED symbols remain**

Run:

```powershell
rg -n "__MSP430|HWREG16|OFS_PAOUT|GPIO_PORT_P3|gpio\.h" usr/src/OLED.c usr/inc/OLED.h
```

Expected: no matches.

---

### Task 6: Main Application Skeleton

**Files:**
- Modify: `main.c`

**Interfaces:**
- Consumes: `Sysclk_Init`, `uart_debugInit`, `Key_Init`, `ADC12_SetSampleRateHz`, `ADC12_GetSampleRateHz`, `ADC12_FrameSampleOnce`, `SP_myFFTInit`.

- [ ] **Step 1: Replace demo-only main**

Replace the current single ADC DMA demo with:

```c
#include <stdio.h>
#include "sysconfig.h"
#include "uart_debug.h"
#include "adc2chSample.h"
#include "signalProcess.h"
#include "key.h"

int main(void)
{
    Sysclk_Init();
    uart_debugInit();
    Key_Init();
    SP_myFFTInit();

    ADC12_SetSampleRateHz(ADC_SAMPLE_RATE_DEFAULT_HZ);
    printf("ADC sample rate: %lu Hz\r\n", (unsigned long) ADC12_GetSampleRateHz());

    while (1) {
        ADC12_FrameSampleOnce();
        printf("ADC frame done, keys=0x%02x\r\n", Key_ReadDebounced());
        __WFI();
    }
}
```

- [ ] **Step 2: Remove old `ADC12_0_INST_IRQHandler` from main**

Delete the demo ADC IRQ handler from `main.c` unless it is still needed by the new ADC implementation.

- [ ] **Step 3: Verify main references migrated APIs**

Run:

```powershell
rg -n "ADC12_SetSampleRateHz|ADC12_GetSampleRateHz|Key_Init|ADC12_0_INST_IRQHandler|gADCSamples" main.c
```

Expected: matches for the new APIs, no matches for old demo symbols.

---

### Task 7: Build and Migration Verification

**Files:**
- Test only.

**Interfaces:**
- Consumes all previous tasks.
- Produces a build log and a list of remaining issues if any.

- [ ] **Step 1: Run MSP430 symbol scan**

Run:

```powershell
rg -n "PMM_|UCS_|USCI_A|Timer_A|ADC12_A|__MSP430|#pragma vector|__interrupt|HWREG16|GPIO_PORT_|driverlib\.h|gpio\.h" usr main.c ti_msp_dl_config.c ti_msp_dl_config.h
```

Expected: no matches except harmless comments if any.

- [ ] **Step 2: Build with Keil command line**

Run from `project`:

```powershell
$uv4 = 'D:\MDK5stm32\install\UV4\UV4.exe'
$log = 'uv4_build.log'
if (Test-Path $log) { Remove-Item $log -Force }
$p = Start-Process -FilePath $uv4 -WorkingDirectory (Get-Location) -ArgumentList @('-b', 'mspm0_damo.uvprojx', '-t', 'MSPM0G3507', '-j0', '-o', $log) -WindowStyle Hidden -PassThru
$p.WaitForExit()
Write-Output "EXIT=$($p.ExitCode)"
if (Test-Path $log) { Get-Content $log -TotalCount 240 }
```

Expected: `EXIT=0` and `0 Error(s)`.

- [ ] **Step 3: If build fails, fix one root cause at a time**

For each compiler error:

1. Read the first error line and the referenced file.
2. Inspect the local DriverLib header for the exact API name.
3. Patch only the file responsible for that first error.
4. Re-run the build command.

- [ ] **Step 4: Confirm explicit sample-rate visibility**

Run:

```powershell
rg -n "ADC12_SetSampleRateHz|ADC12_GetSampleRateHz|gADCSampleRateHz|ADC sample rate" usr main.c
```

Expected: definitions in `adc2chSample.c/.h` and a runtime print in `main.c`.

