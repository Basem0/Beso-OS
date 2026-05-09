/*
 * beso_flappy.c
 *
 * Flappy-style game for BESO OS.
 */

#include "beso_os_private.h"

typedef struct
{
    sint16 bird_y;
    sint16 bird_v;
    sint16 pipe_x;
    sint16 gap_y;
    uint16 score;
    uint32 last_update_ms;
    uint8 needs_redraw;
} beso_flappy_t;

static beso_flappy_t g_flappy;

void BESO_Flappy_Reset(void)
{
    g_flappy.bird_y = (sint16)(APP_GAME_H / 2U);
    g_flappy.bird_v = 0;
    g_flappy.pipe_x = (sint16)(APP_GAME_W + 10U);
    g_flappy.gap_y = (sint16)(APP_GAME_H / 2U);
    g_flappy.score = 0U;
    g_flappy.last_update_ms = 0U;
    g_flappy.needs_redraw = 1U;
    BESO_OS_SetScreen(APP_SCREEN_FLAPPY);
}

void BESO_Flappy_Update(uint32 now_ms, uint8 key)
{
    sint16 bird_x = 30;
    sint16 pipe_w = 18;
    sint16 gap_h = 40;
    sint16 top_h;
    sint16 bot_y;
    char buf[12];

    if (key == '*')
    {
        BESO_OS_ShowMenu();
        return;
    }

    if ((key == 'A') || (key == '#'))
    {
        g_flappy.bird_v = -6;
    }

    if (g_flappy.needs_redraw != 0U)
    {
        HAL_ST7735S_FillScreen(&g_app_tft, APP_COLOR_BG);
        BESO_UI_DrawHeader("BESO Fly", APP_COLOR_ACCENT);
        BESO_UI_DrawFooter("A/# Flap  *=Home");
        g_flappy.needs_redraw = 0U;
    }

    if ((uint32)(now_ms - g_flappy.last_update_ms) < 80U)
    {
        return;
    }
    g_flappy.last_update_ms = now_ms;

    g_flappy.bird_v += 1;
    g_flappy.bird_y += g_flappy.bird_v;

    g_flappy.pipe_x -= 6;
    if (g_flappy.pipe_x < -pipe_w)
    {
        g_flappy.pipe_x = (sint16)(APP_GAME_W + 10U);
        g_flappy.gap_y = (sint16)(10 + (BESO_OS_Rand() % (uint32)(APP_GAME_H - gap_h - 20)));
        g_flappy.score++;
    }

    if ((g_flappy.bird_y < 0) || (g_flappy.bird_y > (sint16)(APP_GAME_H - 8)))
    {
        BESO_Flappy_Reset();
        return;
    }

    top_h = g_flappy.gap_y;
    bot_y = (sint16)(g_flappy.gap_y + gap_h);

    if ((bird_x + 6 >= g_flappy.pipe_x) && (bird_x <= g_flappy.pipe_x + pipe_w))
    {
        if ((g_flappy.bird_y < top_h) || (g_flappy.bird_y > bot_y))
        {
            BESO_Flappy_Reset();
            return;
        }
    }

    BESO_UI_ClearGameArea(APP_COLOR_PANEL_DARK);
    HAL_ST7735S_FillRect(&g_app_tft, 1U, (uint16)(APP_GAME_Y + APP_GAME_H - 8U),
                         (uint16)(APP_GAME_W - 2U), 7U, APP_COLOR_PANEL);

    HAL_ST7735S_FillRect(&g_app_tft, (uint16)g_flappy.pipe_x, APP_GAME_Y,
                         (uint16)pipe_w, (uint16)top_h, HAL_ST7735S_COLOR_GREEN);
    HAL_ST7735S_FillRect(&g_app_tft,
                         (uint16)g_flappy.pipe_x,
                         (uint16)(APP_GAME_Y + bot_y),
                         (uint16)pipe_w,
                         (uint16)(APP_GAME_H - (uint16)bot_y),
                         HAL_ST7735S_COLOR_GREEN);

    HAL_ST7735S_FillRect(&g_app_tft, (uint16)bird_x, (uint16)(APP_GAME_Y + g_flappy.bird_y),
                         6U, 6U, APP_COLOR_ACCENT);
    HAL_ST7735S_FillRect(&g_app_tft, (uint16)(bird_x + 5U),
                         (uint16)(APP_GAME_Y + g_flappy.bird_y + 2U),
                         2U, 2U, APP_COLOR_TEXT);

    BESO_IntToStr((sint32)g_flappy.score, buf, sizeof(buf));
    HAL_ST7735S_FillRect(&g_app_tft, (uint16)(APP_TFT_WIDTH - 46U), 2U,
                         44U, (uint16)(APP_UI_HEADER_H - 4U), APP_COLOR_PANEL_DARK);
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(APP_TFT_WIDTH - 44U), 6U, "S:",
                           APP_COLOR_MUTED, APP_COLOR_PANEL_DARK);
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(APP_TFT_WIDTH - 30U), 6U, buf,
                           APP_COLOR_TEXT, APP_COLOR_PANEL_DARK);
}
