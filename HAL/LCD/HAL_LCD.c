/*
 * HAL_LCD.c
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "HAL_LCD.h"

#include "MCAL_RCC.h"
#include "UTILS.h"

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
static const uint8 g_hal_lcd_row_offsets[HAL_LCD_MAX_ROWS] = {0x00U, 0x40U, 0x14U, 0x54U};

static hal_lcd_status_t HAL_LCD_EnableGPIOClock(GPIO_RegDef_t *gpio_port);
static hal_lcd_status_t HAL_LCD_InitPin(const hal_lcd_t *lcd, const hal_lcd_pin_t *pin);
static void HAL_LCD_DelayCycles(volatile uint32 cycles);
static void HAL_LCD_DelayUs(uint32 microseconds);
static void HAL_LCD_DelayMs(uint32 milliseconds);
static hal_lcd_status_t HAL_LCD_WritePinValue(const hal_lcd_pin_t *pin, mcal_gpio_pin_state_t value);
static hal_lcd_status_t HAL_LCD_PulseEnable(const hal_lcd_t *lcd);
static hal_lcd_status_t HAL_LCD_WriteNibble(const hal_lcd_t *lcd, uint8 nibble);
static hal_lcd_status_t HAL_LCD_WriteByte(const hal_lcd_t *lcd, uint8 value);
static hal_lcd_status_t HAL_LCD_ValidatePins(const hal_lcd_t *lcd);
static hal_lcd_status_t HAL_LCD_ApplyDisplayControl(hal_lcd_t *lcd);

/* ================================================================ */
/* ================= Static Function Definitions ================== */
/* ================================================================ */
static hal_lcd_status_t HAL_LCD_EnableGPIOClock(GPIO_RegDef_t *gpio_port)
{
    if (gpio_port == GPIOA)
    {
        MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_A);
        return HAL_LCD_STATUS_OK;
    }

    if (gpio_port == GPIOB)
    {
        MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_B);
        return HAL_LCD_STATUS_OK;
    }

    if (gpio_port == GPIOC)
    {
        MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_C);
        return HAL_LCD_STATUS_OK;
    }

    return HAL_LCD_STATUS_INVALID_PARAM;
}

static hal_lcd_status_t HAL_LCD_InitPin(const hal_lcd_t *lcd, const hal_lcd_pin_t *pin)
{
    hal_lcd_status_t status;

    if ((lcd == NULL) || (pin == NULL) || (pin->port == NULL) || (pin->pin > MCAL_GPIO_PIN_15))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    status = HAL_LCD_EnableGPIOClock(pin->port);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    MCAL_GPIO_Init(pin->port, pin->pin, MCAL_GPIO_MODE_OUTPUT_PP, lcd->gpio_speed);
    MCAL_GPIO_WritePin(pin->port, pin->pin, GPIO_PIN_RESET);

    return HAL_LCD_STATUS_OK;
}

static void HAL_LCD_DelayCycles(volatile uint32 cycles)
{
    while (cycles > 0U)
    {
        __asm volatile ("nop");
        cycles--;
    }
}

static void HAL_LCD_DelayUs(uint32 microseconds)
{
    while (microseconds > 0U)
    {
        HAL_LCD_DelayCycles(16U);
        microseconds--;
    }
}

static void HAL_LCD_DelayMs(uint32 milliseconds)
{
    while (milliseconds > 0U)
    {
        HAL_LCD_DelayUs(1000U);
        milliseconds--;
    }
}

static hal_lcd_status_t HAL_LCD_WritePinValue(const hal_lcd_pin_t *pin, mcal_gpio_pin_state_t value)
{
    if ((pin == NULL) || (pin->port == NULL) || (pin->pin > MCAL_GPIO_PIN_15))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    MCAL_GPIO_WritePin(pin->port, pin->pin, value);

    return HAL_LCD_STATUS_OK;
}

static hal_lcd_status_t HAL_LCD_PulseEnable(const hal_lcd_t *lcd)
{
    hal_lcd_status_t status;

    status = HAL_LCD_WritePinValue(&lcd->en_pin, GPIO_PIN_RESET);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    HAL_LCD_DelayUs(1U);

    status = HAL_LCD_WritePinValue(&lcd->en_pin, GPIO_PIN_SET);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    HAL_LCD_DelayUs(1U);

    status = HAL_LCD_WritePinValue(&lcd->en_pin, GPIO_PIN_RESET);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    HAL_LCD_DelayUs(100U);

    return HAL_LCD_STATUS_OK;
}

static hal_lcd_status_t HAL_LCD_WriteNibble(const hal_lcd_t *lcd, uint8 nibble)
{
    uint8 index;
    mcal_gpio_pin_state_t state;
    hal_lcd_status_t status;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    for (index = 0U; index < 4U; index++)
    {
        if (((nibble >> index) & 0x1U) == 0U)
        {
            state = GPIO_PIN_RESET;
        }
        else
        {
            state = GPIO_PIN_SET;
        }

        status = HAL_LCD_WritePinValue(&lcd->data_pins[index + 4U], state);
        if (status != HAL_LCD_STATUS_OK)
        {
            return status;
        }
    }

    return HAL_LCD_PulseEnable(lcd);
}

static hal_lcd_status_t HAL_LCD_WriteByte(const hal_lcd_t *lcd, uint8 value)
{
    uint8 index;
    mcal_gpio_pin_state_t state;
    hal_lcd_status_t status;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    if (lcd->interface_mode == HAL_LCD_INTERFACE_8BIT)
    {
        for (index = 0U; index < 8U; index++)
        {
            if (((value >> index) & 0x1U) == 0U)
            {
                state = GPIO_PIN_RESET;
            }
            else
            {
                state = GPIO_PIN_SET;
            }

            status = HAL_LCD_WritePinValue(&lcd->data_pins[index], state);
            if (status != HAL_LCD_STATUS_OK)
            {
                return status;
            }
        }

        return HAL_LCD_PulseEnable(lcd);
    }

    status = HAL_LCD_WriteNibble(lcd, (uint8)((value >> 4U) & 0x0FU));
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    status = HAL_LCD_WriteNibble(lcd, (uint8)(value & 0x0FU));
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    return HAL_LCD_STATUS_OK;
}

static hal_lcd_status_t HAL_LCD_ValidatePins(const hal_lcd_t *lcd)
{
    uint8 index;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    if ((lcd->rows == 0U) || (lcd->rows > HAL_LCD_MAX_ROWS) || (lcd->columns == 0U))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    if ((lcd->rs_pin.port == NULL) || (lcd->en_pin.port == NULL))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    if ((lcd->rw_pin_used != 0U) && (lcd->rw_pin.port == NULL))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    if ((lcd->backlight_pin_used != 0U) && (lcd->backlight_pin.port == NULL))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    if (lcd->interface_mode == HAL_LCD_INTERFACE_8BIT)
    {
        for (index = 0U; index < 8U; index++)
        {
            if (lcd->data_pins[index].port == NULL)
            {
                return HAL_LCD_STATUS_INVALID_PARAM;
            }
        }
    }
    else
    {
        for (index = 4U; index < 8U; index++)
        {
            if (lcd->data_pins[index].port == NULL)
            {
                return HAL_LCD_STATUS_INVALID_PARAM;
            }
        }
    }

    return HAL_LCD_STATUS_OK;
}

static hal_lcd_status_t HAL_LCD_ApplyDisplayControl(hal_lcd_t *lcd)
{
    return HAL_LCD_WriteCommand(lcd, (uint8)(HAL_LCD_CMD_DISPLAY_CONTROL | lcd->display_control));
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
hal_lcd_status_t HAL_LCD_Init(hal_lcd_t *lcd)
{
    uint8 index;
    hal_lcd_status_t status;

    status = HAL_LCD_ValidatePins(lcd);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    status = HAL_LCD_InitPin(lcd, &lcd->rs_pin);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    status = HAL_LCD_InitPin(lcd, &lcd->en_pin);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    if (lcd->rw_pin_used != 0U)
    {
        status = HAL_LCD_InitPin(lcd, &lcd->rw_pin);
        if (status != HAL_LCD_STATUS_OK)
        {
            return status;
        }
    }

    if (lcd->interface_mode == HAL_LCD_INTERFACE_8BIT)
    {
        for (index = 0U; index < 8U; index++)
        {
            status = HAL_LCD_InitPin(lcd, &lcd->data_pins[index]);
            if (status != HAL_LCD_STATUS_OK)
            {
                return status;
            }
        }
    }
    else
    {
        for (index = 4U; index < 8U; index++)
        {
            status = HAL_LCD_InitPin(lcd, &lcd->data_pins[index]);
            if (status != HAL_LCD_STATUS_OK)
            {
                return status;
            }
        }
    }

    if (lcd->backlight_pin_used != 0U)
    {
        status = HAL_LCD_InitPin(lcd, &lcd->backlight_pin);
        if (status != HAL_LCD_STATUS_OK)
        {
            return status;
        }
    }

    HAL_LCD_DelayMs(40U);

    HAL_LCD_WritePinValue(&lcd->rs_pin, GPIO_PIN_RESET);
    HAL_LCD_WritePinValue(&lcd->en_pin, GPIO_PIN_RESET);

    if (lcd->rw_pin_used != 0U)
    {
        HAL_LCD_WritePinValue(&lcd->rw_pin, GPIO_PIN_RESET);
    }

    if (lcd->interface_mode == HAL_LCD_INTERFACE_4BIT)
    {
        HAL_LCD_WriteNibble(lcd, 0x03U);
        HAL_LCD_DelayMs(5U);

        HAL_LCD_WriteNibble(lcd, 0x03U);
        HAL_LCD_DelayUs(150U);

        HAL_LCD_WriteNibble(lcd, 0x03U);
        HAL_LCD_DelayUs(150U);

        HAL_LCD_WriteNibble(lcd, 0x02U);
        HAL_LCD_DelayUs(150U);

        lcd->function_set = HAL_LCD_CMD_FUNCTION_SET;
    }
    else
    {
        lcd->function_set = (uint8)(HAL_LCD_CMD_FUNCTION_SET | HAL_LCD_FUNCTION_8BIT_FLAG);
    }

    if (lcd->lines_mode == HAL_LCD_LINES_2)
    {
        lcd->function_set |= HAL_LCD_FUNCTION_2LINE_FLAG;
    }

    if ((lcd->font_mode == HAL_LCD_FONT_5X10) && (lcd->lines_mode == HAL_LCD_LINES_1))
    {
        lcd->function_set |= HAL_LCD_FUNCTION_5X10DOTS_FLAG;
    }

    status = HAL_LCD_WriteCommand(lcd, lcd->function_set);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    lcd->display_control = HAL_LCD_DISPLAY_ON_FLAG;
    status = HAL_LCD_ApplyDisplayControl(lcd);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    status = HAL_LCD_Clear(lcd);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    status = HAL_LCD_SetEntryMode(lcd, HAL_LCD_ENTRY_LEFT_TO_RIGHT, HAL_LCD_AUTOSCROLL_DISABLE);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    if (lcd->backlight_pin_used != 0U)
    {
        status = HAL_LCD_BacklightOn(lcd);
        if (status != HAL_LCD_STATUS_OK)
        {
            return status;
        }
    }

    return HAL_LCD_STATUS_OK;
}

void HAL_LCD_DeInit(hal_lcd_t *lcd)
{
    if (lcd == NULL)
    {
        return;
    }

    HAL_LCD_DisplayOff(lcd);

    if (lcd->backlight_pin_used != 0U)
    {
        HAL_LCD_BacklightOff(lcd);
    }
}

hal_lcd_status_t HAL_LCD_WriteCommand(hal_lcd_t *lcd, uint8 command)
{
    hal_lcd_status_t status;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    status = HAL_LCD_WritePinValue(&lcd->rs_pin, GPIO_PIN_RESET);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    if (lcd->rw_pin_used != 0U)
    {
        status = HAL_LCD_WritePinValue(&lcd->rw_pin, GPIO_PIN_RESET);
        if (status != HAL_LCD_STATUS_OK)
        {
            return status;
        }
    }

    status = HAL_LCD_WriteByte(lcd, command);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    if ((command == HAL_LCD_CMD_CLEAR_DISPLAY) || (command == HAL_LCD_CMD_RETURN_HOME))
    {
        HAL_LCD_DelayMs(2U);
    }
    else
    {
        HAL_LCD_DelayUs(50U);
    }

    return HAL_LCD_STATUS_OK;
}

hal_lcd_status_t HAL_LCD_WriteData(hal_lcd_t *lcd, uint8 data)
{
    hal_lcd_status_t status;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    status = HAL_LCD_WritePinValue(&lcd->rs_pin, GPIO_PIN_SET);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    if (lcd->rw_pin_used != 0U)
    {
        status = HAL_LCD_WritePinValue(&lcd->rw_pin, GPIO_PIN_RESET);
        if (status != HAL_LCD_STATUS_OK)
        {
            return status;
        }
    }

    status = HAL_LCD_WriteByte(lcd, data);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    HAL_LCD_DelayUs(50U);

    return HAL_LCD_STATUS_OK;
}

hal_lcd_status_t HAL_LCD_Clear(hal_lcd_t *lcd)
{
    return HAL_LCD_WriteCommand(lcd, HAL_LCD_CMD_CLEAR_DISPLAY);
}

hal_lcd_status_t HAL_LCD_Home(hal_lcd_t *lcd)
{
    return HAL_LCD_WriteCommand(lcd, HAL_LCD_CMD_RETURN_HOME);
}

hal_lcd_status_t HAL_LCD_SetCursor(hal_lcd_t *lcd, uint8 row, uint8 column)
{
    uint8 address;

    if ((lcd == NULL) || (row >= lcd->rows) || (column >= lcd->columns))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    address = (uint8)(g_hal_lcd_row_offsets[row] + column);

    return HAL_LCD_WriteCommand(lcd, (uint8)(HAL_LCD_CMD_SET_DDRAM_ADDR | address));
}

hal_lcd_status_t HAL_LCD_WriteChar(hal_lcd_t *lcd, uint8 character)
{
    return HAL_LCD_WriteData(lcd, character);
}

hal_lcd_status_t HAL_LCD_WriteString(hal_lcd_t *lcd, const uint8 *string)
{
    uint16 index;
    hal_lcd_status_t status;

    if ((lcd == NULL) || (string == NULL))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    index = 0U;
    while (string[index] != (uint8)'\0')
    {
        status = HAL_LCD_WriteChar(lcd, string[index]);
        if (status != HAL_LCD_STATUS_OK)
        {
            return status;
        }

        index++;
    }

    return HAL_LCD_STATUS_OK;
}

hal_lcd_status_t HAL_LCD_WriteStringAt(hal_lcd_t *lcd,
                                       uint8 row,
                                       uint8 column,
                                       const uint8 *string)
{
    hal_lcd_status_t status;

    status = HAL_LCD_SetCursor(lcd, row, column);
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    return HAL_LCD_WriteString(lcd, string);
}

hal_lcd_status_t HAL_LCD_WriteUInt(hal_lcd_t *lcd, uint32 value)
{
    uint8 buffer[10];
    uint8 digits;
    hal_lcd_status_t status;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    if (value == 0U)
    {
        return HAL_LCD_WriteChar(lcd, (uint8)'0');
    }

    digits = 0U;
    while ((value > 0U) && (digits < 10U))
    {
        buffer[digits] = (uint8)((value % 10U) + (uint32)'0');
        value /= 10U;
        digits++;
    }

    while (digits > 0U)
    {
        digits--;

        status = HAL_LCD_WriteChar(lcd, buffer[digits]);
        if (status != HAL_LCD_STATUS_OK)
        {
            return status;
        }
    }

    return HAL_LCD_STATUS_OK;
}

hal_lcd_status_t HAL_LCD_WriteInt(hal_lcd_t *lcd, sint32 value)
{
    uint32 magnitude;
    hal_lcd_status_t status;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    if (value < 0)
    {
        status = HAL_LCD_WriteChar(lcd, (uint8)'-');
        if (status != HAL_LCD_STATUS_OK)
        {
            return status;
        }

        magnitude = (uint32)(-value);
    }
    else
    {
        magnitude = (uint32)value;
    }

    return HAL_LCD_WriteUInt(lcd, magnitude);
}

hal_lcd_status_t HAL_LCD_WriteHex(hal_lcd_t *lcd, uint32 value)
{
    uint8 nibble;
    uint8 index;
    uint8 started;
    uint8 digit;
    hal_lcd_status_t status;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    status = HAL_LCD_WriteChar(lcd, (uint8)'0');
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    status = HAL_LCD_WriteChar(lcd, (uint8)'x');
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    started = 0U;
    for (index = 0U; index < 8U; index++)
    {
        nibble = (uint8)((value >> ((7U - index) * 4U)) & 0x0FU);

        if ((nibble != 0U) || (started != 0U) || (index == 7U))
        {
            started = 1U;

            if (nibble < 10U)
            {
                digit = (uint8)('0' + nibble);
            }
            else
            {
                digit = (uint8)('A' + (nibble - 10U));
            }

            status = HAL_LCD_WriteChar(lcd, digit);
            if (status != HAL_LCD_STATUS_OK)
            {
                return status;
            }
        }
    }

    return HAL_LCD_STATUS_OK;
}

hal_lcd_status_t HAL_LCD_CreateCustomChar(hal_lcd_t *lcd,
                                          uint8 location,
                                          const uint8 pattern[8])
{
    uint8 index;
    hal_lcd_status_t status;

    if ((lcd == NULL) || (pattern == NULL) || (location >= HAL_LCD_MAX_CUSTOM_CHARS))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    status = HAL_LCD_WriteCommand(lcd, (uint8)(HAL_LCD_CMD_SET_CGRAM_ADDR | (location << 3U)));
    if (status != HAL_LCD_STATUS_OK)
    {
        return status;
    }

    for (index = 0U; index < 8U; index++)
    {
        status = HAL_LCD_WriteData(lcd, (uint8)(pattern[index] & 0x1FU));
        if (status != HAL_LCD_STATUS_OK)
        {
            return status;
        }
    }

    return HAL_LCD_STATUS_OK;
}

hal_lcd_status_t HAL_LCD_WriteCustomChar(hal_lcd_t *lcd, uint8 location)
{
    if ((lcd == NULL) || (location >= HAL_LCD_MAX_CUSTOM_CHARS))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    return HAL_LCD_WriteData(lcd, location);
}

hal_lcd_status_t HAL_LCD_DisplayOn(hal_lcd_t *lcd)
{
    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    lcd->display_control |= HAL_LCD_DISPLAY_ON_FLAG;

    return HAL_LCD_ApplyDisplayControl(lcd);
}

hal_lcd_status_t HAL_LCD_DisplayOff(hal_lcd_t *lcd)
{
    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    lcd->display_control &= (uint8)(~HAL_LCD_DISPLAY_ON_FLAG);

    return HAL_LCD_ApplyDisplayControl(lcd);
}

hal_lcd_status_t HAL_LCD_CursorOn(hal_lcd_t *lcd)
{
    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    lcd->display_control |= HAL_LCD_CURSOR_ON_FLAG;

    return HAL_LCD_ApplyDisplayControl(lcd);
}

hal_lcd_status_t HAL_LCD_CursorOff(hal_lcd_t *lcd)
{
    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    lcd->display_control &= (uint8)(~HAL_LCD_CURSOR_ON_FLAG);

    return HAL_LCD_ApplyDisplayControl(lcd);
}

hal_lcd_status_t HAL_LCD_BlinkOn(hal_lcd_t *lcd)
{
    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    lcd->display_control |= HAL_LCD_BLINK_ON_FLAG;

    return HAL_LCD_ApplyDisplayControl(lcd);
}

hal_lcd_status_t HAL_LCD_BlinkOff(hal_lcd_t *lcd)
{
    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    lcd->display_control &= (uint8)(~HAL_LCD_BLINK_ON_FLAG);

    return HAL_LCD_ApplyDisplayControl(lcd);
}

hal_lcd_status_t HAL_LCD_SetEntryMode(hal_lcd_t *lcd,
                                      hal_lcd_entry_direction_t direction,
                                      hal_lcd_autoscroll_t auto_scroll)
{
    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    lcd->entry_mode = 0U;

    if (direction == HAL_LCD_ENTRY_LEFT_TO_RIGHT)
    {
        lcd->entry_mode |= HAL_LCD_ENTRY_LEFT_FLAG;
    }

    if (auto_scroll == HAL_LCD_AUTOSCROLL_ENABLE)
    {
        lcd->entry_mode |= HAL_LCD_ENTRY_SHIFT_INC_FLAG;
    }

    return HAL_LCD_WriteCommand(lcd, (uint8)(HAL_LCD_CMD_ENTRY_MODE_SET | lcd->entry_mode));
}

hal_lcd_status_t HAL_LCD_AutoScrollOn(hal_lcd_t *lcd)
{
    hal_lcd_entry_direction_t direction;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    if ((lcd->entry_mode & HAL_LCD_ENTRY_LEFT_FLAG) != 0U)
    {
        direction = HAL_LCD_ENTRY_LEFT_TO_RIGHT;
    }
    else
    {
        direction = HAL_LCD_ENTRY_RIGHT_TO_LEFT;
    }

    return HAL_LCD_SetEntryMode(lcd, direction, HAL_LCD_AUTOSCROLL_ENABLE);
}

hal_lcd_status_t HAL_LCD_AutoScrollOff(hal_lcd_t *lcd)
{
    hal_lcd_entry_direction_t direction;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    if ((lcd->entry_mode & HAL_LCD_ENTRY_LEFT_FLAG) != 0U)
    {
        direction = HAL_LCD_ENTRY_LEFT_TO_RIGHT;
    }
    else
    {
        direction = HAL_LCD_ENTRY_RIGHT_TO_LEFT;
    }

    return HAL_LCD_SetEntryMode(lcd, direction, HAL_LCD_AUTOSCROLL_DISABLE);
}

hal_lcd_status_t HAL_LCD_Shift(hal_lcd_t *lcd,
                               hal_lcd_shift_target_t target,
                               hal_lcd_shift_direction_t direction)
{
    uint8 command;

    if (lcd == NULL)
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    command = HAL_LCD_CMD_CURSOR_SHIFT;

    if (target == HAL_LCD_SHIFT_TARGET_DISPLAY)
    {
        command |= HAL_LCD_SHIFT_DISPLAY_FLAG;
    }

    if (direction == HAL_LCD_SHIFT_RIGHT)
    {
        command |= HAL_LCD_SHIFT_RIGHT_FLAG;
    }

    return HAL_LCD_WriteCommand(lcd, command);
}

hal_lcd_status_t HAL_LCD_BacklightOn(hal_lcd_t *lcd)
{
    if ((lcd == NULL) || (lcd->backlight_pin_used == 0U))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    return HAL_LCD_WritePinValue(&lcd->backlight_pin, GPIO_PIN_SET);
}

hal_lcd_status_t HAL_LCD_BacklightOff(hal_lcd_t *lcd)
{
    if ((lcd == NULL) || (lcd->backlight_pin_used == 0U))
    {
        return HAL_LCD_STATUS_INVALID_PARAM;
    }

    return HAL_LCD_WritePinValue(&lcd->backlight_pin, GPIO_PIN_RESET);
}
