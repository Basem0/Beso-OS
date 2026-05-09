/*
 * beso_snake.c
 *
 * Snake game for BESO OS.
 */

#include "beso_os_private.h"

typedef struct
{
    uint8 x[APP_SNAKE_MAX_LEN];
    uint8 y[APP_SNAKE_MAX_LEN];
    uint8 length;
    uint8 dir;
    uint8 food_x;
    uint8 food_y;
    uint32 last_update_ms;
    uint8 needs_redraw;
} beso_snake_t;

static beso_snake_t g_snake;

void BESO_Snake_Reset(void)
{
    uint8 i;

    g_snake.length = 3U;
    g_snake.dir = 1U;
    g_snake.last_update_ms = 0U;
    g_snake.needs_redraw = 1U;

    for (i = 0U; i < g_snake.length; i++)
    {
        g_snake.x[i] = (uint8)(6U - i);
        g_snake.y[i] = 8U;
    }

    g_snake.food_x = (uint8)(BESO_OS_Rand() % APP_SNAKE_COLS);
    g_snake.food_y = (uint8)(BESO_OS_Rand() % APP_SNAKE_ROWS);

    BESO_OS_SetScreen(APP_SCREEN_SNAKE);
}

void BESO_Snake_Update(uint32 now_ms, uint8 key)
{
    uint8 head_x;
    uint8 head_y;
    uint8 new_x;
    uint8 new_y;
    uint8 i;

    if (key == '*')
    {
        BESO_OS_ShowMenu();
        return;
    }

    if (key == 'A' && g_snake.dir != 2U) g_snake.dir = 0U;
    if (key == 'B' && g_snake.dir != 0U) g_snake.dir = 2U;
    if (key == 'C' && g_snake.dir != 1U) g_snake.dir = 3U;
    if (key == 'D' && g_snake.dir != 3U) g_snake.dir = 1U;

    if (g_snake.needs_redraw != 0U)
    {
        HAL_ST7735S_FillScreen(&g_app_tft, APP_COLOR_BG);
        BESO_UI_DrawHeader("BESO Snake", APP_COLOR_ACCENT_ALT);
        BESO_UI_DrawFooter("A/B/C/D Move  *=Home");
        BESO_UI_ClearGameArea(APP_COLOR_PANEL_DARK);
        g_snake.needs_redraw = 0U;
    }

    if ((uint32)(now_ms - g_snake.last_update_ms) < 150U)
    {
        return;
    }
    g_snake.last_update_ms = now_ms;

    head_x = g_snake.x[0];
    head_y = g_snake.y[0];
    new_x = head_x;
    new_y = head_y;

    if (g_snake.dir == 0U) new_y--;
    if (g_snake.dir == 1U) new_x++;
    if (g_snake.dir == 2U) new_y++;
    if (g_snake.dir == 3U) new_x--;

    if ((new_x >= APP_SNAKE_COLS) || (new_y >= APP_SNAKE_ROWS))
    {
        BESO_Snake_Reset();
        return;
    }

    for (i = 0U; i < g_snake.length; i++)
    {
        if ((g_snake.x[i] == new_x) && (g_snake.y[i] == new_y))
        {
            BESO_Snake_Reset();
            return;
        }
    }

    if ((new_x == g_snake.food_x) && (new_y == g_snake.food_y))
    {
        if (g_snake.length < APP_SNAKE_MAX_LEN)
        {
            g_snake.length++;
        }
        g_snake.food_x = (uint8)(BESO_OS_Rand() % APP_SNAKE_COLS);
        g_snake.food_y = (uint8)(BESO_OS_Rand() % APP_SNAKE_ROWS);
    }
    else
    {
        uint8 tail_x = g_snake.x[g_snake.length - 1U];
        uint8 tail_y = g_snake.y[g_snake.length - 1U];
        HAL_ST7735S_FillRect(&g_app_tft,
                             (uint16)(APP_SNAKE_OFFSET_X + (tail_x * APP_SNAKE_CELL)),
                             (uint16)(APP_SNAKE_OFFSET_Y + (tail_y * APP_SNAKE_CELL)),
                             APP_SNAKE_CELL,
                             APP_SNAKE_CELL,
                             APP_COLOR_PANEL_DARK);
    }

    for (i = g_snake.length - 1U; i > 0U; i--)
    {
        g_snake.x[i] = g_snake.x[i - 1U];
        g_snake.y[i] = g_snake.y[i - 1U];
    }
    g_snake.x[0] = new_x;
    g_snake.y[0] = new_y;

    HAL_ST7735S_FillRect(&g_app_tft,
                         (uint16)(APP_SNAKE_OFFSET_X + (new_x * APP_SNAKE_CELL)),
                         (uint16)(APP_SNAKE_OFFSET_Y + (new_y * APP_SNAKE_CELL)),
                         APP_SNAKE_CELL,
                         APP_SNAKE_CELL,
                         HAL_ST7735S_COLOR_GREEN);

    HAL_ST7735S_FillRect(&g_app_tft,
                         (uint16)(APP_SNAKE_OFFSET_X + (g_snake.food_x * APP_SNAKE_CELL)),
                         (uint16)(APP_SNAKE_OFFSET_Y + (g_snake.food_y * APP_SNAKE_CELL)),
                         APP_SNAKE_CELL,
                         APP_SNAKE_CELL,
                         HAL_ST7735S_COLOR_RED);
}
