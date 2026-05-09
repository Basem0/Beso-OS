/*
 * HAL_ST7735S.c
 *
 * ST7735S 1.8" SPI TFT driver (RGB565).
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "HAL_ST7735S.h"
#include <stddef.h>

/* ================================================================ */
/* =========================== Macros ============================= */
/* ================================================================ */
#define ST7735S_CMD_NOP     (0x00U)
#define ST7735S_CMD_SWRESET (0x01U)
#define ST7735S_CMD_SLPIN   (0x10U)
#define ST7735S_CMD_SLPOUT  (0x11U)
#define ST7735S_CMD_INVOFF  (0x20U)
#define ST7735S_CMD_INVON   (0x21U)
#define ST7735S_CMD_COLMOD  (0x3AU)
#define ST7735S_CMD_MADCTL  (0x36U)
#define ST7735S_CMD_CASET   (0x2AU)
#define ST7735S_CMD_RASET   (0x2BU)
#define ST7735S_CMD_RAMWR   (0x2CU)
#define ST7735S_CMD_GMCTRP1 (0xE0U)
#define ST7735S_CMD_GMCTRN1 (0xE1U)
#define ST7735S_CMD_FRMCTR1 (0xB1U)
#define ST7735S_CMD_FRMCTR2 (0xB2U)
#define ST7735S_CMD_FRMCTR3 (0xB3U)
#define ST7735S_CMD_INVCTR  (0xB4U)
#define ST7735S_CMD_PWCTR1  (0xC0U)
#define ST7735S_CMD_PWCTR2  (0xC1U)
#define ST7735S_CMD_PWCTR3  (0xC2U)
#define ST7735S_CMD_PWCTR4  (0xC3U)
#define ST7735S_CMD_PWCTR5  (0xC4U)
#define ST7735S_CMD_VMCTR1  (0xC5U)
#define ST7735S_CMD_DISPON  (0x29U)

#define ST7735S_MADCTL_MX   (0x40U)
#define ST7735S_MADCTL_MY   (0x80U)
#define ST7735S_MADCTL_MV   (0x20U)
#define ST7735S_MADCTL_RGB  (0x00U)

#define ST7735S_FONT_WIDTH  (5U)
#define ST7735S_FONT_HEIGHT (7U)

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
static const uint8 g_st7735s_font5x7[96][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* ' ' */
    {0x00,0x00,0x5F,0x00,0x00}, /* '!' */
    {0x00,0x07,0x00,0x07,0x00}, /* '"' */
    {0x14,0x7F,0x14,0x7F,0x14}, /* '#' */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* '$' */
    {0x23,0x13,0x08,0x64,0x62}, /* '%' */
    {0x36,0x49,0x55,0x22,0x50}, /* '&' */
    {0x00,0x05,0x03,0x00,0x00}, /* '\'' */
    {0x00,0x1C,0x22,0x41,0x00}, /* '(' */
    {0x00,0x41,0x22,0x1C,0x00}, /* ')' */
    {0x14,0x08,0x3E,0x08,0x14}, /* '*' */
    {0x08,0x08,0x3E,0x08,0x08}, /* '+' */
    {0x00,0x50,0x30,0x00,0x00}, /* ',' */
    {0x08,0x08,0x08,0x08,0x08}, /* '-' */
    {0x00,0x60,0x60,0x00,0x00}, /* '.' */
    {0x20,0x10,0x08,0x04,0x02}, /* '/' */
    {0x3E,0x51,0x49,0x45,0x3E}, /* '0' */
    {0x00,0x42,0x7F,0x40,0x00}, /* '1' */
    {0x42,0x61,0x51,0x49,0x46}, /* '2' */
    {0x21,0x41,0x45,0x4B,0x31}, /* '3' */
    {0x18,0x14,0x12,0x7F,0x10}, /* '4' */
    {0x27,0x45,0x45,0x45,0x39}, /* '5' */
    {0x3C,0x4A,0x49,0x49,0x30}, /* '6' */
    {0x01,0x71,0x09,0x05,0x03}, /* '7' */
    {0x36,0x49,0x49,0x49,0x36}, /* '8' */
    {0x06,0x49,0x49,0x29,0x1E}, /* '9' */
    {0x00,0x36,0x36,0x00,0x00}, /* ':' */
    {0x00,0x56,0x36,0x00,0x00}, /* ';' */
    {0x08,0x14,0x22,0x41,0x00}, /* '<' */
    {0x14,0x14,0x14,0x14,0x14}, /* '=' */
    {0x00,0x41,0x22,0x14,0x08}, /* '>' */
    {0x02,0x01,0x51,0x09,0x06}, /* '?' */
    {0x32,0x49,0x79,0x41,0x3E}, /* '@' */
    {0x7E,0x11,0x11,0x11,0x7E}, /* 'A' */
    {0x7F,0x49,0x49,0x49,0x36}, /* 'B' */
    {0x3E,0x41,0x41,0x41,0x22}, /* 'C' */
    {0x7F,0x41,0x41,0x22,0x1C}, /* 'D' */
    {0x7F,0x49,0x49,0x49,0x41}, /* 'E' */
    {0x7F,0x09,0x09,0x09,0x01}, /* 'F' */
    {0x3E,0x41,0x49,0x49,0x7A}, /* 'G' */
    {0x7F,0x08,0x08,0x08,0x7F}, /* 'H' */
    {0x00,0x41,0x7F,0x41,0x00}, /* 'I' */
    {0x20,0x40,0x41,0x3F,0x01}, /* 'J' */
    {0x7F,0x08,0x14,0x22,0x41}, /* 'K' */
    {0x7F,0x40,0x40,0x40,0x40}, /* 'L' */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* 'M' */
    {0x7F,0x04,0x08,0x10,0x7F}, /* 'N' */
    {0x3E,0x41,0x41,0x41,0x3E}, /* 'O' */
    {0x7F,0x09,0x09,0x09,0x06}, /* 'P' */
    {0x3E,0x41,0x51,0x21,0x5E}, /* 'Q' */
    {0x7F,0x09,0x19,0x29,0x46}, /* 'R' */
    {0x46,0x49,0x49,0x49,0x31}, /* 'S' */
    {0x01,0x01,0x7F,0x01,0x01}, /* 'T' */
    {0x3F,0x40,0x40,0x40,0x3F}, /* 'U' */
    {0x1F,0x20,0x40,0x20,0x1F}, /* 'V' */
    {0x7F,0x20,0x18,0x20,0x7F}, /* 'W' */
    {0x63,0x14,0x08,0x14,0x63}, /* 'X' */
    {0x03,0x04,0x78,0x04,0x03}, /* 'Y' */
    {0x61,0x51,0x49,0x45,0x43}, /* 'Z' */
    {0x00,0x7F,0x41,0x41,0x00}, /* '[' */
    {0x02,0x04,0x08,0x10,0x20}, /* '\\' */
    {0x00,0x41,0x41,0x7F,0x00}, /* ']' */
    {0x04,0x02,0x01,0x02,0x04}, /* '^' */
    {0x40,0x40,0x40,0x40,0x40}, /* '_' */
    {0x00,0x03,0x05,0x00,0x00}, /* '`' */
    {0x20,0x54,0x54,0x54,0x78}, /* 'a' */
    {0x7F,0x48,0x44,0x44,0x38}, /* 'b' */
    {0x38,0x44,0x44,0x44,0x20}, /* 'c' */
    {0x38,0x44,0x44,0x48,0x7F}, /* 'd' */
    {0x38,0x54,0x54,0x54,0x18}, /* 'e' */
    {0x08,0x7E,0x09,0x01,0x02}, /* 'f' */
    {0x0C,0x52,0x52,0x52,0x3E}, /* 'g' */
    {0x7F,0x08,0x04,0x04,0x78}, /* 'h' */
    {0x00,0x44,0x7D,0x40,0x00}, /* 'i' */
    {0x20,0x40,0x44,0x3D,0x00}, /* 'j' */
    {0x7F,0x10,0x28,0x44,0x00}, /* 'k' */
    {0x00,0x41,0x7F,0x40,0x00}, /* 'l' */
    {0x7C,0x04,0x18,0x04,0x78}, /* 'm' */
    {0x7C,0x08,0x04,0x04,0x78}, /* 'n' */
    {0x38,0x44,0x44,0x44,0x38}, /* 'o' */
    {0x7C,0x14,0x14,0x14,0x08}, /* 'p' */
    {0x08,0x14,0x14,0x18,0x7C}, /* 'q' */
    {0x7C,0x08,0x04,0x04,0x08}, /* 'r' */
    {0x48,0x54,0x54,0x54,0x20}, /* 's' */
    {0x04,0x3F,0x44,0x40,0x20}, /* 't' */
    {0x3C,0x40,0x40,0x20,0x7C}, /* 'u' */
    {0x1C,0x20,0x40,0x20,0x1C}, /* 'v' */
    {0x3C,0x40,0x30,0x40,0x3C}, /* 'w' */
    {0x44,0x28,0x10,0x28,0x44}, /* 'x' */
    {0x0C,0x50,0x50,0x50,0x3C}, /* 'y' */
    {0x44,0x64,0x54,0x4C,0x44}, /* 'z' */
    {0x00,0x08,0x36,0x41,0x00}, /* '{' */
    {0x00,0x00,0x7F,0x00,0x00}, /* '|' */
    {0x00,0x41,0x36,0x08,0x00}, /* '}' */
    {0x10,0x08,0x08,0x10,0x08}, /* '~' */
    {0x00,0x00,0x00,0x00,0x00}  /* DEL */
};

static uint8 HAL_ST7735S_IsPinValid(const hal_st7735s_pin_t *pin)
{
    return (uint8)((pin != NULL) && (pin->port != NULL) && (pin->pin <= MCAL_GPIO_PIN_15));
}

static void HAL_ST7735S_Select(const hal_st7735s_t *lcd)
{
    MCAL_GPIO_WritePin(lcd->cs_pin.port, lcd->cs_pin.pin, GPIO_PIN_RESET);
}

static void HAL_ST7735S_Deselect(const hal_st7735s_t *lcd)
{
    MCAL_GPIO_WritePin(lcd->cs_pin.port, lcd->cs_pin.pin, GPIO_PIN_SET);
}

static void HAL_ST7735S_WriteCommand(const hal_st7735s_t *lcd, uint8 cmd)
{
    MCAL_GPIO_WritePin(lcd->dc_pin.port, lcd->dc_pin.pin, GPIO_PIN_RESET);
    HAL_ST7735S_Select(lcd);
    (void)MCAL_SPI_SendData(lcd->spi_port, &cmd, 1U);
    HAL_ST7735S_Deselect(lcd);
}

static void HAL_ST7735S_WriteData(const hal_st7735s_t *lcd, const uint8 *data, uint16 length)
{
    if ((data == NULL) || (length == 0U))
    {
        return;
    }

    MCAL_GPIO_WritePin(lcd->dc_pin.port, lcd->dc_pin.pin, GPIO_PIN_SET);
    HAL_ST7735S_Select(lcd);
    (void)MCAL_SPI_SendData(lcd->spi_port, data, length);
    HAL_ST7735S_Deselect(lcd);
}

static void HAL_ST7735S_Reset(const hal_st7735s_t *lcd)
{
    if (!HAL_ST7735S_IsPinValid(&lcd->rst_pin))
    {
        return;
    }

    MCAL_GPIO_WritePin(lcd->rst_pin.port, lcd->rst_pin.pin, GPIO_PIN_RESET);
    if (lcd->delay_ms != NULL)
    {
        lcd->delay_ms(10U);
    }
    MCAL_GPIO_WritePin(lcd->rst_pin.port, lcd->rst_pin.pin, GPIO_PIN_SET);
    if (lcd->delay_ms != NULL)
    {
        lcd->delay_ms(120U);
    }
}

static void HAL_ST7735S_SetAddressWindow(hal_st7735s_t *lcd,
                                        uint16 x0,
                                        uint16 y0,
                                        uint16 x1,
                                        uint16 y1)
{
    uint8 data[4];
    uint16 col_start = (uint16)(x0 + lcd->col_offset);
    uint16 col_end = (uint16)(x1 + lcd->col_offset);
    uint16 row_start = (uint16)(y0 + lcd->row_offset);
    uint16 row_end = (uint16)(y1 + lcd->row_offset);

    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_CASET);
    data[0] = (uint8)(col_start >> 8);
    data[1] = (uint8)(col_start & 0xFFU);
    data[2] = (uint8)(col_end >> 8);
    data[3] = (uint8)(col_end & 0xFFU);
    HAL_ST7735S_WriteData(lcd, data, 4U);

    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_RASET);
    data[0] = (uint8)(row_start >> 8);
    data[1] = (uint8)(row_start & 0xFFU);
    data[2] = (uint8)(row_end >> 8);
    data[3] = (uint8)(row_end & 0xFFU);
    HAL_ST7735S_WriteData(lcd, data, 4U);

    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_RAMWR);
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
hal_st7735s_status_t HAL_ST7735S_Init(hal_st7735s_t *lcd, const hal_st7735s_config_t *config)
{
    uint8 data[16];

    if ((lcd == NULL) || (config == NULL) || (config->delay_ms == NULL))
    {
        return HAL_ST7735S_STATUS_INVALID_PARAM;
    }

    if (!HAL_ST7735S_IsPinValid(&config->cs_pin) ||
        !HAL_ST7735S_IsPinValid(&config->dc_pin) ||
        !HAL_ST7735S_IsPinValid(&config->rst_pin))
    {
        return HAL_ST7735S_STATUS_INVALID_PARAM;
    }

    lcd->spi_port = config->spi_port;
    lcd->cs_pin = config->cs_pin;
    lcd->dc_pin = config->dc_pin;
    lcd->rst_pin = config->rst_pin;
    lcd->bl_pin = config->bl_pin;
    lcd->width = config->width;
    lcd->height = config->height;
    lcd->col_offset = config->col_offset;
    lcd->row_offset = config->row_offset;
    lcd->rotation = config->rotation;
    lcd->invert = config->invert;
    lcd->delay_ms = config->delay_ms;
    lcd->is_initialized = 0U;

    HAL_ST7735S_Deselect(lcd);
    HAL_ST7735S_Reset(lcd);

    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_SWRESET);
    lcd->delay_ms(150U);

    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_SLPOUT);
    lcd->delay_ms(120U);

    data[0] = 0x01U; data[1] = 0x2CU; data[2] = 0x2DU;
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_FRMCTR1);
    HAL_ST7735S_WriteData(lcd, data, 3U);

    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_FRMCTR2);
    HAL_ST7735S_WriteData(lcd, data, 3U);

    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_FRMCTR3);
    data[3] = 0x01U; data[4] = 0x2CU; data[5] = 0x2DU;
    HAL_ST7735S_WriteData(lcd, data, 6U);

    data[0] = 0x07U;
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_INVCTR);
    HAL_ST7735S_WriteData(lcd, data, 1U);

    data[0] = 0xA2U; data[1] = 0x02U; data[2] = 0x84U;
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_PWCTR1);
    HAL_ST7735S_WriteData(lcd, data, 3U);

    data[0] = 0xC5U;
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_PWCTR2);
    HAL_ST7735S_WriteData(lcd, data, 1U);

    data[0] = 0x0AU; data[1] = 0x00U;
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_PWCTR3);
    HAL_ST7735S_WriteData(lcd, data, 2U);

    data[0] = 0x8AU; data[1] = 0x2AU;
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_PWCTR4);
    HAL_ST7735S_WriteData(lcd, data, 2U);

    data[0] = 0x8AU; data[1] = 0xEEU;
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_PWCTR5);
    HAL_ST7735S_WriteData(lcd, data, 2U);

    data[0] = 0x0EU;
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_VMCTR1);
    HAL_ST7735S_WriteData(lcd, data, 1U);

    if (lcd->invert != 0U)
    {
        HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_INVON);
    }
    else
    {
        HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_INVOFF);
    }

    data[0] = 0x05U; /* 16-bit color */
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_COLMOD);
    HAL_ST7735S_WriteData(lcd, data, 1U);

    HAL_ST7735S_SetRotation(lcd, lcd->rotation);

    data[0] = 0x02U; data[1] = 0x1CU; data[2] = 0x07U; data[3] = 0x12U;
    data[4] = 0x37U; data[5] = 0x32U; data[6] = 0x29U; data[7] = 0x2DU;
    data[8] = 0x29U; data[9] = 0x25U; data[10] = 0x2BU; data[11] = 0x39U;
    data[12] = 0x00U; data[13] = 0x01U; data[14] = 0x03U; data[15] = 0x10U;
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_GMCTRP1);
    HAL_ST7735S_WriteData(lcd, data, 16U);

    data[0] = 0x03U; data[1] = 0x1DU; data[2] = 0x07U; data[3] = 0x06U;
    data[4] = 0x2EU; data[5] = 0x2CU; data[6] = 0x29U; data[7] = 0x2DU;
    data[8] = 0x2EU; data[9] = 0x2EU; data[10] = 0x37U; data[11] = 0x3FU;
    data[12] = 0x00U; data[13] = 0x00U; data[14] = 0x02U; data[15] = 0x10U;
    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_GMCTRN1);
    HAL_ST7735S_WriteData(lcd, data, 16U);

    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_DISPON);
    lcd->delay_ms(50U);

    if (HAL_ST7735S_IsPinValid(&lcd->bl_pin))
    {
        MCAL_GPIO_WritePin(lcd->bl_pin.port, lcd->bl_pin.pin, GPIO_PIN_SET);
    }

    lcd->is_initialized = 1U;
    return HAL_ST7735S_STATUS_OK;
}

void HAL_ST7735S_SetRotation(hal_st7735s_t *lcd, uint8 rotation)
{
    uint8 madctl = ST7735S_MADCTL_RGB;

    if (lcd == NULL)
    {
        return;
    }

    lcd->rotation = (uint8)(rotation % 4U);
    switch (lcd->rotation)
    {
        case 0U:
            madctl |= (uint8)(ST7735S_MADCTL_MX | ST7735S_MADCTL_MY);
            break;
        case 1U:
            madctl |= (uint8)(ST7735S_MADCTL_MY | ST7735S_MADCTL_MV);
            break;
        case 2U:
            madctl |= 0U;
            break;
        case 3U:
        default:
            madctl |= (uint8)(ST7735S_MADCTL_MX | ST7735S_MADCTL_MV);
            break;
    }

    HAL_ST7735S_WriteCommand(lcd, ST7735S_CMD_MADCTL);
    HAL_ST7735S_WriteData(lcd, &madctl, 1U);
}

void HAL_ST7735S_DrawPixel(hal_st7735s_t *lcd, uint16 x, uint16 y, uint16 color)
{
    uint8 data[2];

    if ((lcd == NULL) || (x >= lcd->width) || (y >= lcd->height))
    {
        return;
    }

    HAL_ST7735S_SetAddressWindow(lcd, x, y, x, y);
    data[0] = (uint8)(color >> 8);
    data[1] = (uint8)(color & 0xFFU);
    HAL_ST7735S_WriteData(lcd, data, 2U);
}

void HAL_ST7735S_FillRect(hal_st7735s_t *lcd, uint16 x, uint16 y, uint16 w, uint16 h, uint16 color)
{
    uint32 count;
    uint8 data[64];
    uint16 i;

    if ((lcd == NULL) || (w == 0U) || (h == 0U))
    {
        return;
    }

    if (x >= lcd->width || y >= lcd->height)
    {
        return;
    }

    if ((uint16)(x + w) > lcd->width)
    {
        w = (uint16)(lcd->width - x);
    }
    if ((uint16)(y + h) > lcd->height)
    {
        h = (uint16)(lcd->height - y);
    }

    HAL_ST7735S_SetAddressWindow(lcd, x, y, (uint16)(x + w - 1U), (uint16)(y + h - 1U));

    data[0] = (uint8)(color >> 8);
    data[1] = (uint8)(color & 0xFFU);
    for (i = 2U; i < sizeof(data); i += 2U)
    {
        data[i] = data[0];
        data[i + 1U] = data[1];
    }

    count = (uint32)w * (uint32)h;
    while (count > 0U)
    {
        uint16 chunk = (count > (sizeof(data) / 2U)) ? (uint16)(sizeof(data) / 2U) : (uint16)count;
        HAL_ST7735S_WriteData(lcd, data, (uint16)(chunk * 2U));
        count -= chunk;
    }
}

void HAL_ST7735S_FillScreen(hal_st7735s_t *lcd, uint16 color)
{
    HAL_ST7735S_FillRect(lcd, 0U, 0U, lcd->width, lcd->height, color);
}

void HAL_ST7735S_DrawRect(hal_st7735s_t *lcd, uint16 x, uint16 y, uint16 w, uint16 h, uint16 color)
{
    if ((lcd == NULL) || (w == 0U) || (h == 0U))
    {
        return;
    }

    HAL_ST7735S_FillRect(lcd, x, y, w, 1U, color);
    if (h > 1U)
    {
        HAL_ST7735S_FillRect(lcd, x, (uint16)(y + h - 1U), w, 1U, color);
    }
    if (h > 2U)
    {
        HAL_ST7735S_FillRect(lcd, x, (uint16)(y + 1U), 1U, (uint16)(h - 2U), color);
        if (w > 1U)
        {
            HAL_ST7735S_FillRect(lcd, (uint16)(x + w - 1U), (uint16)(y + 1U),
                                 1U, (uint16)(h - 2U), color);
        }
    }
}

void HAL_ST7735S_DrawPanel(hal_st7735s_t *lcd,
                           uint16 x,
                           uint16 y,
                           uint16 w,
                           uint16 h,
                           uint16 border,
                           uint16 fill,
                           uint16 highlight)
{
    if ((lcd == NULL) || (w == 0U) || (h == 0U))
    {
        return;
    }

    HAL_ST7735S_FillRect(lcd, x, y, w, h, border);
    if ((w > 4U) && (h > 4U))
    {
        HAL_ST7735S_FillRect(lcd, (uint16)(x + 1U), (uint16)(y + 1U),
                             (uint16)(w - 2U), (uint16)(h - 2U), fill);
        HAL_ST7735S_FillRect(lcd, (uint16)(x + 1U), (uint16)(y + 1U),
                             (uint16)(w - 2U), 1U, highlight);
    }
}

void HAL_ST7735S_DrawChar(hal_st7735s_t *lcd,
                          uint16 x,
                          uint16 y,
                          char ch,
                          uint16 color,
                          uint16 bg)
{
    uint8 col;
    uint8 row;
    uint8 line;
    uint8 idx;

    if ((lcd == NULL) || (ch < 32) || (ch > 127))
    {
        return;
    }

    idx = (uint8)(ch - 32);
    for (col = 0U; col < ST7735S_FONT_WIDTH; col++)
    {
        line = g_st7735s_font5x7[idx][col];
        for (row = 0U; row < ST7735S_FONT_HEIGHT; row++)
        {
            if ((line & 0x01U) != 0U)
            {
                HAL_ST7735S_DrawPixel(lcd, (uint16)(x + col), (uint16)(y + row), color);
            }
            else if (bg != color)
            {
                HAL_ST7735S_DrawPixel(lcd, (uint16)(x + col), (uint16)(y + row), bg);
            }
            line >>= 1U;
        }
    }

    if (bg != color)
    {
        for (row = 0U; row < ST7735S_FONT_HEIGHT; row++)
        {
            HAL_ST7735S_DrawPixel(lcd, (uint16)(x + ST7735S_FONT_WIDTH), (uint16)(y + row), bg);
        }
    }
}

void HAL_ST7735S_DrawString(hal_st7735s_t *lcd,
                            uint16 x,
                            uint16 y,
                            const char *text,
                            uint16 color,
                            uint16 bg)
{
    uint16 cursor_x = x;

    if ((lcd == NULL) || (text == NULL))
    {
        return;
    }

    while (*text != '\0')
    {
        HAL_ST7735S_DrawChar(lcd, cursor_x, y, *text, color, bg);
        cursor_x = (uint16)(cursor_x + ST7735S_FONT_WIDTH + 1U);
        text++;
    }
}
