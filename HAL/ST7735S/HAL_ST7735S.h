/*
 * HAL_ST7735S.h
 *
 * ST7735S 1.8" SPI TFT driver (RGB565).
 */

#ifndef HAL_ST7735S_H
#define HAL_ST7735S_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "PLATFORM_TYPES.h"
#include "mcal_gpio.h"
#include "MCAL_SPI.h"

/* ================================================================ */
/* =========================== Macros ============================= */
/* ================================================================ */
#define HAL_ST7735S_COLOR_BLACK   (0x0000U)
#define HAL_ST7735S_COLOR_WHITE   (0xFFFFU)
#define HAL_ST7735S_COLOR_RED     (0xF800U)
#define HAL_ST7735S_COLOR_GREEN   (0x07E0U)
#define HAL_ST7735S_COLOR_BLUE    (0x001FU)
#define HAL_ST7735S_COLOR_YELLOW  (0xFFE0U)
#define HAL_ST7735S_COLOR_CYAN    (0x07FFU)
#define HAL_ST7735S_COLOR_MAGENTA (0xF81FU)
#define HAL_ST7735S_COLOR_GRAY    (0x8410U)

/* ================================================================ */
/* ======================= Data Structures ======================== */
/* ================================================================ */
typedef enum
{
    HAL_ST7735S_STATUS_OK = 0U,
    HAL_ST7735S_STATUS_ERROR,
    HAL_ST7735S_STATUS_INVALID_PARAM,
    HAL_ST7735S_STATUS_TIMEOUT
} hal_st7735s_status_t;

typedef struct
{
    GPIO_RegDef_t *port;
    mcal_gpio_pin_t pin;
} hal_st7735s_pin_t;

typedef void (*hal_st7735s_delay_ms_t)(uint32 milliseconds);

typedef struct
{
    SPI_RegDef_t *spi_port;
    hal_st7735s_pin_t cs_pin;
    hal_st7735s_pin_t dc_pin;
    hal_st7735s_pin_t rst_pin;
    hal_st7735s_pin_t bl_pin;

    uint16 width;
    uint16 height;
    uint16 col_offset;
    uint16 row_offset;
    uint8 rotation;
    uint8 invert;

    hal_st7735s_delay_ms_t delay_ms;
} hal_st7735s_config_t;

typedef struct
{
    SPI_RegDef_t *spi_port;
    hal_st7735s_pin_t cs_pin;
    hal_st7735s_pin_t dc_pin;
    hal_st7735s_pin_t rst_pin;
    hal_st7735s_pin_t bl_pin;

    uint16 width;
    uint16 height;
    uint16 col_offset;
    uint16 row_offset;
    uint8 rotation;
    uint8 invert;

    uint8 is_initialized;
    hal_st7735s_delay_ms_t delay_ms;
} hal_st7735s_t;

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
hal_st7735s_status_t HAL_ST7735S_Init(hal_st7735s_t *lcd, const hal_st7735s_config_t *config);
void HAL_ST7735S_SetRotation(hal_st7735s_t *lcd, uint8 rotation);

void HAL_ST7735S_DrawPixel(hal_st7735s_t *lcd, uint16 x, uint16 y, uint16 color);
void HAL_ST7735S_FillScreen(hal_st7735s_t *lcd, uint16 color);
void HAL_ST7735S_FillRect(hal_st7735s_t *lcd, uint16 x, uint16 y, uint16 w, uint16 h, uint16 color);
void HAL_ST7735S_DrawRect(hal_st7735s_t *lcd, uint16 x, uint16 y, uint16 w, uint16 h, uint16 color);
void HAL_ST7735S_DrawPanel(hal_st7735s_t *lcd,
                           uint16 x,
                           uint16 y,
                           uint16 w,
                           uint16 h,
                           uint16 border,
                           uint16 fill,
                           uint16 highlight);

void HAL_ST7735S_DrawChar(hal_st7735s_t *lcd,
                          uint16 x,
                          uint16 y,
                          char ch,
                          uint16 color,
                          uint16 bg);
void HAL_ST7735S_DrawString(hal_st7735s_t *lcd,
                            uint16 x,
                            uint16 y,
                            const char *text,
                            uint16 color,
                            uint16 bg);

#endif
