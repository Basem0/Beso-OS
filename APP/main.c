/*
 * main.c - BESO OS for STM32F401RC Blackpill
 *
 * LCD 16x2, 4-bit mode:
 *   RS=PB14, EN=PB12, D4=PB4, D5=PB3, D6=PB6, D7=PB5
 *
 * Keypad 4x4:
 *   Rows: PA0, PA1, PA2, PA3
 *   Cols: PA4, PA5, PA6, PA7
 *
 * Key map:
 *   A = up/left/minus, B = down/right/plus
 *   C/# = select/equals/save, D/* = back/delete where shown
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "app.h"
#include "HAL_KEYPAD.h"
#include "HAL_LCD.h"
#include "MCAL_RCC.h"
#include "mcal_gpio.h"

/* ================================================================ */
/* =========================== Macros ============================= */
/* ================================================================ */
enum
{
    APP_LCD_ROWS = 2U,
    APP_LCD_COLS = 16U,
    APP_KEYPAD_ROWS = 4U,
    APP_KEYPAD_COLS = 4U,
    APP_PASSWORD_LEN = 4U,
    APP_LOOP_DELAY_MS = 40U,
    APP_DELAY_LOOPS_PER_MS = 4000UL,
    APP_TOAST_MS = 900U,
    APP_HOME_ITEMS = 3U,
    APP_APPS_ITEMS = 3U,
    APP_SETTINGS_ITEMS = 4U,
    APP_DETAIL_ITEMS = 4U
};

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
typedef enum
{
    APP_STATE_LOGIN = 0U,
    APP_STATE_HOME,
    APP_STATE_APPS,
    APP_STATE_SETTINGS,
    APP_STATE_CALCULATOR,
    APP_STATE_CAR,
    APP_STATE_QUIZ,
    APP_STATE_EDIT_FPS,
    APP_STATE_CHANGE_PASSWORD,
    APP_STATE_CHANGE_DETAILS,
    APP_STATE_TOAST
} app_state_t;

typedef struct
{
    sint32 operand_a;
    sint32 operand_b;
    char operation;
    uint8 has_operation;
    uint8 entering_second;
    uint8 has_result;
    sint32 result;
} app_calc_t;

typedef struct
{
    uint8 car_lane;
    uint8 obstacle_lane;
    uint8 obstacle_row;
    uint16 score;
    uint32 last_frame_ms;
} app_car_t;

typedef struct
{
    uint8 a;
    uint8 b;
    sint32 answer;
    sint32 input;
    uint8 input_len;
    uint8 score;
} app_quiz_t;

static void APP_DelayMs(uint32 milliseconds);

static const hal_keypad_pin_t g_app_keypad_rows[APP_KEYPAD_ROWS] = {
    { GPIOA, MCAL_GPIO_PIN_0 },
    { GPIOA, MCAL_GPIO_PIN_1 },
    { GPIOA, MCAL_GPIO_PIN_2 },
    { GPIOA, MCAL_GPIO_PIN_3 }
};

static const hal_keypad_pin_t g_app_keypad_cols[APP_KEYPAD_COLS] = {
    { GPIOA, MCAL_GPIO_PIN_4 },
    { GPIOA, MCAL_GPIO_PIN_5 },
    { GPIOA, MCAL_GPIO_PIN_6 },
    { GPIOA, MCAL_GPIO_PIN_7 }
};

static const hal_keypad_config_t g_app_keypad_config = {
    .rows = g_app_keypad_rows,
    .cols = g_app_keypad_cols,
    .row_count = APP_KEYPAD_ROWS,
    .col_count = APP_KEYPAD_COLS,
    .debounce_ms = 20U,
    .delay_ms = APP_DelayMs
};

static hal_lcd_t g_app_lcd = {
    .interface_mode = HAL_LCD_INTERFACE_4BIT,
    .lines_mode = HAL_LCD_LINES_2,
    .font_mode = HAL_LCD_FONT_5X8,
    .gpio_speed = MCAL_GPIO_SPEED_LOW,
    .rows = APP_LCD_ROWS,
    .columns = APP_LCD_COLS,
    .rs_pin = { GPIOB, MCAL_GPIO_PIN_14 },
    .en_pin = { GPIOB, MCAL_GPIO_PIN_12 },
    .rw_pin_used = 0U,
    .rw_pin = { NULL, MCAL_GPIO_PIN_0 },
    .data_pins = {
        { NULL, MCAL_GPIO_PIN_0 },
        { NULL, MCAL_GPIO_PIN_0 },
        { NULL, MCAL_GPIO_PIN_0 },
        { NULL, MCAL_GPIO_PIN_0 },
        { GPIOB, MCAL_GPIO_PIN_4 },
        { GPIOB, MCAL_GPIO_PIN_3 },
        { GPIOB, MCAL_GPIO_PIN_6 },
        { GPIOB, MCAL_GPIO_PIN_5 }
    },
    .backlight_pin_used = 0U,
    .backlight_pin = { NULL, MCAL_GPIO_PIN_0 },
    .display_control = 0U,
    .entry_mode = 0U,
    .function_set = 0U
};

static hal_keypad_t g_app_keypad;

static app_state_t g_app_state = APP_STATE_LOGIN;
static app_state_t g_app_toast_next_state = APP_STATE_LOGIN;
static uint32 g_app_ticks_ms = 0U;
static uint32 g_app_toast_until_ms = 0U;
static uint32 g_app_rng = 0xBEE50A55UL;

static uint8 g_app_password[APP_PASSWORD_LEN + 1U] = "1234";
static uint8 g_app_pin_input[APP_PASSWORD_LEN + 1U] = { 0U };
static uint8 g_app_pin_len = 0U;
static uint8 g_app_edit_pin[APP_PASSWORD_LEN + 1U] = { 0U };
static uint8 g_app_edit_pin_len = 0U;

static uint8 g_app_fake_fps = 10U;
static uint8 g_app_menu_index = 0U;
static uint8 g_app_details_index = 0U;
static uint8 g_app_user_detail[APP_LCD_COLS + 1U] = "Blackpill F401";
static uint8 g_app_toast_line0[APP_LCD_COLS + 1U] = { 0U };
static uint8 g_app_toast_line1[APP_LCD_COLS + 1U] = { 0U };

static app_calc_t g_app_calc = { 0, 0, '+', 0U, 0U, 0U, 0 };
static app_car_t g_app_car = { 1U, 0U, 0U, 0U, 0U };
static app_quiz_t g_app_quiz = { 0U, 0U, 0, 0, 0U, 0U };

static const uint8 *g_app_home_items[APP_HOME_ITEMS] = {
    (const uint8 *)"Apps",
    (const uint8 *)"Settings",
    (const uint8 *)"Logout"
};

static const uint8 *g_app_apps_items[APP_APPS_ITEMS] = {
    (const uint8 *)"Calculator",
    (const uint8 *)"Car Racing",
    (const uint8 *)"Math Quiz"
};

static const uint8 *g_app_settings_items[APP_SETTINGS_ITEMS] = {
    (const uint8 *)"Edit Fake FPS",
    (const uint8 *)"Change Password",
    (const uint8 *)"Change Details",
    (const uint8 *)"Back"
};

static const uint8 *g_app_detail_options[APP_DETAIL_ITEMS] = {
    (const uint8 *)"Blackpill F401",
    (const uint8 *)"BESO User",
    (const uint8 *)"AgriVision",
    (const uint8 *)"STM32 MiniOS"
};

static void APP_DelayCycles(volatile uint32 cycles);
static uint8 APP_InitPeripherals(void);
static void APP_StatusLed_Init(void);
static void APP_StatusLed_Set(uint8 is_on);
static void APP_StatusLed_Toggle(void);
static uint8 APP_ReadKey(void);
static void APP_ProcessKey(uint8 key);
static void APP_Update(void);
static void APP_Render(void);

static void APP_LCD_WriteLine(uint8 row, const uint8 *text);
static void APP_LCD_WriteTwoLines(const uint8 *line0, const uint8 *line1);
static void APP_CopyText(uint8 *dst, uint8 dst_len, const uint8 *src);
static uint8 APP_TextEqual(const uint8 *a, const uint8 *b, uint8 len);
static void APP_ClearBuffer(uint8 *buffer, uint8 len);
static void APP_AppendChar(uint8 *buffer, uint8 len, uint8 *pos, uint8 ch);
static void APP_AppendText(uint8 *buffer, uint8 len, uint8 *pos, const uint8 *text);
static void APP_AppendUInt(uint8 *buffer, uint8 len, uint8 *pos, uint32 value);
static void APP_IntToStr(sint32 value, uint8 *out, uint8 out_len);
static uint32 APP_Rand(void);
static void APP_ShowToast(const uint8 *line0, const uint8 *line1, app_state_t next_state);

static void APP_RenderLogin(void);
static void APP_RenderMenu(const uint8 *title, const uint8 **items, uint8 count, uint8 index);
static void APP_RenderCalculator(void);
static void APP_RenderCar(void);
static void APP_RenderQuiz(void);
static void APP_RenderEditFps(void);
static void APP_RenderChangePassword(void);
static void APP_RenderChangeDetails(void);

static void APP_LoginKey(uint8 key);
static void APP_HomeKey(uint8 key);
static void APP_AppsKey(uint8 key);
static void APP_SettingsKey(uint8 key);
static void APP_CalculatorKey(uint8 key);
static void APP_CarKey(uint8 key);
static void APP_QuizKey(uint8 key);
static void APP_EditFpsKey(uint8 key);
static void APP_ChangePasswordKey(uint8 key);
static void APP_ChangeDetailsKey(uint8 key);

static void APP_CalcReset(void);
static void APP_CalcEvaluate(void);
static void APP_CarReset(void);
static void APP_CarFrame(void);
static void APP_QuizNewQuestion(void);

/* ================================================================ */
/* ================= Static Function Definitions ================== */
/* ================================================================ */
static void APP_DelayCycles(volatile uint32 cycles)
{
    while (cycles > 0U)
    {
        __asm volatile ("nop");
        cycles--;
    }
}

static void APP_DelayMs(uint32 milliseconds)
{
    while (milliseconds > 0U)
    {
        APP_DelayCycles(APP_DELAY_LOOPS_PER_MS);
        milliseconds--;
    }
}

static void APP_StatusLed_Init(void)
{
    MCAL_GPIO_Init(GPIOC, MCAL_GPIO_PIN_13, MCAL_GPIO_MODE_OUTPUT_PP, MCAL_GPIO_SPEED_LOW);
    MCAL_GPIO_WritePin(GPIOC, MCAL_GPIO_PIN_13, GPIO_PIN_SET);
}

static void APP_StatusLed_Set(uint8 is_on)
{
    MCAL_GPIO_WritePin(GPIOC, MCAL_GPIO_PIN_13, (is_on != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void APP_StatusLed_Toggle(void)
{
    MCAL_GPIO_TogglePin(GPIOC, MCAL_GPIO_PIN_13);
}

static uint8 APP_InitPeripherals(void)
{
    MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_A);
    MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_B);
    MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_C);

    APP_StatusLed_Init();

    if (HAL_KEYPAD_Init(&g_app_keypad, &g_app_keypad_config) != HAL_KEYPAD_STATUS_OK)
    {
        APP_StatusLed_Set(1U);
        return 0U;
    }

    if (HAL_LCD_Init(&g_app_lcd) != HAL_LCD_STATUS_OK)
    {
        APP_StatusLed_Set(1U);
        return 0U;
    }

    HAL_LCD_CursorOff(&g_app_lcd);
    HAL_LCD_Clear(&g_app_lcd);
    return 1U;
}

static uint8 APP_ReadKey(void)
{
    uint8 key = 0U;

    if (HAL_KEYPAD_GetKey(&g_app_keypad, &key) != HAL_KEYPAD_STATUS_OK)
    {
        return 0U;
    }

    if (key != 0U)
    {
        APP_StatusLed_Toggle();
    }

    return key;
}

static void APP_ProcessKey(uint8 key)
{
    if (key == 0U)
    {
        return;
    }

    switch (g_app_state)
    {
        case APP_STATE_LOGIN:
            APP_LoginKey(key);
            break;
        case APP_STATE_HOME:
            APP_HomeKey(key);
            break;
        case APP_STATE_APPS:
            APP_AppsKey(key);
            break;
        case APP_STATE_SETTINGS:
            APP_SettingsKey(key);
            break;
        case APP_STATE_CALCULATOR:
            APP_CalculatorKey(key);
            break;
        case APP_STATE_CAR:
            APP_CarKey(key);
            break;
        case APP_STATE_QUIZ:
            APP_QuizKey(key);
            break;
        case APP_STATE_EDIT_FPS:
            APP_EditFpsKey(key);
            break;
        case APP_STATE_CHANGE_PASSWORD:
            APP_ChangePasswordKey(key);
            break;
        case APP_STATE_CHANGE_DETAILS:
            APP_ChangeDetailsKey(key);
            break;
        default:
            break;
    }
}

static void APP_Update(void)
{
    if ((g_app_state == APP_STATE_TOAST) &&
        ((uint32)(g_app_ticks_ms - g_app_toast_until_ms) < 0x80000000UL))
    {
        g_app_state = g_app_toast_next_state;
        APP_Render();
    }

    if (g_app_state == APP_STATE_CAR)
    {
        APP_CarFrame();
    }
}

static void APP_Render(void)
{
    switch (g_app_state)
    {
        case APP_STATE_LOGIN:
            APP_RenderLogin();
            break;
        case APP_STATE_HOME:
            APP_RenderMenu((const uint8 *)"BESO OS Home", g_app_home_items, APP_HOME_ITEMS, g_app_menu_index);
            break;
        case APP_STATE_APPS:
            APP_RenderMenu((const uint8 *)"Apps", g_app_apps_items, APP_APPS_ITEMS, g_app_menu_index);
            break;
        case APP_STATE_SETTINGS:
            APP_RenderMenu((const uint8 *)"Settings", g_app_settings_items, APP_SETTINGS_ITEMS, g_app_menu_index);
            break;
        case APP_STATE_CALCULATOR:
            APP_RenderCalculator();
            break;
        case APP_STATE_CAR:
            APP_RenderCar();
            break;
        case APP_STATE_QUIZ:
            APP_RenderQuiz();
            break;
        case APP_STATE_EDIT_FPS:
            APP_RenderEditFps();
            break;
        case APP_STATE_CHANGE_PASSWORD:
            APP_RenderChangePassword();
            break;
        case APP_STATE_CHANGE_DETAILS:
            APP_RenderChangeDetails();
            break;
        case APP_STATE_TOAST:
            APP_LCD_WriteTwoLines(g_app_toast_line0, g_app_toast_line1);
            break;
        default:
            break;
    }
}

static void APP_LCD_WriteLine(uint8 row, const uint8 *text)
{
    uint8 col;

    HAL_LCD_SetCursor(&g_app_lcd, row, 0U);

    for (col = 0U; col < APP_LCD_COLS; col++)
    {
        if ((text != NULL) && (text[col] != 0U))
        {
            HAL_LCD_WriteChar(&g_app_lcd, text[col]);
        }
        else
        {
            HAL_LCD_WriteChar(&g_app_lcd, (uint8)' ');
        }
    }
}

static void APP_LCD_WriteTwoLines(const uint8 *line0, const uint8 *line1)
{
    APP_LCD_WriteLine(0U, line0);
    APP_LCD_WriteLine(1U, line1);
}

static void APP_CopyText(uint8 *dst, uint8 dst_len, const uint8 *src)
{
    uint8 i = 0U;

    if ((dst == NULL) || (dst_len == 0U))
    {
        return;
    }

    while ((src != NULL) && (src[i] != 0U) && (i < (uint8)(dst_len - 1U)))
    {
        dst[i] = src[i];
        i++;
    }

    dst[i] = 0U;
}

static uint8 APP_TextEqual(const uint8 *a, const uint8 *b, uint8 len)
{
    uint8 i;

    if ((a == NULL) || (b == NULL))
    {
        return 0U;
    }

    for (i = 0U; i < len; i++)
    {
        if (a[i] != b[i])
        {
            return 0U;
        }
    }

    return 1U;
}

static void APP_ClearBuffer(uint8 *buffer, uint8 len)
{
    uint8 i;

    if (buffer == NULL)
    {
        return;
    }

    for (i = 0U; i < len; i++)
    {
        buffer[i] = 0U;
    }
}

static void APP_AppendChar(uint8 *buffer, uint8 len, uint8 *pos, uint8 ch)
{
    if ((buffer == NULL) || (pos == NULL) || (*pos >= (uint8)(len - 1U)))
    {
        return;
    }

    buffer[*pos] = ch;
    (*pos)++;
    buffer[*pos] = 0U;
}

static void APP_AppendText(uint8 *buffer, uint8 len, uint8 *pos, const uint8 *text)
{
    uint8 i = 0U;

    while ((text != NULL) && (text[i] != 0U))
    {
        APP_AppendChar(buffer, len, pos, text[i]);
        i++;
    }
}

static void APP_AppendUInt(uint8 *buffer, uint8 len, uint8 *pos, uint32 value)
{
    uint8 number[12];

    APP_IntToStr((sint32)value, number, sizeof(number));
    APP_AppendText(buffer, len, pos, number);
}

static void APP_IntToStr(sint32 value, uint8 *out, uint8 out_len)
{
    uint8 temp[12];
    uint8 idx = 0U;
    uint8 out_idx = 0U;
    uint8 negative = 0U;

    if ((out == NULL) || (out_len == 0U))
    {
        return;
    }

    if (value < 0)
    {
        negative = 1U;
        value = -value;
    }

    do
    {
        temp[idx] = (uint8)('0' + (value % 10));
        value /= 10;
        idx++;
    } while ((value > 0) && (idx < sizeof(temp)));

    if ((negative != 0U) && (idx < sizeof(temp)))
    {
        temp[idx] = (uint8)'-';
        idx++;
    }

    while ((idx > 0U) && (out_idx < (uint8)(out_len - 1U)))
    {
        idx--;
        out[out_idx] = temp[idx];
        out_idx++;
    }

    out[out_idx] = 0U;
}

static uint32 APP_Rand(void)
{
    g_app_rng = (g_app_rng * 1664525UL) + 1013904223UL;
    return g_app_rng;
}

static void APP_ShowToast(const uint8 *line0, const uint8 *line1, app_state_t next_state)
{
    APP_CopyText(g_app_toast_line0, sizeof(g_app_toast_line0), line0);
    APP_CopyText(g_app_toast_line1, sizeof(g_app_toast_line1), line1);
    g_app_toast_next_state = next_state;
    g_app_toast_until_ms = g_app_ticks_ms + APP_TOAST_MS;
    g_app_state = APP_STATE_TOAST;
    APP_Render();
}

static void APP_RenderLogin(void)
{
    uint8 line[APP_LCD_COLS + 1U];
    uint8 pos;
    uint8 i;

    APP_ClearBuffer(line, sizeof(line));
    pos = 0U;
    APP_AppendText(line, sizeof(line), &pos, (const uint8 *)"PIN:");

    for (i = 0U; i < APP_PASSWORD_LEN; i++)
    {
        APP_AppendChar(line, sizeof(line), &pos, (i < g_app_pin_len) ? (uint8)'*' : (uint8)'_');
    }

    APP_AppendText(line, sizeof(line), &pos, (const uint8 *)" #OK");
    APP_LCD_WriteTwoLines((const uint8 *)"BESO OS LOGIN", line);
}

static void APP_RenderMenu(const uint8 *title, const uint8 **items, uint8 count, uint8 index)
{
    uint8 line[APP_LCD_COLS + 1U];
    uint8 pos = 0U;

    APP_ClearBuffer(line, sizeof(line));
    APP_AppendChar(line, sizeof(line), &pos, (uint8)'>');

    if ((items != NULL) && (index < count))
    {
        APP_AppendText(line, sizeof(line), &pos, items[index]);
    }

    APP_LCD_WriteTwoLines(title, line);
}

static void APP_RenderCalculator(void)
{
    uint8 line0[APP_LCD_COLS + 1U];
    uint8 line1[APP_LCD_COLS + 1U];
    uint8 pos = 0U;
    uint8 number[12];

    APP_ClearBuffer(line0, sizeof(line0));
    APP_ClearBuffer(line1, sizeof(line1));

    if (g_app_calc.has_result != 0U)
    {
        APP_AppendChar(line0, sizeof(line0), &pos, (uint8)'=');
        APP_IntToStr(g_app_calc.result, number, sizeof(number));
        APP_AppendText(line0, sizeof(line0), &pos, number);
    }
    else
    {
        APP_IntToStr(g_app_calc.operand_a, number, sizeof(number));
        APP_AppendText(line0, sizeof(line0), &pos, number);

        if (g_app_calc.has_operation != 0U)
        {
            APP_AppendChar(line0, sizeof(line0), &pos, (uint8)g_app_calc.operation);
            APP_IntToStr(g_app_calc.operand_b, number, sizeof(number));
            APP_AppendText(line0, sizeof(line0), &pos, number);
        }
    }

    APP_CopyText(line1, sizeof(line1), (const uint8 *)"A+B-C*D/ #=");
    APP_LCD_WriteTwoLines(line0, line1);
}

static void APP_RenderCar(void)
{
    uint8 line0[APP_LCD_COLS + 1U];
    uint8 line1[APP_LCD_COLS + 1U];
    uint8 lane_pos[3] = { 3U, 7U, 11U };
    uint16 score = g_app_car.score;

    APP_CopyText(line0, sizeof(line0), (const uint8 *)"000  |   |     ");
    APP_CopyText(line1, sizeof(line1), (const uint8 *)"     |   |  *BK");

    if (score > 999U)
    {
        score = 999U;
    }

    line0[0U] = (uint8)('0' + (score / 100U));
    line0[1U] = (uint8)('0' + ((score / 10U) % 10U));
    line0[2U] = (uint8)('0' + (score % 10U));

    if (g_app_car.obstacle_row == 0U)
    {
        line0[lane_pos[g_app_car.obstacle_lane]] = (uint8)'X';
    }
    else
    {
        line1[lane_pos[g_app_car.obstacle_lane]] = (uint8)'X';
    }

    line1[lane_pos[g_app_car.car_lane]] = (uint8)'A';
    APP_LCD_WriteTwoLines(line0, line1);
}

static void APP_RenderQuiz(void)
{
    uint8 line0[APP_LCD_COLS + 1U];
    uint8 line1[APP_LCD_COLS + 1U];
    uint8 pos = 0U;

    APP_ClearBuffer(line0, sizeof(line0));
    APP_ClearBuffer(line1, sizeof(line1));

    APP_AppendUInt(line0, sizeof(line0), &pos, g_app_quiz.a);
    APP_AppendChar(line0, sizeof(line0), &pos, (uint8)'+');
    APP_AppendUInt(line0, sizeof(line0), &pos, g_app_quiz.b);
    APP_AppendText(line0, sizeof(line0), &pos, (const uint8 *)"=? S:");
    APP_AppendUInt(line0, sizeof(line0), &pos, g_app_quiz.score);

    pos = 0U;
    APP_AppendText(line1, sizeof(line1), &pos, (const uint8 *)"Ans:");
    if (g_app_quiz.input_len == 0U)
    {
        APP_AppendChar(line1, sizeof(line1), &pos, (uint8)'_');
    }
    else
    {
        APP_AppendUInt(line1, sizeof(line1), &pos, (uint32)g_app_quiz.input);
    }
    APP_AppendText(line1, sizeof(line1), &pos, (const uint8 *)" #OK");

    APP_LCD_WriteTwoLines(line0, line1);
}

static void APP_RenderEditFps(void)
{
    uint8 line0[APP_LCD_COLS + 1U];
    uint8 pos = 0U;

    APP_ClearBuffer(line0, sizeof(line0));
    APP_AppendText(line0, sizeof(line0), &pos, (const uint8 *)"Fake FPS:");
    APP_AppendUInt(line0, sizeof(line0), &pos, g_app_fake_fps);
    APP_LCD_WriteTwoLines(line0, (const uint8 *)"A- B+ #Save");
}

static void APP_RenderChangePassword(void)
{
    uint8 line[APP_LCD_COLS + 1U];
    uint8 pos = 0U;
    uint8 i;

    APP_ClearBuffer(line, sizeof(line));
    APP_AppendText(line, sizeof(line), &pos, (const uint8 *)"PIN:");

    for (i = 0U; i < APP_PASSWORD_LEN; i++)
    {
        APP_AppendChar(line, sizeof(line), &pos, (i < g_app_edit_pin_len) ? (uint8)'*' : (uint8)'_');
    }

    APP_LCD_WriteTwoLines((const uint8 *)"New Password", line);
}

static void APP_RenderChangeDetails(void)
{
    APP_LCD_WriteTwoLines((const uint8 *)"Change Details", g_app_detail_options[g_app_details_index]);
}

static void APP_LoginKey(uint8 key)
{
    if ((key >= (uint8)'0') && (key <= (uint8)'9') && (g_app_pin_len < APP_PASSWORD_LEN))
    {
        g_app_pin_input[g_app_pin_len] = key;
        g_app_pin_len++;
        g_app_pin_input[g_app_pin_len] = 0U;
        APP_Render();
        return;
    }

    if ((key == (uint8)'*') && (g_app_pin_len > 0U))
    {
        g_app_pin_len--;
        g_app_pin_input[g_app_pin_len] = 0U;
        APP_Render();
        return;
    }

    if ((key == (uint8)'#') || (key == (uint8)'C'))
    {
        if ((g_app_pin_len == APP_PASSWORD_LEN) &&
            (APP_TextEqual(g_app_pin_input, g_app_password, APP_PASSWORD_LEN) != 0U))
        {
            g_app_pin_len = 0U;
            APP_ClearBuffer(g_app_pin_input, sizeof(g_app_pin_input));
            g_app_menu_index = 0U;
            APP_ShowToast((const uint8 *)"BESO OS", g_app_user_detail, APP_STATE_HOME);
        }
        else
        {
            g_app_pin_len = 0U;
            APP_ClearBuffer(g_app_pin_input, sizeof(g_app_pin_input));
            APP_ShowToast((const uint8 *)"Wrong Password", (const uint8 *)"Try Again", APP_STATE_LOGIN);
        }
    }
}

static void APP_HomeKey(uint8 key)
{
    if ((key == (uint8)'A') && (g_app_menu_index > 0U))
    {
        g_app_menu_index--;
        APP_Render();
    }
    else if ((key == (uint8)'B') && (g_app_menu_index < (APP_HOME_ITEMS - 1U)))
    {
        g_app_menu_index++;
        APP_Render();
    }
    else if ((key == (uint8)'#') || (key == (uint8)'C'))
    {
        if (g_app_menu_index == 0U)
        {
            g_app_state = APP_STATE_APPS;
            g_app_menu_index = 0U;
            APP_Render();
        }
        else if (g_app_menu_index == 1U)
        {
            g_app_state = APP_STATE_SETTINGS;
            g_app_menu_index = 0U;
            APP_Render();
        }
        else
        {
            g_app_state = APP_STATE_LOGIN;
            g_app_menu_index = 0U;
            APP_Render();
        }
    }
}

static void APP_AppsKey(uint8 key)
{
    if ((key == (uint8)'A') && (g_app_menu_index > 0U))
    {
        g_app_menu_index--;
        APP_Render();
    }
    else if ((key == (uint8)'B') && (g_app_menu_index < (APP_APPS_ITEMS - 1U)))
    {
        g_app_menu_index++;
        APP_Render();
    }
    else if ((key == (uint8)'*') || (key == (uint8)'D'))
    {
        g_app_state = APP_STATE_HOME;
        g_app_menu_index = 0U;
        APP_Render();
    }
    else if ((key == (uint8)'#') || (key == (uint8)'C'))
    {
        if (g_app_menu_index == 0U)
        {
            APP_CalcReset();
            g_app_state = APP_STATE_CALCULATOR;
        }
        else if (g_app_menu_index == 1U)
        {
            APP_CarReset();
            g_app_state = APP_STATE_CAR;
        }
        else
        {
            g_app_quiz.score = 0U;
            APP_QuizNewQuestion();
            g_app_state = APP_STATE_QUIZ;
        }
        APP_Render();
    }
}

static void APP_SettingsKey(uint8 key)
{
    if ((key == (uint8)'A') && (g_app_menu_index > 0U))
    {
        g_app_menu_index--;
        APP_Render();
    }
    else if ((key == (uint8)'B') && (g_app_menu_index < (APP_SETTINGS_ITEMS - 1U)))
    {
        g_app_menu_index++;
        APP_Render();
    }
    else if ((key == (uint8)'*') || (key == (uint8)'D'))
    {
        g_app_state = APP_STATE_HOME;
        g_app_menu_index = 0U;
        APP_Render();
    }
    else if ((key == (uint8)'#') || (key == (uint8)'C'))
    {
        if (g_app_menu_index == 0U)
        {
            g_app_state = APP_STATE_EDIT_FPS;
        }
        else if (g_app_menu_index == 1U)
        {
            g_app_edit_pin_len = 0U;
            APP_ClearBuffer(g_app_edit_pin, sizeof(g_app_edit_pin));
            g_app_state = APP_STATE_CHANGE_PASSWORD;
        }
        else if (g_app_menu_index == 2U)
        {
            g_app_details_index = 0U;
            g_app_state = APP_STATE_CHANGE_DETAILS;
        }
        else
        {
            g_app_state = APP_STATE_HOME;
            g_app_menu_index = 0U;
        }
        APP_Render();
    }
}

static void APP_CalculatorKey(uint8 key)
{
    sint32 *target;

    if ((key == (uint8)'*') || ((key == (uint8)'D') && (g_app_calc.has_operation == 0U)))
    {
        g_app_state = APP_STATE_APPS;
        g_app_menu_index = 0U;
        APP_Render();
        return;
    }

    if ((key >= (uint8)'0') && (key <= (uint8)'9'))
    {
        if (g_app_calc.has_result != 0U)
        {
            APP_CalcReset();
        }

        target = (g_app_calc.entering_second != 0U) ? &g_app_calc.operand_b : &g_app_calc.operand_a;
        if (*target < 1000000)
        {
            *target = (*target * 10) + (sint32)(key - (uint8)'0');
        }
        APP_Render();
        return;
    }

    if ((key == (uint8)'A') || (key == (uint8)'B') || (key == (uint8)'C') || (key == (uint8)'D'))
    {
        if (key == (uint8)'A') g_app_calc.operation = '+';
        if (key == (uint8)'B') g_app_calc.operation = '-';
        if (key == (uint8)'C') g_app_calc.operation = '*';
        if (key == (uint8)'D') g_app_calc.operation = '/';
        g_app_calc.has_operation = 1U;
        g_app_calc.entering_second = 1U;
        g_app_calc.has_result = 0U;
        APP_Render();
        return;
    }

    if (key == (uint8)'#')
    {
        APP_CalcEvaluate();
        APP_Render();
    }
}

static void APP_CarKey(uint8 key)
{
    if ((key == (uint8)'*') || (key == (uint8)'D'))
    {
        g_app_state = APP_STATE_APPS;
        g_app_menu_index = 1U;
        APP_Render();
    }
    else if ((key == (uint8)'A') && (g_app_car.car_lane > 0U))
    {
        g_app_car.car_lane--;
        APP_Render();
    }
    else if ((key == (uint8)'B') && (g_app_car.car_lane < 2U))
    {
        g_app_car.car_lane++;
        APP_Render();
    }
}

static void APP_QuizKey(uint8 key)
{
    if ((key == (uint8)'*') || (key == (uint8)'D'))
    {
        g_app_state = APP_STATE_APPS;
        g_app_menu_index = 2U;
        APP_Render();
        return;
    }

    if ((key >= (uint8)'0') && (key <= (uint8)'9') && (g_app_quiz.input_len < 2U))
    {
        g_app_quiz.input = (g_app_quiz.input * 10) + (sint32)(key - (uint8)'0');
        g_app_quiz.input_len++;
        APP_Render();
        return;
    }

    if (key == (uint8)'#')
    {
        if ((g_app_quiz.input_len > 0U) && (g_app_quiz.input == g_app_quiz.answer))
        {
            if (g_app_quiz.score < 99U)
            {
                g_app_quiz.score++;
            }
            APP_QuizNewQuestion();
            APP_ShowToast((const uint8 *)"Correct", (const uint8 *)"Next Question", APP_STATE_QUIZ);
        }
        else
        {
            g_app_quiz.input = 0;
            g_app_quiz.input_len = 0U;
            APP_ShowToast((const uint8 *)"Wrong Answer", (const uint8 *)"Try Again", APP_STATE_QUIZ);
        }
    }
}

static void APP_EditFpsKey(uint8 key)
{
    if (((key == (uint8)'A') || (key == (uint8)'4')) && (g_app_fake_fps > 1U))
    {
        g_app_fake_fps--;
        APP_Render();
    }
    else if (((key == (uint8)'B') || (key == (uint8)'6')) && (g_app_fake_fps < 30U))
    {
        g_app_fake_fps++;
        APP_Render();
    }
    else if ((key == (uint8)'#') || (key == (uint8)'C'))
    {
        APP_ShowToast((const uint8 *)"Fake FPS Saved", (const uint8 *)"Used by Car", APP_STATE_SETTINGS);
    }
    else if ((key == (uint8)'*') || (key == (uint8)'D'))
    {
        g_app_state = APP_STATE_SETTINGS;
        APP_Render();
    }
}

static void APP_ChangePasswordKey(uint8 key)
{
    if ((key >= (uint8)'0') && (key <= (uint8)'9') && (g_app_edit_pin_len < APP_PASSWORD_LEN))
    {
        g_app_edit_pin[g_app_edit_pin_len] = key;
        g_app_edit_pin_len++;
        g_app_edit_pin[g_app_edit_pin_len] = 0U;
        APP_Render();
    }
    else if ((key == (uint8)'*') && (g_app_edit_pin_len > 0U))
    {
        g_app_edit_pin_len--;
        g_app_edit_pin[g_app_edit_pin_len] = 0U;
        APP_Render();
    }
    else if ((key == (uint8)'#') || (key == (uint8)'C'))
    {
        if (g_app_edit_pin_len == APP_PASSWORD_LEN)
        {
            APP_CopyText(g_app_password, sizeof(g_app_password), g_app_edit_pin);
            APP_ShowToast((const uint8 *)"Password Saved", (const uint8 *)"Use New PIN", APP_STATE_SETTINGS);
        }
        else
        {
            APP_ShowToast((const uint8 *)"Need 4 Digits", (const uint8 *)"Not Saved", APP_STATE_CHANGE_PASSWORD);
        }
    }
    else if (key == (uint8)'D')
    {
        g_app_state = APP_STATE_SETTINGS;
        APP_Render();
    }
}

static void APP_ChangeDetailsKey(uint8 key)
{
    if ((key == (uint8)'A') && (g_app_details_index > 0U))
    {
        g_app_details_index--;
        APP_Render();
    }
    else if ((key == (uint8)'B') && (g_app_details_index < (APP_DETAIL_ITEMS - 1U)))
    {
        g_app_details_index++;
        APP_Render();
    }
    else if ((key == (uint8)'#') || (key == (uint8)'C'))
    {
        APP_CopyText(g_app_user_detail, sizeof(g_app_user_detail), g_app_detail_options[g_app_details_index]);
        APP_ShowToast((const uint8 *)"Details Saved", g_app_user_detail, APP_STATE_SETTINGS);
    }
    else if ((key == (uint8)'*') || (key == (uint8)'D'))
    {
        g_app_state = APP_STATE_SETTINGS;
        APP_Render();
    }
}

static void APP_CalcReset(void)
{
    g_app_calc.operand_a = 0;
    g_app_calc.operand_b = 0;
    g_app_calc.operation = '+';
    g_app_calc.has_operation = 0U;
    g_app_calc.entering_second = 0U;
    g_app_calc.has_result = 0U;
    g_app_calc.result = 0;
}

static void APP_CalcEvaluate(void)
{
    sint32 a = g_app_calc.operand_a;
    sint32 b = g_app_calc.operand_b;

    if (g_app_calc.has_operation == 0U)
    {
        g_app_calc.result = a;
        g_app_calc.has_result = 1U;
        return;
    }

    switch (g_app_calc.operation)
    {
        case '+':
            g_app_calc.result = a + b;
            break;
        case '-':
            g_app_calc.result = a - b;
            break;
        case '*':
            g_app_calc.result = a * b;
            break;
        case '/':
            g_app_calc.result = (b == 0) ? 0 : (a / b);
            break;
        default:
            g_app_calc.result = 0;
            break;
    }

    g_app_calc.has_result = 1U;
}

static void APP_CarReset(void)
{
    g_app_car.car_lane = 1U;
    g_app_car.obstacle_lane = (uint8)(APP_Rand() % 3U);
    g_app_car.obstacle_row = 0U;
    g_app_car.score = 0U;
    g_app_car.last_frame_ms = g_app_ticks_ms;
}

static void APP_CarFrame(void)
{
    uint32 frame_ms = 1000UL / (uint32)g_app_fake_fps;

    if (frame_ms < 40UL)
    {
        frame_ms = 40UL;
    }

    if ((uint32)(g_app_ticks_ms - g_app_car.last_frame_ms) < frame_ms)
    {
        return;
    }

    g_app_car.last_frame_ms = g_app_ticks_ms;

    if (g_app_car.obstacle_row == 0U)
    {
        g_app_car.obstacle_row = 1U;
    }
    else
    {
        if (g_app_car.obstacle_lane == g_app_car.car_lane)
        {
            APP_CarReset();
            APP_ShowToast((const uint8 *)"Car Crashed", (const uint8 *)"# Restarted", APP_STATE_CAR);
            return;
        }

        if (g_app_car.score < 999U)
        {
            g_app_car.score++;
        }

        g_app_car.obstacle_lane = (uint8)(APP_Rand() % 3U);
        g_app_car.obstacle_row = 0U;
    }

    APP_Render();
}

static void APP_QuizNewQuestion(void)
{
    g_app_quiz.a = (uint8)(APP_Rand() % 10U);
    g_app_quiz.b = (uint8)(APP_Rand() % 10U);
    g_app_quiz.answer = (sint32)g_app_quiz.a + (sint32)g_app_quiz.b;
    g_app_quiz.input = 0;
    g_app_quiz.input_len = 0U;
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
void APP_Run(void)
{
    uint8 key;

    if (APP_InitPeripherals() == 0U)
    {
        while (1) { }
    }

    APP_LCD_WriteTwoLines((const uint8 *)"BESO OS", (const uint8 *)"Booting...");
    APP_DelayMs(700U);
    APP_Render();

    while (1)
    {
        key = APP_ReadKey();
        APP_ProcessKey(key);
        APP_Update();
        APP_DelayMs(APP_LOOP_DELAY_MS);
        g_app_ticks_ms += APP_LOOP_DELAY_MS;
    }
}

int main(void)
{
    APP_Run();
    return 0;
}
