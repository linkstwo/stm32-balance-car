#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "stm32f10x.h"
#include "board_config.h"
#include "soft_i2c.h"
#include "timebase.h"
#include "oledfont.h"
#include "ssd1306.h"

#define OLED_ADDRESS_7BIT 0x3CU
#define OLED_WIDTH 128U
#define OLED_PAGES 8U
#define OLED_FRAMEBUFFER_SIZE (OLED_WIDTH * OLED_PAGES)

static SoftI2cBus g_oled_bus = { GPIOB, GPIO_Pin_8, GPIO_Pin_9, 3U };
static uint8_t g_framebuffer[OLED_FRAMEBUFFER_SIZE];

static bool Oled_WriteCommand(const uint8_t *commands, uint8_t length)
{
    uint8_t buffer[33];
    uint8_t index;

    if (length > 32U)
    {
        return false;
    }
    buffer[0] = 0x00U;
    for (index = 0U; index < length; ++index)
    {
        buffer[index + 1U] = commands[index];
    }
    return SoftI2c_Write(&g_oled_bus, OLED_ADDRESS_7BIT, buffer, (uint16_t)length + 1U) == SOFT_I2C_OK;
}

static void Oled_DrawCharacter(uint8_t column, uint8_t page, char character)
{
    uint8_t font_index;
    uint8_t index;
    uint16_t offset;

    if ((column > (OLED_WIDTH - 6U)) || (page >= OLED_PAGES))
    {
        return;
    }
    if ((character < ' ') || (character > '~'))
    {
        character = '?';
    }
    font_index = (uint8_t)(character - ' ');
    offset = (uint16_t)page * OLED_WIDTH + column;
    for (index = 0U; index < 6U; ++index)
    {
        g_framebuffer[offset + index] = F6x8[font_index][index];
    }
}

bool Oled_Init(void)
{
    static const uint8_t init_commands[] = {
        0xAEU, 0x20U, 0x00U, 0x40U, BOARD_OLED_SEGMENT_REMAP_COMMAND,
        BOARD_OLED_COM_SCAN_COMMAND, 0xA6U, 0xA8U,
        0x3FU, 0xD3U, 0x00U, 0xD5U, 0x80U, 0xD9U, 0xF1U, 0xDAU,
        0x12U, 0xDBU, 0x30U, 0x8DU, 0x14U, 0xAFU
    };

    SoftI2c_Init(&g_oled_bus);
    /* Initialization-only delay; no display delay is used after App_Init. */
    Timebase_DelayMs(20U);
    if (!Oled_WriteCommand(init_commands, sizeof(init_commands)))
    {
        return false;
    }
    Oled_Clear();
    return Oled_Flush();
}

void Oled_Clear(void)
{
    memset(g_framebuffer, 0, sizeof(g_framebuffer));
}

void Oled_DrawText(uint8_t column, uint8_t page, const char *text)
{
    while ((*text != '\0') && (column <= (OLED_WIDTH - 6U)))
    {
        Oled_DrawCharacter(column, page, *text);
        column = (uint8_t)(column + 6U);
        ++text;
    }
}

void Oled_DrawInt(uint8_t column, uint8_t page, int32_t value)
{
    char text[12];
    uint8_t length = 0U;
    uint32_t magnitude;

    if (value < 0)
    {
        text[length++] = '-';
        magnitude = (uint32_t)(-value);
    }
    else
    {
        magnitude = (uint32_t)value;
    }
    do
    {
        text[length++] = (char)('0' + (magnitude % 10U));
        magnitude /= 10U;
    } while ((magnitude != 0U) && (length < (sizeof(text) - 1U)));
    text[length] = '\0';

    if (text[0] == '-')
    {
        uint8_t index;
        for (index = 1U; index < length; ++index)
        {
            char swap = text[index];
            text[index] = text[length - index];
            text[length - index] = swap;
        }
    }
    else
    {
        uint8_t start = 0U;
        uint8_t end = (uint8_t)(length - 1U);
        while (start < end)
        {
            char swap = text[start];
            text[start++] = text[end];
            text[end--] = swap;
        }
    }
    Oled_DrawText(column, page, text);
}

void Oled_DrawFloat(uint8_t column, uint8_t page, float value, uint8_t decimals)
{
    int32_t scale = 1;
    int32_t scaled;
    uint8_t index;
    char text[16];
    uint8_t length;

    if (decimals > 3U)
    {
        decimals = 3U;
    }
    for (index = 0U; index < decimals; ++index)
    {
        scale *= 10;
    }
    scaled = (int32_t)(value * (float)scale + ((value >= 0.0f) ? 0.5f : -0.5f));
    length = 0U;
    if (scaled < 0)
    {
        text[length++] = '-';
        scaled = -scaled;
    }
    if (decimals == 0U)
    {
        Oled_DrawInt(column, page, (int32_t)(scaled / scale));
        return;
    }
    {
        int32_t whole = scaled / scale;
        int32_t fraction = scaled % scale;
        char whole_text[12];
        uint8_t whole_length = 0U;
        do
        {
            whole_text[whole_length++] = (char)('0' + (whole % 10));
            whole /= 10;
        } while ((whole != 0) && (whole_length < sizeof(whole_text)));
        while (whole_length > 0U)
        {
            text[length++] = whole_text[--whole_length];
        }
        text[length++] = '.';
        for (index = 0U; index < decimals; ++index)
        {
            scale /= 10;
            text[length++] = (char)('0' + ((fraction / scale) % 10));
        }
        text[length] = '\0';
    }
    Oled_DrawText(column, page, text);
}

bool Oled_FlushPage(uint8_t page)
{
    uint8_t commands[3];
    uint8_t data[OLED_WIDTH + 1U];

    if (page >= OLED_PAGES)
    {
        return false;
    }
    data[0] = 0x40U;
    commands[0] = (uint8_t)(0xB0U | page);
    commands[1] = 0x00U;
    commands[2] = 0x10U;
    if (!Oled_WriteCommand(commands, sizeof(commands)))
    {
        return false;
    }
    memcpy(&data[1], &g_framebuffer[(uint16_t)page * OLED_WIDTH], OLED_WIDTH);
    return SoftI2c_Write(&g_oled_bus, OLED_ADDRESS_7BIT, data, sizeof(data)) == SOFT_I2C_OK;
}

bool Oled_Flush(void)
{
    uint8_t page;

    for (page = 0U; page < OLED_PAGES; ++page)
    {
        if (!Oled_FlushPage(page))
        {
            return false;
        }
    }
    return true;
}
