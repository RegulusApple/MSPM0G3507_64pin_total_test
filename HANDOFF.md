# MSPM0G3507_64pin_total_test 交付文档

> 交付版本：2026-08-16  
> 工程目录：`F:\TI\MSPM0G3507_64pin_total_test`  
> 依据：当前工作区源码、SysConfig 配置、Keil 工程文件和最近构建日志

## 1. 交付结论

本工程为 MSPM0G3507 64-pin 裸机测量与模拟外设验证工程，已集成 ADC/DMA、信号分析、DAC、OLED、UART、低功耗、Flash 记录、校准、输入捕获、PWM、RTC、OPA、COMP、PSU 控制和 AD9850 备用驱动。

当前 `main.c` 是四项硬件自测入口，不是最终产品业务入口。接手者可以直接打开 Keil 工程编译，然后按本文的验收顺序进行上板验证。

当前已确认：

- Keil 工程结构完整，目标设备为 `MSPM0G3507`；
- 最近的 `uv4_all_libs_build.log`、`uv4_selftest_build.log`、`uv4_test4_build.log` 均为 `0 Error(s), 0 Warning(s)`；
- SysConfig 生成文件与工程中使用的实例宏一致；
- 当前没有进行真实硬件下载、示波器测量或信号源精度验收。

## 2. 工具链与工程入口

| 项目 | 当前配置 |
|---|---|
| MCU | TI MSPM0G3507，LQFP-64 |
| IDE/编译器 | Keil uVision，ArmClang 6.22 |
| SysConfig | 1.20.0+3587 |
| SDK | MSPM0 SDK 2.01.00.03 |
| 软件架构 | 裸机，无 FreeRTOS |
| 系统时钟 | SYSOSC/MCLK/BUSCLK/CPUCLK 约 32 MHz |
| 低频时钟 | 板载 LFXT，32.768 kHz，用于 RTC 和低功耗唤醒 |
| Keil 工程 | `project\mspm0_damo.uvprojx` |
| 编译目标 | `MSPM0G3507` |
| SysConfig 源文件 | `mspm0_g3507.syscfg` |
| 生成文件 | `ti_msp_dl_config.c`、`ti_msp_dl_config.h` |
| 链接文件 | `project\mspm0g3507.sct` |

Keil 工程已设置 Before Make 动作调用 `tools\keil\syscfg.bat` 生成 SysConfig 输出。修改外设、引脚、时钟、中断或 DMA 时，应先改 `.syscfg`，再重新生成并编译。

## 3. 目录与职责

```text
main.c                         当前四项上板测试入口
mspm0_g3507.syscfg             SysConfig 唯一配置源
ti_msp_dl_config.c/.h          SysConfig 生成文件，禁止手改
project/mspm0_damo.uvprojx     Keil 工程入口
project/mspm0g3507.sct         Flash/RAM 布局
usr/inc/                       用户库头文件
usr/src/                       用户库实现
source/ti/driverlib/           TI DriverLib 源码
tools/keil/                    SysConfig/Keil 辅助文件
tools/bsl/                     MSPM0 BSL 串口下载工具和示例
tests/                         主机侧 Python 检查脚本
docs/                          设计说明和迁移记录
```

主要用户库：

| 模块 | 作用 |
|---|---|
| `adc2chSample` | ADC0/ADC1 双通道采样、DMA、采样率调整、RMS/频率/相位接口 |
| `signalProcess`、`signalProcess_ext` | FFT、Goertzel、滤波、频率、相位、THD 和功率计算 |
| `analog_measure`、`analog_calibration`、`gain_calibration` | 模拟量换算和增益校准 |
| `dac`、`wave_gen` | DAC 输出、正弦波和扫频；`wave_gen` 也支持 AD9850 路径 |
| `OLED`、`soft_i2c`、`menu` | 软件 I2C OLED、显示和菜单状态机 |
| `usr_uart`、`serial_protocol` | UART0 调试输出、接收缓存和二进制测量帧 |
| `low_power` | ADC/VREF/UART/无线控制和定时器唤醒流程 |
| `flash`、`addr_code` | 测量记录持久化和板卡地址码 |
| `freq_meter` | TIMG12 输入捕获测频 |
| `square_gen`、`pwm_out` | 方波和通用 PWM 输出 |
| `psu_control`、`pid_controller` | 20 kHz CV/CC 控制节拍和 PID |
| `usr_rtc`、`usr_opa`、`usr_comp` | RTC、OPA1 PGA 和 COMP1 阈值比较 |
| `ad9850` | 外接 AD9850 软件串行控制，需真实模块和接线 |

## 4. 当前 SysConfig 资源和引脚

| 资源 | 当前配置 | 用途 |
|---|---|---|
| ADC0 | PA22，VREF 2.5 V，DMA CH0 | 模拟输入通道 0 |
| ADC1 | PA21，VREF 2.5 V，DMA CH1 | 模拟输入通道 1 |
| VREF | PA23 | 外部参考/板级模拟参考 |
| DAC0 | PA15，DMA CH2 | DAC 波形输出 |
| UART0 | PA10 TX、PA11 RX，115200 | 调试和协议输出 |
| OLED 软件 I2C | PA0 SDA、PA1 SCL | 外接 OLED，地址自动探测 0x3C/0x3D |
| LED/按键 | PB14 LED、PB21 按键 | 板级状态和自测控制 |
| 普通按键 | PA8、PB3、PB2、PA7、PA31 | KEY1~KEY5 |
| 地址码 | PA27、PA25、PB24、PB18 | ADDR0~ADDR3，下拉输入 |
| ADC 采样定时器 | TIMG0，78.125 us 默认周期 | ADC 触发事件 |
| 低功耗唤醒定时器 | TIMG8，LFCLK，500 ms | 低功耗唤醒 |
| 测频捕获 | TIMG12 CCP0，PB13 | PERIOD_CAPTURE 输入 |
| 方波输出 | TIMG7 CCP0，PB15 | 方波信号源 |
| 通用 PWM | TIMA0 CCP1，PA9 | PWM 输出 |
| PSU 控制定时器 | TIMG6，50 us，20 kHz | CV/CC 控制节拍 |
| RTC | RTC + LFXT | 时间戳和走时测试 |
| OPA1 | IN0+=PB19，OUT=PA16 | PGA 放大 |
| COMP1 | IN0+=PB26，IN0-=PB27 | 阈值比较 |
| AD9850 控制 | PA28 W_CLK、PA29 FQ_UD、PA30 DATA、PA24 RESET | 外接 DDS 备用接口 |

引脚以 `ti_msp_dl_config.h` 中的生成宏为准，不要依据手写数字直接调用 DriverLib。

## 5. 当前运行入口

`main.c` 的初始化顺序为：

```c
Sysclk_Init();
USR_UART_init();
OLED_Init();
UsrRtc_Init();
```

板载按键 PB21 的行为：

- 短按：在 FREQ、RTC、PSU、ADC 四项之间切换；
- 长按约 0.8 秒：执行当前测试；
- UART0 PA10 以 115200 输出结果；
- OLED 显示当前测试项和按键电平。

四项测试：

| 测试 | 外部条件 | 主要判定 |
|---|---|---|
| FREQ | 信号源接 PB13，输入 0~3.3 V 数字电平 | UART 输出捕获周期和频率 |
| RTC | 无需外接 | LFXT 就绪且秒数持续递增 |
| PSU | 无需外接 | TIMG6 运行，500 ms 约产生 10000 个 tick |
| ADC | 信号源接 ADC0 对应 PA22 | 单帧 100 kHz 采样，输出 RMS/频率 |

具体接头编号和板级注意事项见 [`SELFTEST_WIRING.md`](SELFTEST_WIRING.md)。原理图脚位存在待确认项时，上板前以丝印和万用表通断结果为准。

## 6. 编译与下载

### 编译

1. 用 Keil 打开 `project\mspm0_damo.uvprojx`。
2. 选择目标 `MSPM0G3507`。
3. 执行 Rebuild 或 Build。
4. 确认生成文件没有被手工修改，最终输出在 `project\Objects\`。

最近一次扩展库测试构建记录：

| 日志 | 结果 | 代码/只读数据/RW/ZI |
|---|---|---|
| `project\uv4_all_libs_build.log` | 0 Error，0 Warning | 9400 / 616 / 0 / 1024 |
| `project\uv4_selftest_build.log` | 0 Error，0 Warning | 27992 / 3384 / 4 / 3220 |
| `project\uv4_test4_build.log` | 0 Error，0 Warning | 30792 / 4408 / 20 / 6476 |

### 下载

本工程交接时未执行下载。推荐使用与工程配置匹配的 J-Link、XDS110/DSLite 或 MSPM0 BSL。禁止使用 ST-LINK 给 MSPM0 下载或调试；该硬件路径可能导致器件锁死或出现 PDSC 错误。

### 串口

UART0 使用 PA10/PA11，波特率 115200，通常为 8-N-1。打开串口工具后复位，可看到：

```text
=== MSPM0G3507 4-item test ===
FCL=..., CPUCLK=...
OLED=present/absent
```

## 7. 接手者必须先处理的事项

### 7.1 产品化前替换 `main.c`

当前 `main.c` 只用于外设自测。产品入口应保留一次 `Sysclk_Init()`，然后按需初始化用户库和任务循环。不要在业务代码中重复调用 `SYSCFG_DL_init()` 或 `Sysclk_Init()`。

### 7.2 增益校准的 Flash 区冲突

当前链接文件只把代码区限制到 `0x0001FC00`，并为 `flash.c` 的测量记录保留 `0x0001FC00` 起的 1 KB；但 `gain_calibration.c` 使用 `0x0001F800` 起的另一个 Flash sector。

在启用 `GainCalibration_SaveX100()` 前，必须将 `project\mspm0g3507.sct` 中的程序区终点从 `0x0001FC00` 调整为 `0x0001F800`，确保校准区和测量记录区均不被程序覆盖。修改后必须重新链接并检查 map 文件。

### 7.3 LFXT 启动

SysConfig 当前设置 `validateClkStatus=false`，初始化不会无限等待 LFXT。RTC 使用前应检查 `UsrRtc_IsClockReady()`；若晶振未起振，RTC 读数不会正常递增。

### 7.4 外接 AD9850

`BOARD_AD9850_AVAILABLE` 当前为 1，且 `wave_gen` 会保留四个 GPIO。没有真实 AD9850 模块时，可将该宏改为 0，并从 Keil 工程排除 `ad9850.c`/相关路径，释放 GPIO 供其他功能使用。

## 8. 中断和生成代码约束

当前生成宏对应的中断资源包括：

- `UART_DEBUG_INST_IRQHandler`：UART0 调试接收；
- `ADC12_0_INST_IRQHandler`、`ADC12_1_INST_IRQHandler`：ADC DMA 完成；
- `ADC_SAMPLE_TIMER_INST_IRQHandler`：TIMG0 ADC 采样定时器；
- `FREQ_CAPTURE_INST_IRQHandler`：TIMG12 测频捕获；
- `PSU_CTRL_TIMER_INST_IRQHandler`：TIMG6 PSU 控制节拍；
- `LP_WAKE_TIMER_INST_IRQHandler`：TIMG8 低功耗唤醒。

使用对应用户库时，不要在应用中再次定义同名 ISR。任何新增中断都必须先检查 `ti_msp_dl_config.h` 的实例宏和 IRQ 名称。

以下文件属于生成或构建产物，不应直接编辑：

- `ti_msp_dl_config.c`、`ti_msp_dl_config.h`；
- `project\Objects\` 下的 `.o`、`.d`、`.axf`、`.hex`；
- `project\*.map`、Keil 用户状态文件和临时构建日志。

## 9. 验收顺序

1. Keil Rebuild，确认 0 Error/0 Warning。
2. 使用匹配的调试器下载并复位。
3. 打开 UART0，确认启动信息和 500 ms 心跳持续输出。
4. 短按 PB21，确认四个测试项可以循环切换。
5. 先验收 RTC 和 PSU 这两项无外接信号测试。
6. 再验收 PB13 测频和 PA22 ADC 高速采样。
7. 最后按需验收 OLED、DAC、OPA、COMP、PWM、AD9850 和低功耗流程。
8. 若启用 Flash 校准，先完成第 7.2 节链接区修正，再做擦写和掉电保持测试。

## 10. 当前未闭环问题

- 未在本次交接中进行真实板卡下载；
- 未用示波器确认 PB13 捕获精度、PB15/PA9 PWM 输出和 PA15 DAC 波形；
- 未确认 LFXT 在目标板上的实际起振和 RTC 长时间走时；
- ADC 100 kHz 高速采样仅有源码和构建级验证，仍需信号源实测；
- OPA/COMP/AD9850 需要依据实际模块、电源和接线做硬件验收；
- `check_syscfg.py` 静态检查因当前 Windows Python 启动器会话错误未能运行，不能把该脚本检查标记为通过。

## 11. 相关文档

- [`HANDOFF.md`](HANDOFF.md)：已有扩展交付说明；
- [`SELFTEST_WIRING.md`](SELFTEST_WIRING.md)：自测接线和板级检查；
- [`CONTEST_PREP_ADVICE.md`](CONTEST_PREP_ADVICE.md)：竞赛准备建议；
- [`MODIFICATION_ADVICE.md`](MODIFICATION_ADVICE.md)：修改建议和风险项；
- [`docs/superpowers/specs/2026-07-22-msp430-usr-to-mspm0g3507-design.md`](docs/superpowers/specs/2026-07-22-msp430-usr-to-mspm0g3507-design.md)：迁移设计说明。

