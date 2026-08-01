#include "ti_msp_dl_config.h"
#include "OLED.h"
#include "OLED_Font.h"
#include "soft_i2c.h"

#include "system_time.h"

#define OLED_I2C_ADDRESS_PRIMARY   (0x3CU)
#define OLED_I2C_ADDRESS_SECONDARY (0x3DU)
#define OLED_POWER_ON_DELAY_MS     (100U)

static bool gOledPresent;
static uint8_t gOledAddress;

/**
 * @brief  OLED写命令
 * @param  Command 要写入的命令
 * @retval 无
 */
void OLED_WriteCommand(uint8_t Command)
{
    uint8_t frame[2] = {0x00U, Command};

    if (gOledPresent &&
        (SoftI2C_Write(gOledAddress, frame, 2U) != SOFT_I2C_OK))
    {
        gOledPresent = false;
    }
}

/**
 * @brief  OLED写数据
 * @param  Data 要写入的数据
 * @retval 无
 */
void OLED_WriteData(uint8_t Data)
{
    uint8_t frame[2] = {0x40U, Data};

    if (gOledPresent &&
        (SoftI2C_Write(gOledAddress, frame, 2U) != SOFT_I2C_OK))
    {
        gOledPresent = false;
    }
}

/**
 * @brief  OLED设置光标位置
 * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
 * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
 * @retval 无
 */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0U | Y);
    OLED_WriteCommand(0x10U | ((X & 0xF0U) >> 4U));
    OLED_WriteCommand(0x00U | (X & 0x0FU));
}

/**
 * @brief  OLED清屏
 * @param  无
 * @retval 无
 */
void OLED_Clear(void)
{
    uint8_t i;
    uint8_t j;

    for (j = 0U; j < 8U; j++)
    {
        OLED_SetCursor(j, 0U);
        for (i = 0U; i < 128U; i++)
        {
            OLED_WriteData(0x00U);
        }
    }
}

/**
 * @brief  OLED显示一个字符
 * @param  Line 行位置，范围：1~4
 * @param  Column 列位置，范围：1~16
 * @param  Char 要显示的一个字符，范围：ASCII可见字符
 * @retval 无
 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;

    if ((Line < 1U) || (Line > 4U) ||
        (Column < 1U) || (Column > 16U))
    {
        return;
    }
    if ((Char < ' ') || (Char > '~'))
    {
        Char = '?';
    }

    OLED_SetCursor((uint8_t) ((Line - 1U) * 2U),
        (uint8_t) ((Column - 1U) * 8U));
    for (i = 0U; i < 8U; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);
    }

    OLED_SetCursor((uint8_t) (((Line - 1U) * 2U) + 1U),
        (uint8_t) ((Column - 1U) * 8U));
    for (i = 0U; i < 8U; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8U]);
    }
}

/**
 * @brief  OLED显示字符串
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  String 要显示的字符串，范围：ASCII可见字符
 * @retval 无
 */
void OLED_ShowString(uint8_t Line, uint8_t Column, const char *String)
{
    uint8_t i;

    if (String == NULL)
    {
        return;
    }

    for (i = 0U;
         (String[i] != '\0') && ((uint16_t) Column + i <= 16U);
         i++)
    {
        OLED_ShowChar(Line, (uint8_t) (Column + i), String[i]);
    }
}

/**
 * @brief  OLED次方函数
 * @retval 返回值等于X的Y次方
 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1U;

    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
 * @brief  OLED显示数字（十进制，正数）
 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;

    for (i = 0U; i < Length; i++)
    {
        OLED_ShowChar(Line, (uint8_t) (Column + i),
            (char) (Number / OLED_Pow(10U, Length - i - 1U) % 10U + '0'));
    }
}

/**
 * @brief  OLED显示数字（十进制，带符号数）
 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number,
    uint8_t Length)
{
    uint8_t i;
    uint32_t magnitude;

    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        magnitude = (uint32_t) Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        magnitude = (uint32_t) (-(int64_t) Number);
    }

    for (i = 0U; i < Length; i++)
    {
        OLED_ShowChar(Line, (uint8_t) (Column + i + 1U),
            (char) (magnitude / OLED_Pow(10U, Length - i - 1U) % 10U + '0'));
    }
}

/**
 * @brief  OLED显示数字（十六进制，正数）
 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number,
    uint8_t Length)
{
    uint8_t i;
    uint8_t digit;

    for (i = 0U; i < Length; i++)
    {
        digit = (uint8_t) (Number /
            OLED_Pow(16U, Length - i - 1U) % 16U);
        if (digit < 10U)
        {
            OLED_ShowChar(Line, (uint8_t) (Column + i),
                (char) (digit + '0'));
        }
        else
        {
            OLED_ShowChar(Line, (uint8_t) (Column + i),
                (char) (digit - 10U + 'A'));
        }
    }
}

/**
 * @brief  OLED显示数字（二进制，正数）
 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number,
    uint8_t Length)
{
    uint8_t i;

    for (i = 0U; i < Length; i++)
    {
        OLED_ShowChar(Line, (uint8_t) (Column + i),
            (char) (Number / OLED_Pow(2U, Length - i - 1U) % 2U + '0'));
    }
}

void OLED_ShowFloat(uint8_t Line, uint8_t Column, float Value,
    uint8_t DecimalPlaces)
{
    uint32_t scale = 1U;
    uint32_t scaled;
    uint32_t integerPart;
    uint32_t fractionPart;
    uint32_t temporary;
    uint8_t integerDigits = 1U;
    uint8_t i;

    if ((Line < 1U) || (Line > 4U) ||
        (Column < 1U) || (Column > 16U))
    {
        return;
    }
    if (DecimalPlaces > 6U)
    {
        DecimalPlaces = 6U;
    }
    for (i = 0U; i < DecimalPlaces; i++)
    {
        scale *= 10U;
    }

    if (Value < 0.0f)
    {
        OLED_ShowChar(Line, Column++, '-');
        Value = -Value;
    }
    else
    {
        OLED_ShowChar(Line, Column++, '+');
    }

    if (Value > ((float) UINT32_MAX / (float) scale))
    {
        OLED_ShowString(Line, Column, "OVF");
        return;
    }

    scaled = (uint32_t) ((Value * (float) scale) + 0.5f);
    integerPart = scaled / scale;
    fractionPart = scaled % scale;
    temporary = integerPart;
    while (temporary >= 10U)
    {
        temporary /= 10U;
        integerDigits++;
    }

    OLED_ShowNum(Line, Column, integerPart, integerDigits);
    Column = (uint8_t) (Column + integerDigits);
    if ((DecimalPlaces > 0U) && (Column <= 16U))
    {
        OLED_ShowChar(Line, Column++, '.');
        OLED_ShowNum(Line, Column, fractionPart, DecimalPlaces);
    }
}

void OLED_ShowProgress(uint8_t Line, uint8_t Column, uint8_t Percent)
{
    if (Percent > 100U)
    {
        Percent = 100U;
    }
    OLED_ShowNum(Line, Column, Percent, 3U);
    OLED_ShowChar(Line, (uint8_t) (Column + 3U), '%');
}

void OLED_DisplayOff(void)
{
    OLED_WriteCommand(0xAEU);
    OLED_WriteCommand(0x8DU);
    OLED_WriteCommand(0x10U);
}

void OLED_DisplayOn(void)
{
    OLED_WriteCommand(0x8DU);
    OLED_WriteCommand(0x14U);
    OLED_WriteCommand(0xAFU);
}

void OLED_SetAllPixels(bool enabled)
{
    OLED_WriteCommand(enabled ? 0xA5U : 0xA4U);
}

/**
 * @brief  OLED初始化
 * @param  无
 * @retval 无
 */
void OLED_Init(void)
{
    SystemTime_DelayMs(OLED_POWER_ON_DELAY_MS);

    SoftI2C_Init();
    gOledAddress = OLED_I2C_ADDRESS_PRIMARY;
    gOledPresent = (SoftI2C_Probe(gOledAddress) == SOFT_I2C_OK);
    if (!gOledPresent)
    {
        gOledAddress = OLED_I2C_ADDRESS_SECONDARY;
        gOledPresent = (SoftI2C_Probe(gOledAddress) == SOFT_I2C_OK);
    }
    if (!gOledPresent)
    {
        gOledAddress = 0U;
        return;
    }

    OLED_WriteCommand(0xAEU);
    OLED_WriteCommand(0xD5U);
    OLED_WriteCommand(0x80U);
    OLED_WriteCommand(0xA8U);
    OLED_WriteCommand(0x3FU);
    OLED_WriteCommand(0xD3U);
    OLED_WriteCommand(0x00U);
    OLED_WriteCommand(0x40U);
    OLED_WriteCommand(0xA1U);
    OLED_WriteCommand(0xC8U);
    OLED_WriteCommand(0xDAU);
    OLED_WriteCommand(0x12U);
    OLED_WriteCommand(0x81U);
    OLED_WriteCommand(0xCFU);
    OLED_WriteCommand(0xD9U);
    OLED_WriteCommand(0xF1U);
    OLED_WriteCommand(0xDBU);
    OLED_WriteCommand(0x30U);
    OLED_WriteCommand(0xA4U);
    OLED_WriteCommand(0xA6U);
    OLED_WriteCommand(0x8DU);
    OLED_WriteCommand(0x14U);
    OLED_DisplayOn();
    OLED_Clear();
}

bool OLED_IsPresent(void)
{
    return gOledPresent;
}

uint8_t OLED_GetAddress(void)
{
    return gOledAddress;
}
