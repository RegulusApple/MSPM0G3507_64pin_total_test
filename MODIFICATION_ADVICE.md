# MSPM0G3507 用户库 — TI 杯模拟专题赛修改意见（MODIFICATION_ADVICE）

更新日期：2026-08-15
工程目录：`F:\TI\MSPM0G3507_64pin_total_test`
配套文档：`HANDOFF.md`（交付说明）

## 0. 现状一句话评估

本工程已经是一个相当完整的"信号测量分析平台"：双通道 ADC + DMA 定时采样、
RMS / 频率 / THD / 相位分析、1024 点 FFT、UART、OLED、DAC 正弦输出、PID、
Flash 记录、增益校准、按键。**直接支撑"波形分析仪 / 数字电表 / 信号源"一类
模拟专题赛题目的测量侧**。下面按"必修修正 → 高价值新增库 → 锦上添花"给出意见。

---

## 1. 必修修正（不修会踩坑，先做这些）

### 1.1 scatter 文件 Flash 预留区必须修正（启用增益校准时）

- 现状：`project/mspm0g3507.sct` 程序区只到 `0x0001FC00`，只预留最后 1 KB；
- `gain_calibration.c` 需要 `0x0001F800` 起的 1 KB，`flash.c` 需要 `0x0001FC00` 起的 1 KB；
- **启用 `GainCalibration_SaveX100()` 前**，把 `0x0001FC00` 全部改成 `0x0001F800`
  （见 HANDOFF 10.2），否则校准保存会覆盖用户程序区。

### 1.2 替换测试入口 main.c

当前 `main.c` 是时钟测试入口（PB14 方波 + PA10 发 0x55）。集成业务时必须删除，
并**接管/删除 `SysTick_Handler()`**（HANDOFF 10.5）。业务入口最先只调用一次
`Sysclk_Init()`。

### 1.3 波形生成模块取舍

- 方案 A（推荐，最省事）：保留 `dac.c`，从 Keil 工程排除 `wave_gen.c` 和 `ad9833.c`；
- 方案 B：若要恢复 AD9833 类外部 DDS，需要重新分配 SPI 引脚（PA13/PA14 已被
  板级控制占用，见下文 P1-4）。

---

## 2. 高价值新增库（建议优先做）

> 通用原则：新增外设一律先在 `mspm0_g3507.syscfg` 里加模块 → 用指定版本
> SysConfig 重新生成 → 使用生成的宏（如 `SPI0_INST`、`I2C0_INST`），
> 不要手写物理引脚值。宏名以生成结果为准，下面示例里的名字是预期命名风格。

### P1-1 定时器输入捕获库（等精度测频 / 测周期 / 测脉宽）★最推荐

**为什么加**：模拟赛"频率计 / 周期测量"几乎必考。现在测频只能靠 ADC 采样 +
FFT/Goertzel（受 12.8 kHz 采样率限制，只能测约 6.4 kHz 以下且精度低）。
用定时器输入捕获可以把测频上限做到 MHz 级、精度到 0.01% 量级。

**怎么配**（SysConfig）：
- `TIMG6`：输入捕获通道（CCP0，边沿触发），捕获外部信号边沿 → 计数 `Nx`；
- `TIMG7`（或 TIMG6 另一通道）：对高频基准时钟（如 32 MHz MCLK）计数 → `Ns`；
- 软件闸门（如 100 ms）：`f = Nx / Ns * Fref`，即等精度测频。

**怎么用**（伪代码，宏名以生成为准）：

```c
#include "sysconfig.h"
#include "freq_meter.h"   // 新增库

int main(void)
{
    Sysclk_Init();
    FreqMeter_Init();                       // 使能捕获通道 + 闸门定时器
    while (1) {
        if (FreqMeter_Poll(&freqHz, &periodUs)) {
            USR_UART_printf("F=%.4fHz, T=%.2fus\r\n", freqHz, periodUs);
        }
    }
}
```

注意：TIMG0 已被 ADC 采样定时占用，TIMG8 保留给低功耗，请用 TIMG6/TIMG7/
TIMG12/TIMA0/TIMA1。

### P1-2 PWM 输出库（可调频可调占空比方波源 / 电路控制）

**为什么加**：题目若涉及"程控方波输出、自动增益控制（DAC/开关控制）、蜂鸣器、
LED 调光、加热/电机"，都需要硬件 PWM。现在工程没有 PWM 库。

**怎么配**：SysConfig 加 `TIMER` 实例（推荐 `TIMA0`，4 路 CCP），模式
"PWM generate"，频率/初始占空比设好，引脚任选空闲 GPIO。

**怎么用**：

```c
#include "sysconfig.h"
#include "pwm_out.h"      // 新增库

PwmOut_Init();                                    // SYSCFG 已生成 PWM 配置
PwmOut_SetFreqHz(1000U);                          // 改频率
PwmOut_SetDutyPercent(0.5f);                      // 50% 占空比
PwmOut_Start(); PwmOut_Stop();
```

### P1-3 硬件 I2C 库（I2C0 / I2C1）

**为什么加**：现在 OLED 用的是**软件 I2C**（阻塞、占用 CPU、速度慢）。硬件 I2C
可驱动 OLED/传感器/EEPROM，且不阻塞主循环。注意：若要把 PA0/PA1 从软 I2C 换到
硬件 I2C0，需在 SysConfig 中把 `I2C_LCD_PINS` GPIO 组替换为 `I2C` 模块并确认
引脚复用允许（以 SysConfig 求解结果为准），不要同时保留软 I2C 的 GPIO 输出驱动。

**怎么用**：

```c
#include "sysconfig.h"
#include "i2c_master.h"   // 新增库

I2cMaster_Init(400000U);                          // 400 kHz
I2cMaster_WriteReg(0x3C, 0x00, &buf, 1);          // 例：写 OLED 命令
I2cMaster_ReadReg(0x48, 0x00, &val, 1);           // 例：读温度传感器
```

### P1-4 硬件 SPI 库（SPI0 / SPI1）— 恢复 AD9833 或接外部 ADC/DAC

**为什么加**：
- 若要**恢复 AD9833**（HANDOFF 里因 PA13/PA14 占用而禁用），需要重新选 SPI
  引脚（空闲 GPIO 很多），把 `ad9833.c` 从"禁用桩"改成真实驱动，即可获得
  MHz 级正弦信号源——这是"信号发生器"题目的关键器件；
- 或者接 SPI OLED/TFT、外部高速 ADC（如 ADS1115/AD7606）、SPI Flash。

**怎么配**：SysConfig 加 `SPI` 实例（SPI0 或 SPI1），主机模式，速率按器件选
（AD9833 一般 10~25 MHz SCK），引脚分配空闲 GPIO。

**怎么用（以恢复 AD9833 为例）**：

```c
#include "sysconfig.h"
#include "ad9833.h"       // 重新启用真实驱动

Ad9833_Init();                                   // SPI 已由 SYSCFG 生成
Ad9833_SetWave(WAVE_SINE, 1000000U, 200U);       // 1 MHz 正弦，200 mV 幅度档
```

### P1-5 OLED 菜单 / UI 状态机框架

**为什么加**：比赛现场需要按键切换显示页（测量值 / 频谱 / 标定 / 波形参数），
裸机下用状态机菜单最省事。现有 `key.c` + `OLED.c` 足够支撑。

**怎么用**：

```c
#include "sysconfig.h"
#include "menu.h"         // 新增库

Menu_RegisterPage(0, "RMS",   Menu_ShowRms);
Menu_RegisterPage(1, "SPECT", Menu_ShowSpectrum);
Menu_RegisterPage(2, "CAL",   Menu_ShowCal);
Menu_RunLoop();           // 内部轮询按键 + 刷新 OLED，周期调用即可
```

---

## 3. 锦上添花（时间充裕再做）

| 库 | 作用 | 说明 |
|---|---|---|
| 频谱峰值/谐波搜索 | 在现有 `SP_myFFT` 上做峰值搜索、谐波列表、基波锁定 | 提高测量观感 |
| MATHACL 加速 | G3507 有硬件数学加速器，RMS/FFT/除法提速 | `dl_mathacl.h`，可显著降 CPU 占用 |
| 软件看门狗 | 比赛现场长时间运行防死机 | `dl_iwdt.h`，超时喂狗 |
| ms 级系统节拍 | 统一时间基准（现在只有 `SystemTime_DelayUs`） | 便于任务调度和超时判断 |
| FreeRTOS | 不建议，裸机 + 定时器中断已够 | 引入后所有库要重审中断上下文 |

---

## 4. 集成范式（新增任何库都按这个流程）

1. 改 `mspm0_g3507.syscfg`（加模块/引脚）→ 用 SysConfig CLI 重新生成
   （命令见 HANDOFF 第 4 节）→ 确认 `ti_msp_dl_config.c/.h` 变化符合预期；
2. 新库源文件放入 `usr/src/`，头文件放入 `usr/inc/`，在 Keil 工程里添加并配好
   包含路径；
3. 业务入口：`Sysclk_Init()` → 各库 `*_Init()` → 主循环/状态机；
4. 中断函数所有权：UART/ADC 中断只能由用户库提供（HANDOFF 10.5），新库新增
   中断（如捕获/PWM）注意不要重名；
5. 重新生成 + 全量编译，目标 `0 Errors, 0 Warnings`，再上板逐项验收。

---

## 5. 建议的下一步

- 告诉我本届赛题方向（信号源？波形测量？电参数测量？），可针对性裁剪；
- 我可以直接动手添加以上某个库（含 `.syscfg` 修改 + 重新生成 + 示例 main）。
