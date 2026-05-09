/*
 * main_spi_rx.c - Single-board LCD + keypad UI
 *
 * Wiring:
 *   LCD: same GPIOB pins as configured below.
 *   Keypad 4x4: GPIOA A0..A7 (rows A0-A3, cols A4-A7).
 *
 * Reads the keypad locally and drives the LCD UI state machine.
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "app.h"
#include "app_protocol.h"
#include "HAL_KEYPAD.h"
#include "HAL_LCD.h"
#include "MCAL_RCC.h"
#include "mcal_gpio.h"

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
enum
{
	APP_PASSWORD_LEN = 4U,
	APP_MENU_ITEMS = 3U,
	APP_INFO_PAGES = 4U,
	APP_KEY_DEBOUNCE_TICKS = 5U,
	APP_KEYPAD_ROWS = 4U,
	APP_KEYPAD_COLS = 4U,
	APP_DELAY_LOOPS_PER_MS = 4000UL,
	APP_TICK_MS = 1U,
	APP_TICKS_PER_SECOND = (1000U / APP_TICK_MS),
	APP_FEEDBACK_TICKS = (120U / APP_TICK_MS),
	APP_AUTOLOCK_TICKS = (20000U / APP_TICK_MS),
	APP_AUTOLOCK_SECONDS = (APP_AUTOLOCK_TICKS * APP_TICK_MS) / 1000U,
	APP_TOAST_TICKS = (800U / APP_TICK_MS),
	APP_SPLASH_MS = 900U,
	APP_SELF_TEST_MS = 600U,
	APP_TYPEWRITER_MS = 35U
};

typedef enum
{
	LOGIN_SCREEN = 0U,
	MAIN_MENU,
	SETTINGS_MENU,
	INFO_SCREEN,
	HELP_SCREEN,
	TOAST_SCREEN,
	ERROR_SCREEN
} app_ui_state_t;

typedef enum
{
	MENU_ITEM_SETTINGS = 0U,
	MENU_ITEM_INFO,
	MENU_ITEM_HELP
} app_menu_item_t;

typedef enum
{
	APP_ERROR_NONE = 0U,
	APP_ERROR_KEYPAD_INIT
} app_error_t;

typedef struct
{
	sint32 temperature;
} app_settings_t;

typedef struct
{
	app_ui_state_t state;
	app_ui_state_t toast_next_state;
	uint8 menu_index;
	sint32 temperature;
	uint8 password_input[APP_PASSWORD_LEN + 1U];
	uint8 password_len;
	uint8 login_error;
	uint8 render_requested;
	uint8 feedback_ticks;
	uint8 error_code;
	uint8 settings_dirty;
	uint8 self_test_ok;
	uint8 info_page;
	uint8 last_key;
	uint32 key_press_count;
	uint32 ticks;
	uint32 last_activity_ticks;
	uint32 uptime_seconds;
	uint32 toast_until_ticks;
	const uint8 *toast_line0;
	const uint8 *toast_line1;
} app_ui_context_t;

static const mcal_gpio_pin_t APP_STATUS_LED_PIN = MCAL_GPIO_PIN_13;
static const uint8 g_app_password[APP_PASSWORD_LEN + 1U] = "2222";
static const uint8 g_app_name[] = "AgriVision";
static const uint8 g_app_version[] = "RX v1.0";
static const uint8 g_app_build_date[] = __DATE__;

static app_settings_t g_saved_settings = { 25 };

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
	.rows = 2U,
	.columns = 16U,
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

static app_ui_context_t g_ui = {
	.state = LOGIN_SCREEN,
	.toast_next_state = LOGIN_SCREEN,
	.menu_index = MENU_ITEM_SETTINGS,
	.temperature = 25,
	.password_input = { 0U },
	.password_len = 0U,
	.login_error = 0U,
	.render_requested = 1U,
	.feedback_ticks = 0U,
	.error_code = APP_ERROR_NONE,
	.settings_dirty = 0U,
	.self_test_ok = 0U,
	.info_page = 0U,
	.last_key = 0U,
	.key_press_count = 0U,
	.ticks = 0U,
	.last_activity_ticks = 0U,
	.uptime_seconds = 0U,
	.toast_until_ticks = 0U,
	.toast_line0 = NULL,
	.toast_line1 = NULL
};

typedef struct
{
	uint8 candidate_key;
	uint8 stable_key;
	uint8 stable_ticks;
} app_key_filter_t;

static hal_keypad_t g_app_keypad;
static app_key_filter_t g_app_key_filter = { 0U, 0U, 0U };

static void APP_DelayCycles(volatile uint32 cycles);
static void APP_DelayMs(uint32 milliseconds);
static void APP_StatusLed_Init(void);
static void APP_StatusLed_Toggle(void);
static uint8 APP_InitPeripherals(void);
static uint8 APP_Keypad_GetPressedEvent(uint8 *key);
static uint8 APP_KeyToPacket(uint8 key, Packet *packet);
static void APP_LoadSettings(void);
static void APP_SaveSettings(void);

static void UI_ResetLogin(void);
static void UI_RequestRender(void);
static void UI_RegisterActivity(void);
static uint8 UI_IsDigit(uint8 value);
static uint8 UI_CheckPassword(void);
static void UI_WritePassword(uint8 count);
static void UI_WriteStringType(const uint8 *text, uint8 delay_ms);
static void UI_WriteTwoDigits(uint8 value);
static void UI_WriteThreeDigits(uint16 value);
static void UI_WriteUptime(uint32 seconds);
static void UI_WriteBuildDateShort(void);
static void UI_WriteAutoLock(uint8 row, uint8 col);
static void UI_WriteKeyOrDash(uint8 key);
static void UI_RenderInfoPageIndicator(uint8 page);
static void UI_RenderStatusIcons(void);
static void UI_RenderBackHint(uint8 row);
static void UI_ShowSplash(void);
static void UI_RunSelfTest(void);
static void UI_ShowToast(const uint8 *line0, const uint8 *line1, uint32 duration_ms,
					   app_ui_state_t next_state);
static void UI_SetError(uint8 code);
static void UI_RenderToast(void);
static void UI_RenderError(void);
static void UI_Tick(void);
static void UI_RenderLogin(void);
static void UI_RenderMainMenu(void);
static void UI_RenderSettings(void);
static void UI_RenderInfo(void);
static void UI_RenderHelp(void);
static void UI_Update(void);
static void UI_Render(void);
static void Handle_Input(Packet packet);

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
	MCAL_GPIO_Init(GPIOC, APP_STATUS_LED_PIN, MCAL_GPIO_MODE_OUTPUT_PP, MCAL_GPIO_SPEED_LOW);
	MCAL_GPIO_WritePin(GPIOC, APP_STATUS_LED_PIN, GPIO_PIN_SET);
}

static void APP_StatusLed_Toggle(void)
{
	MCAL_GPIO_TogglePin(GPIOC, APP_STATUS_LED_PIN);
}

static uint8 APP_InitPeripherals(void)
{
	MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_A);
	MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_B);
	MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_C);

	APP_StatusLed_Init();
	HAL_LCD_Init(&g_app_lcd);

	if (HAL_KEYPAD_Init(&g_app_keypad, &g_app_keypad_config) != HAL_KEYPAD_STATUS_OK)
	{
		UI_SetError(APP_ERROR_KEYPAD_INIT);
		return 0U;
	}

	return 1U;
}

static uint8 APP_Keypad_GetPressedEvent(uint8 *key)
{
	uint8 raw_key;

	if ((key == NULL) || (HAL_KEYPAD_Scan(&g_app_keypad, &raw_key) != HAL_KEYPAD_STATUS_OK))
	{
		return 0U;
	}

	if (raw_key == g_app_key_filter.candidate_key)
	{
		if (g_app_key_filter.stable_ticks < APP_KEY_DEBOUNCE_TICKS)
		{
			g_app_key_filter.stable_ticks++;
		}
	}
	else
	{
		g_app_key_filter.candidate_key = raw_key;
		g_app_key_filter.stable_ticks = 0U;
	}

	if ((g_app_key_filter.stable_ticks >= APP_KEY_DEBOUNCE_TICKS) &&
		(g_app_key_filter.stable_key != g_app_key_filter.candidate_key))
	{
		g_app_key_filter.stable_key = g_app_key_filter.candidate_key;
		if (g_app_key_filter.stable_key != 0U)
		{
			*key = g_app_key_filter.stable_key;
			return 1U;
		}
	}

	return 0U;
}

static uint8 APP_KeyToPacket(uint8 key, Packet *packet)
{
	if (packet == NULL)
	{
		return 0U;
	}

	packet->value = 0U;

	switch (key)
	{
		case 'A':
			packet->type = NAV_UP;
			break;

		case 'B':
			packet->type = NAV_DOWN;
			break;

		case 'C':
		case '#':
			packet->type = NAV_SELECT;
			break;

		case 'D':
		case '*':
			packet->type = NAV_BACK;
			break;

		default:
			packet->type = INPUT_KEY;
			packet->value = key;
			break;
	}

	return 1U;
}

static void APP_LoadSettings(void)
{
	g_ui.temperature = g_saved_settings.temperature;
	g_ui.settings_dirty = 0U;
}

static void APP_SaveSettings(void)
{
	g_saved_settings.temperature = g_ui.temperature;
	g_ui.settings_dirty = 0U;
}

static void UI_ResetLogin(void)
{
	uint8 index;

	for (index = 0U; index < sizeof(g_ui.password_input); index++)
	{
		g_ui.password_input[index] = 0U;
	}

	g_ui.password_len = 0U;
	g_ui.login_error = 0U;
}

static void UI_RequestRender(void)
{
	g_ui.render_requested = 1U;
}

static void UI_RegisterActivity(void)
{
	uint8 was_idle;

	was_idle = (g_ui.feedback_ticks == 0U);
	g_ui.last_activity_ticks = g_ui.ticks;
	g_ui.feedback_ticks = (uint8)APP_FEEDBACK_TICKS;

	if (was_idle != 0U)
	{
		UI_RequestRender();
	}
}

static uint8 UI_IsDigit(uint8 value)
{
	return (uint8)((value >= (uint8)'0') && (value <= (uint8)'9'));
}

static uint8 UI_CheckPassword(void)
{
	uint8 index;

	for (index = 0U; index < APP_PASSWORD_LEN; index++)
	{
		if (g_ui.password_input[index] != g_app_password[index])
		{
			return 0U;
		}
	}

	return 1U;
}

static void UI_WritePassword(uint8 count)
{
	uint8 index;

	for (index = 0U; index < count; index++)
	{
		HAL_LCD_WriteChar(&g_app_lcd, g_ui.password_input[index]);
	}
}

static void UI_WriteStringType(const uint8 *text, uint8 delay_ms)
{
	if (text == NULL)
	{
		return;
	}

	while (*text != (uint8)'\0')
	{
		HAL_LCD_WriteChar(&g_app_lcd, *text);
		APP_DelayMs(delay_ms);
		text++;
	}
}

static void UI_WriteTwoDigits(uint8 value)
{
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)('0' + (value / 10U)));
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)('0' + (value % 10U)));
}

static void UI_WriteThreeDigits(uint16 value)
{
	uint16 trimmed;

	trimmed = (uint16)(value % 1000U);
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)('0' + (trimmed / 100U)));
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)('0' + ((trimmed / 10U) % 10U)));
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)('0' + (trimmed % 10U)));
}

static void UI_WriteUptime(uint32 seconds)
{
	uint8 minutes;
	uint8 secs;

	minutes = (uint8)((seconds / 60U) % 100U);
	secs = (uint8)(seconds % 60U);
	UI_WriteTwoDigits(minutes);
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)':');
	UI_WriteTwoDigits(secs);
}

static void UI_WriteBuildDateShort(void)
{
	const uint8 *date;
	uint8 day_tens;

	date = g_app_build_date;
	HAL_LCD_WriteChar(&g_app_lcd, date[0]);
	HAL_LCD_WriteChar(&g_app_lcd, date[1]);
	HAL_LCD_WriteChar(&g_app_lcd, date[2]);
	day_tens = (date[4] == (uint8)' ') ? (uint8)'0' : date[4];
	HAL_LCD_WriteChar(&g_app_lcd, day_tens);
	HAL_LCD_WriteChar(&g_app_lcd, date[5]);
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)' ');
	HAL_LCD_WriteChar(&g_app_lcd, date[9]);
	HAL_LCD_WriteChar(&g_app_lcd, date[10]);
}

static void UI_WriteAutoLock(uint8 row, uint8 col)
{
	HAL_LCD_SetCursor(&g_app_lcd, row, col);
	HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"AutoLock ");
	HAL_LCD_WriteInt(&g_app_lcd, (sint32)APP_AUTOLOCK_SECONDS);
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)'s');
}

static void UI_WriteKeyOrDash(uint8 key)
{
	HAL_LCD_WriteChar(&g_app_lcd, (key == 0U) ? (uint8)'-' : key);
}

static void UI_RenderInfoPageIndicator(uint8 page)
{
	uint8 page_value;

	page_value = (uint8)((page + 1U) % 10U);
	HAL_LCD_SetCursor(&g_app_lcd, 0U, 10U);
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)('0' + page_value));
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)'/');
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)('0' + APP_INFO_PAGES));
}

static void UI_RenderStatusIcons(void)
{
	uint8 is_locked;

	is_locked = (uint8)((g_ui.state == LOGIN_SCREEN) ? 1U : 0U);

	HAL_LCD_SetCursor(&g_app_lcd, 0U, 13U);
	HAL_LCD_WriteChar(&g_app_lcd, is_locked ? (uint8)'L' : (uint8)'U');
	HAL_LCD_WriteChar(&g_app_lcd, (g_ui.feedback_ticks > 0U) ? (uint8)'K' : (uint8)' ');
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)' ');
}

static void UI_RenderBackHint(uint8 row)
{
	HAL_LCD_SetCursor(&g_app_lcd, row, 13U);
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)'B');
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)'K');
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)' ');
}

static void UI_ShowSplash(void)
{
	HAL_LCD_Clear(&g_app_lcd);
	HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
	UI_WriteStringType(g_app_name, (uint8)APP_TYPEWRITER_MS);
	HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
	UI_WriteStringType(g_app_version, (uint8)APP_TYPEWRITER_MS);
	APP_DelayMs(APP_SPLASH_MS);
}

static void UI_RunSelfTest(void)
{
	HAL_LCD_Clear(&g_app_lcd);
	HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
	HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Self Test");
	HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
	HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"LCD OK");
	APP_DelayMs(APP_SELF_TEST_MS / 2U);
	HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
	HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"KEYPAD OK");
	APP_DelayMs(APP_SELF_TEST_MS / 2U);
	g_ui.self_test_ok = 1U;
}

static void UI_ShowToast(const uint8 *line0, const uint8 *line1, uint32 duration_ms,
					   app_ui_state_t next_state)
{
	g_ui.toast_line0 = line0;
	g_ui.toast_line1 = line1;
	g_ui.toast_until_ticks = g_ui.ticks + duration_ms;
	g_ui.toast_next_state = next_state;
	g_ui.state = TOAST_SCREEN;
	UI_RequestRender();
}

static void UI_SetError(uint8 code)
{
	uint32 idle_seconds;

	g_ui.error_code = code;
	if (g_ui.info_page >= APP_INFO_PAGES)
	{
		g_ui.info_page = 0U;
	}
	g_ui.state = ERROR_SCREEN;
	UI_RequestRender();
}

static void UI_RenderLogin(void)
{
	HAL_LCD_Clear(&g_app_lcd);
			UI_RenderInfoPageIndicator(g_ui.info_page);

	if (g_ui.login_error != 0U)
	{
		HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
		HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Wrong Password");
		HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
		HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Try Again");
		return;
			HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"SelfTst ");
			HAL_LCD_WriteString(&g_app_lcd,
				(g_ui.self_test_ok != 0U) ? (const uint8 *)"OK" : (const uint8 *)"ER");
			UI_RenderInfoPageIndicator(g_ui.info_page);
	HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
	HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Password:");
			HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Key:");
	HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
			HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)" C:");
			UI_WriteThreeDigits((uint16)g_ui.key_press_count);
	UI_WritePassword(g_ui.password_len);
}

static void UI_RenderMainMenu(void)
			HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Temp ");
	uint8 top_index;
	uint8 bottom_index;
			UI_RenderInfoPageIndicator(g_ui.info_page);

			UI_WriteAutoLock(1U, 0U);

	if (g_ui.menu_index >= APP_MENU_ITEMS)
	{
			idle_seconds = (g_ui.ticks - g_ui.last_activity_ticks) / 1000U;
		g_ui.menu_index = MENU_ITEM_SETTINGS;
			HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Idle ");
			UI_WriteThreeDigits((uint16)idle_seconds);
			HAL_LCD_WriteChar(&g_app_lcd, (uint8)'s');
			UI_RenderInfoPageIndicator(g_ui.info_page);
	bottom_index = (uint8)(top_index + 1U);
			HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
			HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Uptime ");
			UI_WriteUptime(g_ui.uptime_seconds);
	HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
	HAL_LCD_WriteChar(&g_app_lcd, (g_ui.menu_index == top_index) ? (uint8)'>' : (uint8)' ');
	HAL_LCD_WriteString(&g_app_lcd,
		(top_index == MENU_ITEM_SETTINGS) ? (const uint8 *)" Settings" : (const uint8 *)" Info");
	UI_RenderStatusIcons();

	HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
	HAL_LCD_WriteChar(&g_app_lcd, (g_ui.menu_index == bottom_index) ? (uint8)'>' : (uint8)' ');
	HAL_LCD_WriteString(&g_app_lcd,
		(bottom_index == MENU_ITEM_INFO) ? (const uint8 *)" Info" : (const uint8 *)" Help");
	UI_RenderBackHint(1U);
}

static void UI_RenderSettings(void)
{
	HAL_LCD_Clear(&g_app_lcd);
	HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
	HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Settings");
	if (g_ui.settings_dirty != 0U)
	{
		HAL_LCD_WriteChar(&g_app_lcd, (uint8)'*');
	}
	UI_RenderStatusIcons();
	HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
	HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Temp: ");
	HAL_LCD_WriteInt(&g_app_lcd, g_ui.temperature);
	HAL_LCD_WriteChar(&g_app_lcd, (uint8)'C');
	UI_RenderBackHint(1U);
}

static void UI_RenderInfo(void)
{
	HAL_LCD_Clear(&g_app_lcd);

	switch (g_ui.info_page)
	{
		case 0U:
			HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
			HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Time ");
			UI_WriteUptime(g_ui.uptime_seconds);
			UI_RenderStatusIcons();
			HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
			HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Date ");
			UI_WriteBuildDateShort();
			break;

		case 1U:
			HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
			HAL_LCD_WriteString(&g_app_lcd,
				(g_ui.self_test_ok != 0U) ? (const uint8 *)"SelfTest OK" : (const uint8 *)"SelfTest Err");
			UI_RenderStatusIcons();
			HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
			HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Key: ");
			UI_WriteKeyOrDash(g_ui.last_key);
			break;

		default:
			HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
			HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Temp: ");
			HAL_LCD_WriteInt(&g_app_lcd, g_ui.temperature);
			HAL_LCD_WriteChar(&g_app_lcd, (uint8)'C');
			UI_RenderStatusIcons();
			UI_WriteAutoLock(1U, 0U);
			break;
	}
	UI_RenderBackHint(1U);
}

static void UI_RenderHelp(void)
{
	HAL_LCD_Clear(&g_app_lcd);
	HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
	HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"A/B:UpDn");
	UI_RenderStatusIcons();
	HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
	HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"C:OK D:BK");
	UI_RenderBackHint(1U);
}

static void UI_RenderToast(void)
{
	HAL_LCD_Clear(&g_app_lcd);
	if (g_ui.toast_line0 != NULL)
	{
		HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
		HAL_LCD_WriteString(&g_app_lcd, g_ui.toast_line0);
	}
	if (g_ui.toast_line1 != NULL)
	{
		HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
		HAL_LCD_WriteString(&g_app_lcd, g_ui.toast_line1);
	}
}

static void UI_RenderError(void)
{
	HAL_LCD_Clear(&g_app_lcd);
	HAL_LCD_SetCursor(&g_app_lcd, 0U, 0U);
	HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Error");
	HAL_LCD_SetCursor(&g_app_lcd, 1U, 0U);
	if (g_ui.error_code == APP_ERROR_KEYPAD_INIT)
	{
		HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Keypad Err");
	}
	else
	{
		HAL_LCD_WriteString(&g_app_lcd, (const uint8 *)"Unknown Err");
	}
}

static void UI_Tick(void)
{
	APP_DelayMs(APP_TICK_MS);
	g_ui.ticks += APP_TICK_MS;

	if (g_ui.feedback_ticks > 0U)
	{
		g_ui.feedback_ticks--;
		if (g_ui.feedback_ticks == 0U)
		{
			UI_RequestRender();
		}
	}

	if ((g_ui.ticks % APP_TICKS_PER_SECOND) == 0U)
	{
		g_ui.uptime_seconds++;
		if (g_ui.state == INFO_SCREEN)
		{
			UI_RequestRender();
		}
	}

	if (g_ui.state == TOAST_SCREEN)
	{
		if (g_ui.ticks >= g_ui.toast_until_ticks)
		{
			g_ui.state = g_ui.toast_next_state;
			UI_RequestRender();
		}
	}
	else if ((g_ui.state != LOGIN_SCREEN) && (g_ui.state != ERROR_SCREEN))
	{
		if ((g_ui.ticks - g_ui.last_activity_ticks) >= (uint32)APP_AUTOLOCK_TICKS)
		{
			UI_ResetLogin();
			UI_ShowToast((const uint8 *)"Auto Lock", (const uint8 *)"Returning", APP_TOAST_TICKS,
					 LOGIN_SCREEN);
		}
	}
}

static void UI_Update(void)
{
	UI_Tick();
	UI_Render();
}

static void UI_Render(void)
{
	if (g_ui.render_requested == 0U)
	{
		return;
	}

	switch (g_ui.state)
	{
		case LOGIN_SCREEN:
			UI_RenderLogin();
			break;

		case MAIN_MENU:
			UI_RenderMainMenu();
			break;

		case SETTINGS_MENU:
			UI_RenderSettings();
			break;

		case INFO_SCREEN:
			UI_RenderInfo();
			break;

		case HELP_SCREEN:
			UI_RenderHelp();
			break;

		case TOAST_SCREEN:
			UI_RenderToast();
			break;

		case ERROR_SCREEN:
			UI_RenderError();
			break;

		default:
			g_ui.state = LOGIN_SCREEN;
			UI_RenderLogin();
			break;
	}

	g_ui.render_requested = 0U;
}

static void Handle_Input(Packet packet)
{
	UI_RegisterActivity();

	switch (g_ui.state)
	{
		case LOGIN_SCREEN:
			if ((packet.type == INPUT_KEY) && (UI_IsDigit(packet.value) != 0U))
			{
				if (g_ui.login_error != 0U)
				{
					UI_ResetLogin();
				}

				if (g_ui.password_len < APP_PASSWORD_LEN)
				{
					g_ui.password_input[g_ui.password_len] = packet.value;
					g_ui.password_len++;
				}

				if (g_ui.password_len == APP_PASSWORD_LEN)
				{
					if (UI_CheckPassword() != 0U)
					{
						UI_ResetLogin();
						UI_ShowToast((const uint8 *)"Welcome", g_app_name, APP_TOAST_TICKS,
								 MAIN_MENU);
					}
					else
					{
						UI_ResetLogin();
						g_ui.login_error = 1U;
					}
				}

				UI_RequestRender();
			}
			else if (packet.type == NAV_BACK)
			{
				UI_ResetLogin();
				UI_RequestRender();
			}
			else
			{
			}
			break;

		case MAIN_MENU:
			if (packet.type == NAV_UP)
			{
				g_ui.menu_index = (g_ui.menu_index == 0U) ? (APP_MENU_ITEMS - 1U) : (g_ui.menu_index - 1U);
				UI_RequestRender();
			}
			else if (packet.type == NAV_DOWN)
			{
				g_ui.menu_index = (g_ui.menu_index >= (APP_MENU_ITEMS - 1U)) ? 0U : (g_ui.menu_index + 1U);
				UI_RequestRender();
			}
			else if (packet.type == NAV_SELECT)
			{
				if (g_ui.menu_index == MENU_ITEM_SETTINGS)
				{
					g_ui.state = SETTINGS_MENU;
				}
				else if (g_ui.menu_index == MENU_ITEM_INFO)
				{
					g_ui.state = INFO_SCREEN;
					g_ui.info_page = 0U;
				}
				else
				{
					g_ui.state = HELP_SCREEN;
				}
				UI_RequestRender();
			}
			else if (packet.type == NAV_BACK)
			{
				g_ui.state = LOGIN_SCREEN;
				UI_ResetLogin();
				UI_RequestRender();
			}
			else
			{
			}
			break;

		case SETTINGS_MENU:
			if (packet.type == NAV_UP)
			{
				if (g_ui.temperature < 125)
				{
					g_ui.temperature++;
				}
				g_ui.settings_dirty = 1U;
				UI_RequestRender();
			}
			else if (packet.type == NAV_DOWN)
			{
				if (g_ui.temperature > -40)
				{
					g_ui.temperature--;
				}
				g_ui.settings_dirty = 1U;
				UI_RequestRender();
			}
			else if (packet.type == NAV_BACK)
			{
				if (g_ui.settings_dirty != 0U)
				{
					APP_SaveSettings();
					UI_ShowToast((const uint8 *)"Saved", (const uint8 *)"Settings", APP_TOAST_TICKS,
							 MAIN_MENU);
				}
				else
				{
					g_ui.state = MAIN_MENU;
					UI_RequestRender();
				}
			}
			else
			{
			}
			break;

		case INFO_SCREEN:
			if ((packet.type == NAV_UP) || (packet.type == NAV_DOWN) || (packet.type == NAV_SELECT))
			{
				g_ui.info_page = (uint8)((g_ui.info_page + 1U) % APP_INFO_PAGES);
				UI_RequestRender();
			}
			else if (packet.type == NAV_BACK)
			{
				g_ui.state = MAIN_MENU;
				g_ui.info_page = 0U;
				UI_RequestRender();
			}
			break;

		case HELP_SCREEN:
			if (packet.type == NAV_BACK)
			{
				g_ui.state = MAIN_MENU;
				UI_RequestRender();
			}
			break;

		case TOAST_SCREEN:
			break;

		case ERROR_SCREEN:
			if ((packet.type == NAV_BACK) || (packet.type == NAV_SELECT))
			{
				g_ui.error_code = APP_ERROR_NONE;
				g_ui.state = LOGIN_SCREEN;
				UI_ResetLogin();
				UI_RequestRender();
			}
			break;

		default:
			g_ui.state = LOGIN_SCREEN;
			UI_ResetLogin();
			UI_RequestRender();
			break;
	}
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
void APP_Run(void)
{
	Packet packet;
	uint8 key;

	if (APP_InitPeripherals() == 0U)
	{
		while (1)
		{
			UI_Update();
		}
	}

	APP_LoadSettings();
	UI_ShowSplash();
	UI_RunSelfTest();

	UI_Render();

	while (1)
	{
		if ((APP_Keypad_GetPressedEvent(&key) != 0U) && (APP_KeyToPacket(key, &packet) != 0U))
		{
			g_ui.last_key = key;
			g_ui.key_press_count++;
			APP_StatusLed_Toggle();
			Handle_Input(packet);
		}

		UI_Update();
	}
}

int main(void)
{
	APP_Run();

	return 0;
}
