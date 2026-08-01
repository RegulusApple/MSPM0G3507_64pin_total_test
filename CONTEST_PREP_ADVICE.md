# MSPM0G3507 用户库 — 结合 2024 模邀赛题的准备意见（非控制方向）

更新日期：2026-08-15
配套文档：`HANDOFF.md`、`MODIFICATION_ADVICE.md`

## 0. 背景与方向

- 竞赛为封闭式，赛前不知题目方向，当前是准备阶段；
- **已排除控制类题目**（云台/舵机/电机/小车/无人机方向不准备）；
- **DDS 模块按"可能掉落"准备**：已内置 AD9850 软件串行驱动库（`ad9850.c/.h`，
  不依赖硬件 SPI，四线 W_CLK/FQ_UD/DATA/RESET，默认 PA28/29/30/24，见第 5.4 节）；
  若赛时掉落 AD9833 也可用（现有桩，需恢复）；无模块时用片内 DAC+定时器方波方案；
- 参考：2024 年全国大学生电子设计竞赛模拟电子系统设计专题赛（TI 杯）赛题概略
  （A 运放参数测量 / B D类功放 / C 信号处理+云台激光 / D 光伏阵列模拟器）；
- 排除 C（控制题）后，本文件聚焦 **A 仪器仪表**、**B 音频功放**、**D 电源模拟** 三类，
  其中与工程现有储备（电流测量、MeterRecord、地址码、无线电源控制）最契合的是
  **电参数测量 / 电源类**，建议以此为主、仪器仪表为辅。

## 1. 赛题 → 工程能力映射（非控制方向）

| 赛题 | 核心需求 | 工程现状 | 缺口 |
|---|---|---|---|
| **A 运放参数测量** | kHz~MHz 扫频激励、精密电压测量、增益/带宽/压摆率、频率响应 | DAC 正弦(≤125 kHz 实用)、ADC 双通道、FFT、两点校准 | 片内信号源增强（DAC 扫频 + 定时器方波）、输入捕获测频 |
| **B D 类音频功放** | 音频采样 → PWM 调制 → 功放 | DAC、ADC、FFT(THD 可测) | PWM 输出库、音频缓冲 |
| **D 光伏阵列模拟器** | 双路 ADC、DAC 输出、CV/CC 双环、伏安曲线、显示 | ADC0/1+DMA、DAC0、单环 PID、校准、Flash 记录 | CV/CC 双环 PID、曲线生成、控制节拍验证 |

**结论**：测量侧（ADC+FFT+校准）已强；非控制方向下真正的缺口集中在
**片内信号源（DAC 增强 + 方波发生器）** 和 **电源闭环（CV/CC）** 两条线，
外加一个通用 PWM 输出（D 类功放、电源驱动都要用）。

---

## 2. 建议新增库（按优先级）

### P0-1 片内信号源库（DAC 正弦增强 + 定时器方波发生器）+ AD9850 DDS 驱动 ★

**为什么加**：A 题要测运放频率响应/带宽，需要 kHz~MHz 扫频激励。
赛时若掉落 AD9850 模块则直接用 DDS（已内置 `ad9850` 库，见下）；否则用片内资源组合：

- **低频高质量正弦（~100 Hz ~ 15 kHz）**：DAC 查表正弦（已有 `dac.c` 基础），
  每周期 64 点以上，DMA 输出；
- **高频精确方波（1 Hz ~ MHz）**：定时器输出比较（TIMGx），频率由
  32 MHz SYSOSC（或板上 40 MHz HFXT Y2）分频得到，精度高；
- **方波 → 正弦近似**：方波经外部简单 RC 积分器（比赛现场用电阻电容搭）可得
  三角波/正弦近似，配合 DAC 低频段覆盖大部分测试；
- **方波激励法测带宽**：运放输入方波，输出仍为方波（含基波+谐波），
  用 ADC 采样 + FFT 提取基波幅值即可测幅频响应，无需正弦源。

**AD9850 DDS 库（已加入工程，`usr/src/ad9850.c` + `usr/inc/ad9850.h`）**：
- 软件串行驱动，40 位控制字（32 位频率字 + 5 位相位），**不占硬件 SPI**，
  只接四根线：W_CLK / FQ_UD / DATA / RESET（+ 3V3/GND）；
- 默认引脚 PA28(W_CLK) / PA29(FQ_UD) / PA30(DATA) / PA24(RESET)，
  已在 `.syscfg` 的 `DDS_CTRL` 组登记并重新生成；
- 参考时钟默认 125 MHz（`AD9850_REF_CLK_HZ`），若掉落的是 AD9851（6 倍频）
  或不同晶振，改这一个宏即可；
- 接口：`AD9850_Init()` / `AD9850_SetFrequencyHz()` /
  `AD9850_SetFrequencyPhaseHz(freq, phaseDeg)` / `AD9850_PowerDown()`，
  并已接入 `wave_gen`（`WAVE_SOURCE_AD9850`，支持 SINE/SQUARE）。

**怎么做**：
1. 增强 `dac.c`：新增扫频 API（起始/终止频率、步进、每频点驻留时间）和
   幅度/偏置设置（现有 `DAC_SetSineAmplitudeMv` / `DAC_StartSineHz` 基础上封装）；
2. 新增 `square_gen` 库：SysConfig 加一个 TIMER 实例（推荐 **TIMG7**，TIMA0 留给
   PWM），输出比较模式产生方波，占空比 50%，频率可在线改；
3. 如需更高频率精度，在 `.syscfg` 使能 40 MHz HFXT（板上 Y2 已装）；
4. 赛时接上 AD9850 模块 → `WaveGen_Start({WAVE_SOURCE_AD9850, WAVE_SINE, f, 0, 0, phase})`
   即可，无需改代码。

**怎么用**：

```c
#include "sysconfig.h"
#include "square_gen.h"     // 新增库
#include "dac.h"
#include "ad9850.h"

// 方案一：掉落 AD9850 模块
Ad9850_Init();
Ad9850_SetFrequencyHz(1000000U);        // 1 MHz 正弦
// 扫频：循环改频率即可测运放幅频特性
for (f = 1e3; f <= 1e7; f *= 1.26f) {
    Ad9850_SetFrequencyHz((uint32_t) f);
    Analog_CaptureAndAnalyze(ADC12_CHANNEL_0, ADC_SAMPLE_RATE_DEFAULT_HZ, &r);
    USR_UART_printf("F=%.0f, V=%.3fmV\r\n", f, r.rmsMv);
}

// 方案二：无模块时，低频用 DAC 扫频，高频用定时器方波
Dac_SweepSine(100U, 10000U, 20U, &resultTable);
SquareGen_Init();                       // SYSCFG_DL_SQUARE_TIMER_init()
SquareGen_SetFreqHz(1000000U);          // 1 MHz 方波
SquareGen_Start();
```

### P0-2 输入捕获测频库（TIMG12 为主）——A 题必需 ★

**为什么加**：运放带宽测量要测 MHz 级信号频率/周期，ADC+FFT 做不到
（12.8 kHz 采样率上限）；TIMG 输入捕获可测到几十 MHz，同时可测方波上升沿
时间（压摆率相关）。TIMG0 已给 ADC 采样、TIMG8 保留低功耗。

**SysConfig**：
- `CAPTURE_TIMER`：优先 **TIMG12**（32 位高分辨率，测频精度更高），
  模式 CAPTURE，上升沿捕获，输入引脚选空闲 GPIO；
- `GATE_TIMER`：1 s 周期定时器（TIMG6/7），软件闸门计数，等精度测频。

**怎么用**：

```c
#include "sysconfig.h"
#include "freq_meter.h"

FreqMeter_Init();                       // 捕获通道 + 1 s 闸门
if (FreqMeter_Poll(&freqHz, &periodNs)) {
    USR_UART_printf("F=%.3f Hz\r\n", freqHz);   // 等精度测频
}
```

### P0-3 恒压恒流（CV/CC）双环 PID 库——D 题电源类必需 ★

**为什么加**：现有 `pid_controller` 是单环。D 题要求开路 30 V（波动 ≤50 mV）、
短路 2.2 A（波动 ≤50 mA）、0.1~2.1 A 恒流段，是标准 CV/CC 结构：电压环 + 电流环
+ 取小者输出 + 伏安曲线参考生成。

**新增 `psu_control` 库（复用现有 PID/ADC/DAC）**：

```c
#include "sysconfig.h"
#include "psu_control.h"

Psu_Config cfg = {
    .vrefTargetV = 30.0f,      // 开路电压
    .irefTargetA = 2.2f,       // 短路电流
    .curveMode   = CURVE_PV,   // 光伏伏安特性
};
Psu_Init(&cfg);                          // 启动控制节拍（TIMG 中断，20 kHz）
// 每个控制周期：
float v = Adc_ReadVoltage();             // ADC0 -> PA22
float i = Adc_ReadCurrent();             // ADC1 -> PA21
Psu_Update(v, i, &dacCode);              // CV/CC 切换 + 双 PID
DAC_SetCode12((uint16_t) dacCode);       // DAC0 -> PA15 驱动功率级
```

> 硬件提示：效率 ≥95% 由功率级（Buck 拓扑 + 低阻 MOS + 电感）决定，MCU 只做
> 控制环和显示；50 mV/50 mA 波动要求控制节拍 ≥10~20 kHz，需实测 TIMG0 采样
> 与控制中断的时序，必要时提高采样率并验证（HANDOFF 10.3：当前只验证 12.8 kHz）。

### P0-4 PWM 输出库（TIMA0/TIMA1）——B/D 两题通用

**为什么加**：B 题 D 类功放需要"音频样本 → PWM 占空比"，D 题若改用 PWM 直驱
功率级也需要；TIMA0（4 路 CCP）/TIMA1（2 路）目前完全空闲。

**怎么用**：

```c
#include "sysconfig.h"
#include "pwm_out.h"

PwmOut_Init();                              // SYSCFG_DL_PWM_TIMER_init()
PwmOut_SetFreqHz(PWM_TIMER_INST, 20000U);   // 20 kHz
PwmOut_SetDuty(PWM_TIMER_INST, 0.5f);
PwmOut_Start(PWM_TIMER_INST);
// D 类功放：PwmOut_WriteSample(int16_t sample, uint16_t fullScale);
```

### P0-5 菜单 / UI 状态机库——现场操作通用

```c
#include "sysconfig.h"
#include "menu.h"

Menu_RegisterPage(0, "V",  Menu_ShowVoltage);
Menu_RegisterPage(1, "I",  Menu_ShowCurrent);
Menu_RegisterPage(2, "SET", Menu_EditSetpoint);
Menu_RunLoop();                 // 周期调用：Key_Poll + OLED 刷新
```

---

## 3. 工程现状与储备方向的契合点

工程里已有大量"电参数测量/电源"类痕迹，说明你（或团队）实际储备方向就是这块：

- `MeterRecord`（timestamp / meter_id / meter_addr / current_ma / status / crc16）
  → 多表计量、电流采集、欠载/过载判断；
- `ADDR0..3` 地址码 + `serial_protocol` 测量帧 + CRC16 → 多设备寻址/通信；
- `PA13` 无线电源控制、`PA14` ADC 模拟 MOS 开关 → 电源/负载通道控制；
- `analog_measure`（RMS/峰峰值/THD/相位）+ `gain_calibration` + `analog_calibration`
  → 测量精度链已就绪。

这条线对应赛题方向的优先级：**D 类电源题（CV/CC）≈ A 类仪器仪表（信号源+测频）>
B 类音频（PWM）**。建议按这个顺序投入。

## 4. 必须先处理的工程问题

1. **scatter 修正**：启用 `GainCalibration_SaveX100()` 前把
   `project/mspm0g3507.sct` 的 `0x0001FC00` 改为 `0x0001F800`；
2. **替换 main.c**：当前是时钟测试入口（PB14 方波 + PA10 0x55），含
   `SysTick_Handler()`，集成业务时必须删除/接管；
3. 新定时器避开 TIMG0（ADC 采样）与 TIMG8（低功耗保留），新中断不与用户库
   已提供的中断重名（HANDOFF 10.5）；
4. 若做电源闭环，ADC 采样率与控制节拍要先上板验证（HANDOFF 10.3 明确
   4 MHz 上限未验证）；
5. `ad9833.c` 保持禁用桩即可（无对应硬件）；`wave_gen.c` 已启用
   `WAVE_SOURCE_AD9850` 路径，`ad9850.c` 已加入 Keil 工程并编译通过
   （0 Error / 0 Warning）。

## 5. 硬件适配核查（依据 ZHCUD44《MSP-LITO-G3507 评估模块》用户指南）

> 完整文档见 `F:\TI\pre\zhcud44_MSP-LITO-G3507_硬件资源与管脚适配总结.md`。
> 本 EVM：MSPM0G3507（LQFP64），无板载调试器（SWD 仅走 J3，必须外接
> XDS110/J-Link），板载 3.3 V，32.768 kHz LFXT + 40 MHz HFXT + 100 kΩ ROSC(R9)。

### 5.1 按键/LED（已确认，无需改动）

- **用户已核对实物：LED = PB14、S2 按键 = PB21**，工程 `KEY_LED` 组配置正确；
- 注：ZHCUD44 官方原理图文字标注为 PB14=按键/PB21=LED，与实物相反，
  以实物为准，后续开发不要再按文档文字调整这两个引脚。

### 5.2 与硬件文档一致（无需改动）的工程配置

| 工程 | ZHCUD44 | 结论 |
|---|---|---|
| UART0 PA10/11 | 调试器反向通道（经 J3 直连 PC） | 调试串口即它 |
| OLED 软 I2C PA0/1 | 板载 2.2 kΩ 上拉（R7/R8） | 无需外接上拉 |
| ROSC PA2 | R9=100 kΩ 0.1% | 与 Sysclk_Init 外部 FCL 一致 |
| VREF 内部 2.5V / PA23=VREF+ | PA23=VREF+，板上 C8 1µF DNC | 建议外接 1 µF 去耦 |
| ADC0 PA22=A0_7 / ADC1 PA21=A1_7 | 一致 | 当前采样通道正确 |
| DAC PA15 | DAC_OUT（兼 A1_0/OPA 输入） | 用 DAC 时勿做模拟输入 |

### 5.3 硬件文档揭示、工程尚未使用的资源（模拟赛重点）

| 资源 | 能力 | 建议 |
|---|---|---|
| OPA0/OPA1 可编程增益运放 | 增益 1/2/4/8/16/32、反相 −1/−3/−7/−15/−31，输出片内直连 ADC0 ch13 / ADC1 ch13 | 加 OPA 库做信号调理（小信号放大/电流检测），零外部器件 |
| GPAMP | 通用放大器，输出 ADC ch14 | 同上，备用 |
| 3× 高速 COMP（内置基准 DAC） | 过零/阈值检测、触发 ADC/定时器 | 加 COMP 库：测频、测相位、过压/欠压保护 |
| RTC（LFXT 32.768 kHz 板载） | 报警/日历 | `MeterRecord.timestamp_s` 需要它；加 RTC 库 |
| TIMG12（32 位高分辨率定时器） | 测频/测时精度更高 | 等精度测频优先用它（TIMG6/7 为 16 位备用） |
| ADC 补充通道 | OPA→ch13、GPAMP→ch14、温度 ch11/12、电源监测 ch15、A0_12=PA14 | 可扩展模拟输入 |
| 40 MHz HFXT（板上 Y2） | 高频时钟源 | 方波发生器（P0-1）频率精度更高时使能 |

### 5.4 引脚分配（以此为准）

- **AD9850 DDS（已内置驱动）→ DDS_CTRL 组：PA28(W_CLK) / PA29(FQ_UD) /
  PA30(DATA) / PA24(RESET)**，全部空闲，已在 `.syscfg` 登记并重新生成；
  FQ_UD/DATA/W_CLK/RESET 均软件控制，不占用硬件 SPI；
- 方波发生器（P0-1）优先 **TIMG7**，测频捕获（P0-2）优先 **TIMG12**；
- PWM（P0-4）用 **TIMA0/TIMA1**（4 路 + 2 路 CCP）；
- OLED 若升级硬件 I2C → I2C0 用 PA0/PA1（板上已有上拉）；
- SPI0/SPI1 保持空闲（如需接外部 ADC/Flash 再分配，SPI1 组 PB6/PB7/PB8/PB9
  全空闲最省事；**不要**用 SPI0 的 PA12/13/14 组——PA13/14 已被工程用作
  无线电源/MOS 控制，且该组与 CAN 冲突）；
- 新定时器全部避开 TIMG0（ADC 采样）与 TIMG8（低功耗保留）。

## 6. 建议的投入顺序

1. **P0-2 测频库（TIMG12）**：纯片内、工程小、A 题核心，先做打通"测"的链路；
2. **P0-1 片内信号源（DAC 扫频增强 + TIMG7 方波）**：复用现有 `dac.c`，
   只加扫频封装和方波库，覆盖 A 题激励；
3. **P0-3 CV/CC 双环**：D 题电源核心，工程量大，建议放到前面硬件调通后做；
4. **P0-4 PWM + P0-5 菜单**：通用件，穿插做；
5. **顺手加 OPA/COMP/RTC 库**（5.3）：零外部器件，模拟赛收益高。

## 7. 已实现库清单（2026-08-15 完成，Keil 0 Error / 0 Warning）

以下库已写入工程并通过编译，接口见对应头文件：

| 库 | 头文件 | 资源/引脚 | 说明 |
|---|---|---|---|
| **freq_meter**（测频） | `usr/inc/freq_meter.h` | TIMG12 捕获 PB13 | 32 位边沿捕获等精度测频，`FreqMeter_Read()` 读频率 |
| **square_gen**（方波源） | `usr/inc/square_gen.h` | TIMG7 PWM PB15 | 1 Hz ~ 约 1 MHz 方波，`SquareGen_SetFreqHz()` |
| **pwm_out**（通用 PWM） | `usr/inc/pwm_out.h` | TIMA0 CCP1 PA9 | 可调频/占空比，含 D 类功放样本映射 |
| **psu_control**（CV/CC 双环） | `usr/inc/psu_control.h` | TIMG6 20 kHz 节拍 + DAC PA15 | 电压环/电流环取小切换 + 光伏曲线，复用 pid_controller |
| **menu**（菜单状态机） | `usr/inc/menu.h` | KEY1-5 + OLED | 多页菜单、编辑模式、`Menu_RunLoop(elapsedMs)` |
| **dac 扫频增强** | `usr/inc/dac.h` | DAC PA15 | `DAC_SweepSine(start, stop, ratio, dwell, cb)` |
| **usr_rtc**（时间戳） | `usr/inc/usr_rtc.h` | RTC（LFCLK） | `UsrRtc_GetTime()` / `UsrRtc_GetUnixSeconds2000()`，供 MeterRecord |
| **usr_opa**（可编程增益） | `usr/inc/usr_opa.h` | OPA1：IN0+=PB19、OUT=PA16 | 2x~32x 非反相 PGA，运行时改增益 |
| **usr_comp**（阈值比较） | `usr/inc/usr_comp.h` | COMP1：IN0+=PB26、IN0-=PB27 | VDDA 基准 8 位 DAC 阈值，过零/过压检测 |
| **ad9850**（掉落 DDS） | `usr/inc/ad9850.h` | PA28/29/30/24（软串行） | AD9850/AD9851 软件串行 DDS，已接入 wave_gen |

**引脚占用核对**（全部为空闲脚，与 ZHCUD44 一致）：PB13 测频、PB15 方波、
PA9 PWM、PB19/PA16 OPA1、PB26/PB27 COMP1、PA28/29/30/24 AD9850、PA16 OPA 输出。
新中断所有权：`TIMG12_IRQHandler`（freq_meter）、`TIMG6_IRQHandler`（psu_control）
已由库提供，业务代码不得重名。

**使用示例（组合）**：

```c
#include "sysconfig.h"
#include "freq_meter.h"
#include "square_gen.h"
#include "dac.h"
#include "usr_rtc.h"

Sysclk_Init();
SquareGen_Init(); SquareGen_SetFreqHz(1000000U); SquareGen_Start();  // 1MHz 方波激励
FreqMeter_Init();                                                     // 测运放输出频率
// 主循环：
FreqMeterResult fm;
if (FreqMeter_Read(&fm)) USR_UART_printf("F=%.3fHz\r\n", fm.frequencyHz);
```
