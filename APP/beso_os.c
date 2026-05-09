/*
 * beso_os.c
 *
 * BESO OS scheduler, keypad input, shared UI, and screen routing.
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "beso_os.h"
#include "beso_os_private.h"
#include "HAL_KEYPAD.h"
#include "MCAL_RCC.h"
#include "MCAL_SPI.h"
#include "mcal_gpio.h"

/* ================================================================ */
/* =========================== Macros ============================= */
/* ================================================================ */
#define APP_CPU_HZ                (16000000UL)
#define APP_TICK_HZ               (1000U)
#define APP_KEY_RAW_PRESS_TICKS   (2U)
#define APP_KEY_RAW_RELEASE_TICKS (8U)
#define APP_KEYPAD_ROWS           (4U)
#define APP_KEYPAD_COLS           (4U)

#define APP_STATUS_LED_PORT       GPIOC
#define APP_STATUS_LED_PIN        MCAL_GPIO_PIN_13
#define APP_KEYPAD_LED_HOLD_MS    (200U)

#define APP_SYSTICK_CTRL_ENABLE   (1UL << 0U)
#define APP_SYSTICK_CTRL_TICKINT  (1UL << 1U)
#define APP_SYSTICK_CTRL_CLKSRC   (1UL << 2U)

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
typedef struct
{
    volatile uint32 CTRL;
    volatile uint32 LOAD;
    volatile uint32 VAL;
    volatile uint32 CALIB;
} app_systick_t;

#define APP_SYSTICK_BASE (0xE000E010UL)
#define APP_SYSTICK      ((app_systick_t *)APP_SYSTICK_BASE)

typedef struct
{
    uint32 interval_ms;
    uint32 last_run_ms;
    void (*task)(void);
} app_task_t;

static volatile uint32 g_app_tick_ms = 0U;
static uint32 g_app_rng = 0x1234ABCDUL;

static hal_keypad_t g_app_keypad;
static uint8 g_app_key_event = 0U;
static uint8 g_app_keypad_matrix[APP_KEYPAD_ROWS] = { 0U, 0U, 0U, 0U };
static uint8 g_app_key_raw_latch = 0U;
static uint8 g_app_key_raw_candidate = 0U;
static uint8 g_app_key_raw_ticks = 0U;
static uint8 g_app_key_raw_release_ticks = 0U;
static uint32 g_app_led_until = 0U;

hal_st7735s_t g_app_tft;

static app_screen_t g_app_screen = APP_SCREEN_LOGIN;
static uint8 g_app_menu_index = 0U;

static void APP_DelayMs(uint32 milliseconds);
static void APP_EnableIRQ(void);
static void APP_SysTick_Init(uint32 cpu_hz, uint32 tick_hz);
static void APP_StatusLed_Init(void);
static void APP_StatusLed_Set(uint8 is_on);
static void APP_GPIO_SetAlternateFunction(GPIO_RegDef_t *gpio_port,
                                          mcal_gpio_pin_t pin,
                                          uint8 alternate_function);
static void APP_SPI1_GPIOInit(void);
static void APP_TFT_GPIOInit(void);
static uint8 APP_InitPeripherals(void);

static void APP_TaskInput(void);
static void APP_TaskAppUpdate(void);
static void APP_Scheduler_Run(void);

static uint8 APP_ConsumeKey(void);

static void APP_UI_DrawMenuIcon(uint8 index, uint16 x, uint16 y, uint16 color, uint16 bg);
static void APP_UI_DrawTile(uint8 index, uint8 selected);

static void APP_DrawMenu(uint8 force);
static void APP_Menu_Update(uint32 now_ms, uint8 key);


static const hal_keypad_pin_t g_app_keypad_rows[APP_KEYPAD_ROWS] = {
    { GPIOB, MCAL_GPIO_PIN_12 },
    { GPIOB, MCAL_GPIO_PIN_13 },
    { GPIOB, MCAL_GPIO_PIN_14 },
    { GPIOB, MCAL_GPIO_PIN_15 }
};

static const hal_keypad_pin_t g_app_keypad_cols[APP_KEYPAD_COLS] = {
    { GPIOB, MCAL_GPIO_PIN_9 },
    { GPIOB, MCAL_GPIO_PIN_8 },
    { GPIOB, MCAL_GPIO_PIN_7 },
    { GPIOB, MCAL_GPIO_PIN_6 }
};

static const hal_keypad_config_t g_app_keypad_config = {
    .rows = g_app_keypad_rows,
    .cols = g_app_keypad_cols,
    .row_count = APP_KEYPAD_ROWS,
    .col_count = APP_KEYPAD_COLS,
    .debounce_ms = 20U,
    .delay_ms = APP_DelayMs
};

static const mcal_spi_config_t g_app_spi1_config = {
    .device_mode = MCAL_SPI_DEVICE_MASTER,
    .bus_config = MCAL_SPI_BUS_FULL_DUPLEX,
    .data_frame = MCAL_SPI_DFF_8BIT,
    .clock_polarity = MCAL_SPI_CPOL_LOW,
    .clock_phase = MCAL_SPI_CPHA_FIRST_EDGE,
    .baud_prescaler = MCAL_SPI_BAUD_DIV16,
    .first_bit = MCAL_SPI_FIRSTBIT_MSB,
    .software_slave_management = MCAL_SPI_SSM_ENABLE,
    .nss_mode = MCAL_SPI_NSS_SOFTWARE_INTERNAL,
    .crc_calculation = MCAL_SPI_CRC_DISABLE,
    .ti_mode = MCAL_SPI_TI_MODE_DISABLE,
    .dma_rx = MCAL_SPI_DMA_RX_DISABLE,
    .dma_tx = MCAL_SPI_DMA_TX_DISABLE
};

/* TFT wiring :
 * SCK=PA5, MOSI=PA7, CS=PA4, DC=PB1, RST=PB0, BL=PB2
 */
static const hal_st7735s_config_t g_app_tft_config = {
    .spi_port = SPI1,
    .cs_pin = { GPIOA, MCAL_GPIO_PIN_4 },
    .dc_pin = { GPIOB, MCAL_GPIO_PIN_1 },
    .rst_pin = { GPIOB, MCAL_GPIO_PIN_0 },
    .bl_pin = { GPIOB, MCAL_GPIO_PIN_2 },
    .width = APP_TFT_WIDTH,
    .height = APP_TFT_HEIGHT,
    .col_offset = 2U,
    .row_offset = 1U,
    .rotation = 1U,
    .invert = 1U,
    .delay_ms = APP_DelayMs
};

static app_task_t g_app_tasks[] = {
    { 10U, 0U, APP_TaskInput },
    { 20U, 0U, APP_TaskAppUpdate }
};

static const char *g_menu_short_items[] = {
    "Calc",
    "Snake",
    "Car",
    "Bird"
};

/* ================================================================ */
/* ================= Static Function Definitions ================== */
/* ================================================================ */
void SysTick_Handler(void)
{
    g_app_tick_ms++;
}

static void APP_EnableIRQ(void)
{
    __asm volatile ("cpsie i");
}

static void APP_SysTick_Init(uint32 cpu_hz, uint32 tick_hz)
{
    uint32 reload = (cpu_hz / tick_hz) - 1U;

    APP_SYSTICK->CTRL = 0U;
    APP_SYSTICK->LOAD = reload;
    APP_SYSTICK->VAL = 0U;
    APP_SYSTICK->CTRL = (APP_SYSTICK_CTRL_CLKSRC | APP_SYSTICK_CTRL_TICKINT | APP_SYSTICK_CTRL_ENABLE);
}

static void APP_StatusLed_Init(void)
{
    MCAL_GPIO_Init(APP_STATUS_LED_PORT, APP_STATUS_LED_PIN,
                   MCAL_GPIO_MODE_OUTPUT_PP, MCAL_GPIO_SPEED_LOW);
    MCAL_GPIO_WritePin(APP_STATUS_LED_PORT, APP_STATUS_LED_PIN, GPIO_PIN_SET);
}

static void APP_StatusLed_Set(uint8 is_on)
{
    MCAL_GPIO_WritePin(APP_STATUS_LED_PORT, APP_STATUS_LED_PIN,
                       (is_on != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void APP_DelayMs(uint32 milliseconds)
{
    uint32 start = g_app_tick_ms;
    while ((uint32)(g_app_tick_ms - start) < milliseconds) { }
}

static void APP_GPIO_SetAlternateFunction(GPIO_RegDef_t *gpio_port,
                                          mcal_gpio_pin_t pin,
                                          uint8 alternate_function)
{
    uint32 shift;

    if ((gpio_port == NULL) || (pin > MCAL_GPIO_PIN_15) || (alternate_function > 15U))
    {
        return;
    }

    if (pin < MCAL_GPIO_PIN_8)
    {
        shift = (uint32)pin * 4U;
        gpio_port->AFRL &= ~(0xFUL << shift);
        gpio_port->AFRL |= ((uint32)alternate_function << shift);
    }
    else
    {
        shift = ((uint32)pin - 8U) * 4U;
        gpio_port->AFRH &= ~(0xFUL << shift);
        gpio_port->AFRH |= ((uint32)alternate_function << shift);
    }
}

static void APP_SPI1_GPIOInit(void)
{
    MCAL_GPIO_Init(GPIOA, MCAL_GPIO_PIN_4, MCAL_GPIO_MODE_OUTPUT_PP, MCAL_GPIO_SPEED_VERY_HIGH);
    MCAL_GPIO_WritePin(GPIOA, MCAL_GPIO_PIN_4, GPIO_PIN_SET);

    MCAL_GPIO_Init(GPIOA, MCAL_GPIO_PIN_5, MCAL_GPIO_MODE_AF_PP, MCAL_GPIO_SPEED_VERY_HIGH);
    MCAL_GPIO_Init(GPIOA, MCAL_GPIO_PIN_7, MCAL_GPIO_MODE_AF_PP, MCAL_GPIO_SPEED_VERY_HIGH);

    APP_GPIO_SetAlternateFunction(GPIOA, MCAL_GPIO_PIN_5, 5U);
    APP_GPIO_SetAlternateFunction(GPIOA, MCAL_GPIO_PIN_7, 5U);
}

static void APP_TFT_GPIOInit(void)
{
    MCAL_GPIO_Init(GPIOB, MCAL_GPIO_PIN_0, MCAL_GPIO_MODE_OUTPUT_PP, MCAL_GPIO_SPEED_HIGH);
    MCAL_GPIO_Init(GPIOB, MCAL_GPIO_PIN_1, MCAL_GPIO_MODE_OUTPUT_PP, MCAL_GPIO_SPEED_HIGH);
    MCAL_GPIO_Init(GPIOB, MCAL_GPIO_PIN_2, MCAL_GPIO_MODE_OUTPUT_PP, MCAL_GPIO_SPEED_HIGH);
    MCAL_GPIO_WritePin(GPIOB, MCAL_GPIO_PIN_2, GPIO_PIN_SET);
}

static uint8 APP_InitPeripherals(void)
{
    mcal_spi_status_t spi_status;

    MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_A);
    MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_B);
    MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_C);
    MCAL_RCC_EnableAPB2PeripheralClock(MCAL_RCC_APB2_SPI1);

    APP_SysTick_Init(APP_CPU_HZ, APP_TICK_HZ);
    APP_EnableIRQ();

    APP_StatusLed_Init();

    APP_SPI1_GPIOInit();
    APP_TFT_GPIOInit();

    if (HAL_KEYPAD_Init(&g_app_keypad, &g_app_keypad_config) != HAL_KEYPAD_STATUS_OK)
    {
        return 0U;
    }

    spi_status = MCAL_SPI_Init(SPI1, &g_app_spi1_config);
    if (spi_status != MCAL_SPI_STATUS_OK)
    {
        return 0U;
    }

    MCAL_SPI_Enable(SPI1);

    if (HAL_ST7735S_Init(&g_app_tft, &g_app_tft_config) != HAL_ST7735S_STATUS_OK)
    {
        return 0U;
    }

    HAL_ST7735S_FillScreen(&g_app_tft, APP_COLOR_BG);
    return 1U;
}

static uint8 APP_ConsumeKey(void)
{
    uint8 key = g_app_key_event;
    g_app_key_event = 0U;
    return key;
}

static void APP_TaskInput(void)
{
    uint8 key;
    uint8 row;
    uint8 any_pressed = 0U;

    key = 0U;
    if (HAL_KEYPAD_ReadMatrix(&g_app_keypad, g_app_keypad_matrix, 1U) != HAL_KEYPAD_STATUS_OK)
    {
        return;
    }
    for (row = 0U; row < APP_KEYPAD_ROWS; row++)
    {
        any_pressed |= g_app_keypad_matrix[row];
        if (key == 0U)
        {
            if (g_app_keypad_matrix[row] != 0U)
            {
                (void)HAL_KEYPAD_KeyFromMatrix(&g_app_keypad, g_app_keypad_matrix, &key);
            }
        }
    }

    if (any_pressed != 0U)
    {
        APP_StatusLed_Set(1U);
        g_app_led_until = g_app_tick_ms + APP_KEYPAD_LED_HOLD_MS;
    }
    else
    {
        if (g_app_key_raw_release_ticks < APP_KEY_RAW_RELEASE_TICKS)
        {
            g_app_key_raw_release_ticks++;
        }
        else
        {
            g_app_key_raw_latch = 0U;
            g_app_key_raw_candidate = 0U;
            g_app_key_raw_ticks = 0U;
        }
    }

    if (g_app_key_event != 0U)
    {
        return;
    }

    if (any_pressed != 0U)
    {
        g_app_key_raw_release_ticks = 0U;

        if ((g_app_key_raw_latch == 0U) && (key != 0U))
        {
            if (key == g_app_key_raw_candidate)
            {
                if (g_app_key_raw_ticks < APP_KEY_RAW_PRESS_TICKS)
                {
                    g_app_key_raw_ticks++;
                }
            }
            else
            {
                g_app_key_raw_candidate = key;
                g_app_key_raw_ticks = 1U;
            }

            if (g_app_key_raw_ticks >= APP_KEY_RAW_PRESS_TICKS)
            {
                g_app_key_event = g_app_key_raw_candidate;
                g_app_key_raw_latch = 1U;
                APP_StatusLed_Set(1U);
                g_app_led_until = g_app_tick_ms + APP_KEYPAD_LED_HOLD_MS;
                return;
            }
        }
        return;
    }
}

static void APP_TaskAppUpdate(void)
{
    uint32 now = g_app_tick_ms;
    uint8 key = APP_ConsumeKey();

    switch (g_app_screen)
    {
        case APP_SCREEN_LOGIN:
            BESO_Login_Update(now, key);
            break;
        case APP_SCREEN_MENU:
            APP_Menu_Update(now, key);
            break;
        case APP_SCREEN_CALC:
            BESO_Calc_Update(now, key);
            break;
        case APP_SCREEN_SNAKE:
            BESO_Snake_Update(now, key);
            break;
        case APP_SCREEN_CAR:
            BESO_Car_Update(now, key);
            break;
        case APP_SCREEN_FLAPPY:
            BESO_Flappy_Update(now, key);
            break;
        default:
            BESO_Login_Reset();
            break;
    }

    if ((g_app_led_until != 0U) && ((uint32)(now - g_app_led_until) < 0x80000000U))
    {
        APP_StatusLed_Set(0U);
        g_app_led_until = 0U;
    }
}

static void APP_Scheduler_Run(void)
{
    uint32 now = g_app_tick_ms;
    uint32 index;

    for (index = 0U; index < (sizeof(g_app_tasks) / sizeof(g_app_tasks[0])); index++)
    {
        app_task_t *task = &g_app_tasks[index];
        if ((uint32)(now - task->last_run_ms) >= task->interval_ms)
        {
            task->last_run_ms = now;
            task->task();
        }
    }
}

void BESO_UI_DrawHeader(const char *title, uint16 accent)
{
    HAL_ST7735S_FillRect(&g_app_tft, 0U, 0U, APP_TFT_WIDTH, APP_UI_HEADER_H, APP_COLOR_PANEL_DARK);
    HAL_ST7735S_FillRect(&g_app_tft, 0U, 0U, 4U, APP_UI_HEADER_H, accent);
    HAL_ST7735S_FillRect(&g_app_tft, 4U, 0U, (uint16)(APP_TFT_WIDTH - 4U), 1U, APP_COLOR_PANEL);
    HAL_ST7735S_FillRect(&g_app_tft, 0U, (uint16)(APP_UI_HEADER_H - 2U), APP_TFT_WIDTH, 2U, accent);
    HAL_ST7735S_DrawString(&g_app_tft, 9U, 5U, title, APP_COLOR_TEXT, APP_COLOR_PANEL_DARK);
}

void BESO_UI_DrawFooter(const char *text)
{
    uint16 y = (uint16)(APP_TFT_HEIGHT - APP_UI_FOOTER_H);
    HAL_ST7735S_FillRect(&g_app_tft, 0U, y, APP_TFT_WIDTH, APP_UI_FOOTER_H, APP_COLOR_PANEL_DARK);
    HAL_ST7735S_FillRect(&g_app_tft, 0U, y, APP_TFT_WIDTH, 1U, APP_COLOR_PANEL);
    if (text != NULL)
    {
        HAL_ST7735S_DrawString(&g_app_tft, APP_UI_MARGIN, (uint16)(y + 4U), text,
                               APP_COLOR_MUTED, APP_COLOR_PANEL_DARK);
    }
}

static void APP_UI_DrawMenuIcon(uint8 index, uint16 x, uint16 y, uint16 color, uint16 bg)
{
    HAL_ST7735S_FillRect(&g_app_tft, x, y, 16U, 14U, bg);

    switch (index)
    {
        case 0U:
            HAL_ST7735S_FillRect(&g_app_tft, x, y, 12U, 14U, color);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 1U), (uint16)(y + 1U), 10U, 12U, bg);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 3U), (uint16)(y + 3U), 6U, 2U, color);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 3U), (uint16)(y + 7U), 2U, 2U, color);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 7U), (uint16)(y + 7U), 2U, 2U, color);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 3U), (uint16)(y + 10U), 6U, 2U, color);
            break;

        case 1U:
            HAL_ST7735S_FillRect(&g_app_tft, x, (uint16)(y + 6U), 4U, 4U, APP_COLOR_SUCCESS);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 4U), (uint16)(y + 6U), 4U, 4U, APP_COLOR_SUCCESS);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 8U), (uint16)(y + 2U), 4U, 4U, APP_COLOR_SUCCESS);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 12U), (uint16)(y + 2U), 4U, 4U, APP_COLOR_SUCCESS);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 11U), (uint16)(y + 10U), 3U, 3U, APP_COLOR_DANGER);
            break;

        case 2U:
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 2U), y, 2U, 14U, APP_COLOR_PANEL);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 12U), y, 2U, 14U, APP_COLOR_PANEL);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 5U), (uint16)(y + 4U), 6U, 8U, color);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 6U), (uint16)(y + 2U), 4U, 3U, APP_COLOR_ACCENT_ALT);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 4U), (uint16)(y + 11U), 2U, 2U, APP_COLOR_TEXT);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 10U), (uint16)(y + 11U), 2U, 2U, APP_COLOR_TEXT);
            break;

        case 3U:
        default:
            HAL_ST7735S_FillRect(&g_app_tft, x, (uint16)(y + 2U), 3U, 12U, APP_COLOR_SUCCESS);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 13U), y, 3U, 5U, APP_COLOR_SUCCESS);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 13U), (uint16)(y + 9U), 3U, 5U, APP_COLOR_SUCCESS);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 5U), (uint16)(y + 6U), 6U, 5U, APP_COLOR_ACCENT);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 9U), (uint16)(y + 4U), 3U, 3U, APP_COLOR_ACCENT);
            HAL_ST7735S_FillRect(&g_app_tft, (uint16)(x + 11U), (uint16)(y + 5U), 1U, 1U, APP_COLOR_TEXT);
            break;
    }
}

static void APP_UI_DrawTile(uint8 index, uint8 selected)
{
    uint16 grid_w = (uint16)((2U * APP_UI_TILE_SIZE) + APP_UI_TILE_GAP);
    uint16 x0 = (uint16)((APP_TFT_WIDTH - grid_w) / 2U);
    uint16 y0 = (uint16)(APP_GAME_Y + 5U);
    uint16 col = (uint16)(index % 2U);
    uint16 row = (uint16)(index / 2U);
    uint16 x = (uint16)(x0 + (col * (APP_UI_TILE_SIZE + APP_UI_TILE_GAP)));
    uint16 y = (uint16)(y0 + (row * (APP_UI_TILE_SIZE + APP_UI_TILE_GAP)));
    uint16 border = (selected != 0U) ? APP_COLOR_ACCENT : APP_COLOR_PANEL_DARK;
    uint16 fill = (selected != 0U) ? APP_COLOR_PANEL : APP_COLOR_PANEL_DARK;
    uint16 text_color = (selected != 0U) ? APP_COLOR_TEXT : APP_COLOR_MUTED;
    uint16 label_w = BESO_UI_TextWidth(g_menu_short_items[index]);
    char idx[2];

    HAL_ST7735S_DrawPanel(&g_app_tft, x, y, APP_UI_TILE_SIZE, APP_UI_TILE_SIZE,
                          border, fill, APP_COLOR_PANEL);
    HAL_ST7735S_FillRect(&g_app_tft, x, y, APP_UI_TILE_SIZE, 3U,
                         (selected != 0U) ? APP_COLOR_ACCENT_ALT : APP_COLOR_PANEL);
    APP_UI_DrawMenuIcon(index, (uint16)(x + 13U), (uint16)(y + 8U),
                        (selected != 0U) ? APP_COLOR_ACCENT : APP_COLOR_MUTED, fill);
    HAL_ST7735S_DrawString(&g_app_tft,
                           (uint16)(x + ((APP_UI_TILE_SIZE - label_w) / 2U)),
                           (uint16)(y + 31U),
                           g_menu_short_items[index],
                           text_color,
                           fill);
    idx[0] = (char)('1' + index);
    idx[1] = '\0';
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(x + APP_UI_TILE_SIZE - 8U),
                           (uint16)(y + 5U), idx, text_color, fill);
}

void BESO_UI_ClearGameArea(uint16 color)
{
    HAL_ST7735S_FillRect(&g_app_tft, APP_GAME_X, APP_GAME_Y, APP_GAME_W, APP_GAME_H, color);
    HAL_ST7735S_FillRect(&g_app_tft, APP_GAME_X, APP_GAME_Y, APP_GAME_W, 1U, APP_COLOR_PANEL);
    HAL_ST7735S_FillRect(&g_app_tft, APP_GAME_X, (uint16)(APP_GAME_Y + APP_GAME_H - 1U),
                         APP_GAME_W, 1U, APP_COLOR_PANEL);
    HAL_ST7735S_FillRect(&g_app_tft, APP_GAME_X, APP_GAME_Y, 1U, APP_GAME_H, APP_COLOR_PANEL);
    HAL_ST7735S_FillRect(&g_app_tft, (uint16)(APP_GAME_X + APP_GAME_W - 1U),
                         APP_GAME_Y, 1U, APP_GAME_H, APP_COLOR_PANEL);
}

static void APP_DrawMenu(uint8 force)
{
    uint8 index;

    if (force != 0U)
    {
        HAL_ST7735S_FillScreen(&g_app_tft, APP_COLOR_BG);
        BESO_UI_DrawHeader("BESO OS", APP_COLOR_ACCENT);
        BESO_UI_DrawFooter("A/B Move  C/D/# Open");
    }

    for (index = 0U; index < 4U; index++)
    {
        APP_UI_DrawTile(index, (uint8)(index == g_app_menu_index));
    }
}

static void APP_Menu_Update(uint32 now_ms, uint8 key)
{
    if (key == 0U)
    {
        return;
    }

    if (key == 'A')
    {
        if (g_app_menu_index > 0U)
        {
            uint8 old_index = g_app_menu_index;
            g_app_menu_index--;
            APP_UI_DrawTile(old_index, 0U);
            APP_UI_DrawTile(g_app_menu_index, 1U);
        }
    }
    else if (key == 'B')
    {
        if (g_app_menu_index < 3U)
        {
            uint8 old_index = g_app_menu_index;
            g_app_menu_index++;
            APP_UI_DrawTile(old_index, 0U);
            APP_UI_DrawTile(g_app_menu_index, 1U);
        }
    }
    else if ((key == 'C') || (key == 'D') || (key == '#'))
    {
        switch (g_app_menu_index)
        {
            case 0U: BESO_Calc_Reset(); break;
            case 1U: BESO_Snake_Reset(); break;
            case 2U: BESO_Car_Reset(); break;
            case 3U: BESO_Flappy_Reset(); break;
            default: g_app_screen = APP_SCREEN_MENU; break;
        }
    }
    else if ((key >= '1') && (key <= '4'))
    {
        uint8 new_index = (uint8)(key - '1');
        if (new_index != g_app_menu_index)
        {
            uint8 old_index = g_app_menu_index;
            g_app_menu_index = new_index;
            APP_UI_DrawTile(old_index, 0U);
            APP_UI_DrawTile(g_app_menu_index, 1U);
        }
    }
}

void BESO_OS_SetScreen(app_screen_t screen)
{
    g_app_screen = screen;
}

void BESO_OS_ShowMenu(void)
{
    g_app_menu_index = 0U;
    g_app_screen = APP_SCREEN_MENU;
    APP_DrawMenu(1U);
}

uint32 BESO_OS_Rand(void)
{
    g_app_rng = (g_app_rng * 1664525UL) + 1013904223UL;
    return g_app_rng;
}

uint16 BESO_UI_TextWidth(const char *text)
{
    uint16 len = 0U;

    if (text == NULL)
    {
        return 0U;
    }

    while (*text != '\0')
    {
        len++;
        text++;
    }

    return (uint16)(len * (APP_FONT_W + APP_FONT_SPACING));
}

void BESO_IntToStr(sint32 value, char *out, uint8 out_len)
{
    char temp[12];
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
        temp[idx++] = (char)('0' + (value % 10));
        value /= 10;
    } while ((value > 0) && (idx < (sizeof(temp) - 1U)));

    if (negative != 0U)
    {
        temp[idx++] = '-';
    }

    while ((idx > 0U) && (out_idx < (out_len - 1U)))
    {
        out[out_idx++] = temp[--idx];
    }

    out[out_idx] = '\0';
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
void BESO_OS_Run(void)
{
    uint32 index;

    if (APP_InitPeripherals() == 0U)
    {
        while (1) { }
    }

    for (index = 0U; index < (sizeof(g_app_tasks) / sizeof(g_app_tasks[0])); index++)
    {
        g_app_tasks[index].last_run_ms = g_app_tick_ms;
    }

    BESO_Login_Reset();

    while (1)
    {
        APP_Scheduler_Run();
    }
}
