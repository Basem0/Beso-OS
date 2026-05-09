/*
 * HAL_LCD.h
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

#ifndef HAL_LCD_H
#define HAL_LCD_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "PLATFORM_TYPES.h"
#include "mcal_gpio.h"

/* ================================================================ */
/* ========================== LCD Macros ========================== */
/* ================================================================ */
#define HAL_LCD_MAX_ROWS                        (4U)
#define HAL_LCD_MAX_CUSTOM_CHARS                (8U)

#define HAL_LCD_CMD_CLEAR_DISPLAY               (0x01U)
#define HAL_LCD_CMD_RETURN_HOME                 (0x02U)
#define HAL_LCD_CMD_ENTRY_MODE_SET              (0x04U)
#define HAL_LCD_CMD_DISPLAY_CONTROL             (0x08U)
#define HAL_LCD_CMD_CURSOR_SHIFT                (0x10U)
#define HAL_LCD_CMD_FUNCTION_SET                (0x20U)
#define HAL_LCD_CMD_SET_CGRAM_ADDR              (0x40U)
#define HAL_LCD_CMD_SET_DDRAM_ADDR              (0x80U)

#define HAL_LCD_FUNCTION_8BIT_FLAG              (0x10U)
#define HAL_LCD_FUNCTION_2LINE_FLAG             (0x08U)
#define HAL_LCD_FUNCTION_5X10DOTS_FLAG          (0x04U)

#define HAL_LCD_DISPLAY_ON_FLAG                 (0x04U)
#define HAL_LCD_CURSOR_ON_FLAG                  (0x02U)
#define HAL_LCD_BLINK_ON_FLAG                   (0x01U)

#define HAL_LCD_ENTRY_LEFT_FLAG                 (0x02U)
#define HAL_LCD_ENTRY_SHIFT_INC_FLAG            (0x01U)

#define HAL_LCD_SHIFT_DISPLAY_FLAG              (0x08U)
#define HAL_LCD_SHIFT_RIGHT_FLAG                (0x04U)

/* ================================================================ */
/* =============== Macros Configuration References ================ */
/* ================================================================ */
typedef enum
{
    HAL_LCD_STATUS_OK = 0U,
    HAL_LCD_STATUS_ERROR,
    HAL_LCD_STATUS_INVALID_PARAM
} hal_lcd_status_t;

typedef enum
{
    HAL_LCD_INTERFACE_4BIT = 0U,
    HAL_LCD_INTERFACE_8BIT
} hal_lcd_interface_t;

typedef enum
{
    HAL_LCD_LINES_1 = 0U,
    HAL_LCD_LINES_2
} hal_lcd_lines_t;

typedef enum
{
    HAL_LCD_FONT_5X8 = 0U,
    HAL_LCD_FONT_5X10
} hal_lcd_font_t;

typedef enum
{
    HAL_LCD_ENTRY_LEFT_TO_RIGHT = 0U,
    HAL_LCD_ENTRY_RIGHT_TO_LEFT
} hal_lcd_entry_direction_t;

typedef enum
{
    HAL_LCD_AUTOSCROLL_DISABLE = 0U,
    HAL_LCD_AUTOSCROLL_ENABLE
} hal_lcd_autoscroll_t;

typedef enum
{
    HAL_LCD_SHIFT_TARGET_CURSOR = 0U,
    HAL_LCD_SHIFT_TARGET_DISPLAY
} hal_lcd_shift_target_t;

typedef enum
{
    HAL_LCD_SHIFT_LEFT = 0U,
    HAL_LCD_SHIFT_RIGHT
} hal_lcd_shift_direction_t;

typedef struct
{
    GPIO_RegDef_t *port;
    mcal_gpio_pin_t pin;
} hal_lcd_pin_t;

typedef struct
{
    hal_lcd_interface_t interface_mode;
    hal_lcd_lines_t lines_mode;
    hal_lcd_font_t font_mode;
    mcal_gpio_speed_t gpio_speed;

    uint8 rows;
    uint8 columns;

    hal_lcd_pin_t rs_pin;
    hal_lcd_pin_t en_pin;

    uint8 rw_pin_used;
    hal_lcd_pin_t rw_pin;

    hal_lcd_pin_t data_pins[8];

    uint8 backlight_pin_used;
    hal_lcd_pin_t backlight_pin;

    uint8 display_control;
    uint8 entry_mode;
    uint8 function_set;
} hal_lcd_t;

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
hal_lcd_status_t HAL_LCD_Init(hal_lcd_t *lcd);
void HAL_LCD_DeInit(hal_lcd_t *lcd);

hal_lcd_status_t HAL_LCD_WriteCommand(hal_lcd_t *lcd, uint8 command);
hal_lcd_status_t HAL_LCD_WriteData(hal_lcd_t *lcd, uint8 data);

hal_lcd_status_t HAL_LCD_Clear(hal_lcd_t *lcd);
hal_lcd_status_t HAL_LCD_Home(hal_lcd_t *lcd);

hal_lcd_status_t HAL_LCD_SetCursor(hal_lcd_t *lcd, uint8 row, uint8 column);

hal_lcd_status_t HAL_LCD_WriteChar(hal_lcd_t *lcd, uint8 character);
hal_lcd_status_t HAL_LCD_WriteString(hal_lcd_t *lcd, const uint8 *string);
hal_lcd_status_t HAL_LCD_WriteStringAt(hal_lcd_t *lcd,
                                       uint8 row,
                                       uint8 column,
                                       const uint8 *string);

hal_lcd_status_t HAL_LCD_WriteUInt(hal_lcd_t *lcd, uint32 value);
hal_lcd_status_t HAL_LCD_WriteInt(hal_lcd_t *lcd, sint32 value);
hal_lcd_status_t HAL_LCD_WriteHex(hal_lcd_t *lcd, uint32 value);

hal_lcd_status_t HAL_LCD_CreateCustomChar(hal_lcd_t *lcd,
                                          uint8 location,
                                          const uint8 pattern[8]);
hal_lcd_status_t HAL_LCD_WriteCustomChar(hal_lcd_t *lcd, uint8 location);

hal_lcd_status_t HAL_LCD_DisplayOn(hal_lcd_t *lcd);
hal_lcd_status_t HAL_LCD_DisplayOff(hal_lcd_t *lcd);
hal_lcd_status_t HAL_LCD_CursorOn(hal_lcd_t *lcd);
hal_lcd_status_t HAL_LCD_CursorOff(hal_lcd_t *lcd);
hal_lcd_status_t HAL_LCD_BlinkOn(hal_lcd_t *lcd);
hal_lcd_status_t HAL_LCD_BlinkOff(hal_lcd_t *lcd);

hal_lcd_status_t HAL_LCD_SetEntryMode(hal_lcd_t *lcd,
                                      hal_lcd_entry_direction_t direction,
                                      hal_lcd_autoscroll_t auto_scroll);
hal_lcd_status_t HAL_LCD_AutoScrollOn(hal_lcd_t *lcd);
hal_lcd_status_t HAL_LCD_AutoScrollOff(hal_lcd_t *lcd);

hal_lcd_status_t HAL_LCD_Shift(hal_lcd_t *lcd,
                               hal_lcd_shift_target_t target,
                               hal_lcd_shift_direction_t direction);

hal_lcd_status_t HAL_LCD_BacklightOn(hal_lcd_t *lcd);
hal_lcd_status_t HAL_LCD_BacklightOff(hal_lcd_t *lcd);

#endif
