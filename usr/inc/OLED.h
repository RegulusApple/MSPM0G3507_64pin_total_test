#ifndef __OLED_H
#define __OLED_H

#include <stdbool.h>
#include <stdint.h>

void OLED_Init(void);
bool OLED_IsPresent(void);
uint8_t OLED_GetAddress(void);
void OLED_DisplayOn(void);
void OLED_DisplayOff(void);
void OLED_SetAllPixels(bool enabled);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, const char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowFloat(uint8_t Line, uint8_t Column, float Value,
    uint8_t DecimalPlaces);
void OLED_ShowProgress(uint8_t Line, uint8_t Column, uint8_t Percent);

#endif
