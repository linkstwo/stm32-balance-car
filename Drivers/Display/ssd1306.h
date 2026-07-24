#ifndef DRIVERS_DISPLAY_SSD1306_H
#define DRIVERS_DISPLAY_SSD1306_H

#include <stdbool.h>
#include <stdint.h>

bool Oled_Init(void);
void Oled_Clear(void);
void Oled_DrawText(uint8_t column, uint8_t page, const char *text);
void Oled_DrawInt(uint8_t column, uint8_t page, int32_t value);
void Oled_DrawFloat(uint8_t column, uint8_t page, float value, uint8_t decimals);
bool Oled_FlushPage(uint8_t page);
bool Oled_Flush(void);

#endif
