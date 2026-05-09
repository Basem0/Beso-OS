/*
 * beso_car.c
 *
 * Driving game for BESO OS.
 */

#include "beso_os_private.h"

typedef struct
{
    uint8 lane;
    uint8 obstacle_lane;
    sint16 obstacle_y;
    uint16 score;
    uint32 last_update_ms;
    uint8 needs_redraw;
} beso_car_t;

static beso_car_t g_car;

void BESO_Car_Reset(void)
{
    g_car.lane = 1U;
    g_car.obstacle_lane = (uint8)(BESO_OS_Rand() % 3U);
    g_car.obstacle_y = -20;
    g_car.score = 0U;
    g_car.last_update_ms = 0U;
    g_car.needs_redraw = 1U;
    BESO_OS_SetScreen(APP_SCREEN_CAR);
}

void BESO_Car_Update(uint32 now_ms, uint8 key)
{
    uint16 lane_w = (APP_GAME_W / 3U);
    sint16 car_w = 20;
    sint16 car_h = 14;
    sint16 car_x;
    sint16 car_y = (sint16)(APP_GAME_Y + APP_GAME_H - car_h - 4U);
    sint16 obs_x;
    sint16 obs_screen_y;
    char buf[12];

    if (key == '*')
    {
        BESO_OS_ShowMenu();
        return;
    }

    if (key == 'A' && g_car.lane > 0U) g_car.lane--;
    if (key == 'B' && g_car.lane < 2U) g_car.lane++;

    if (g_car.needs_redraw != 0U)
    {
        HAL_ST7735S_FillScreen(&g_app_tft, APP_COLOR_BG);
        BESO_UI_DrawHeader("BESO Drive", APP_COLOR_ACCENT);
        BESO_UI_DrawFooter("A/B Lane  *=Home");
        g_car.needs_redraw = 0U;
    }

    if ((uint32)(now_ms - g_car.last_update_ms) < 120U)
    {
        return;
    }
    g_car.last_update_ms = now_ms;

    g_car.obstacle_y += 8;
    if (g_car.obstacle_y > (sint16)(APP_GAME_H + 20U))
    {
        g_car.obstacle_y = -20;
        g_car.obstacle_lane = (uint8)(BESO_OS_Rand() % 3U);
        g_car.score++;
    }

    car_x = (uint16)((g_car.lane * lane_w) + (lane_w / 2U) - (car_w / 2U));
    obs_x = (uint16)((g_car.obstacle_lane * lane_w) + (lane_w / 2U) - (car_w / 2U));
    obs_screen_y = (sint16)(APP_GAME_Y + g_car.obstacle_y);

    if ((g_car.obstacle_lane == g_car.lane) &&
        (obs_screen_y + car_h >= car_y) && (obs_screen_y <= car_y + car_h))
    {
        BESO_Car_Reset();
        return;
    }

    BESO_UI_ClearGameArea(APP_COLOR_PANEL_DARK);

    HAL_ST7735S_FillRect(&g_app_tft, (uint16)(lane_w), APP_GAME_Y,
                         2U, APP_GAME_H, APP_COLOR_PANEL);
    HAL_ST7735S_FillRect(&g_app_tft, (uint16)(lane_w * 2U), APP_GAME_Y,
                         2U, APP_GAME_H, APP_COLOR_PANEL);
    HAL_ST7735S_FillRect(&g_app_tft, (uint16)(lane_w / 2U), (uint16)(APP_GAME_Y + 10U),
                         2U, 12U, APP_COLOR_MUTED);
    HAL_ST7735S_FillRect(&g_app_tft, (uint16)(lane_w + (lane_w / 2U)), (uint16)(APP_GAME_Y + 34U),
                         2U, 12U, APP_COLOR_MUTED);
    HAL_ST7735S_FillRect(&g_app_tft, (uint16)((lane_w * 2U) + (lane_w / 2U)), (uint16)(APP_GAME_Y + 58U),
                         2U, 12U, APP_COLOR_MUTED);

    HAL_ST7735S_FillRect(&g_app_tft, car_x, car_y, car_w, car_h, APP_COLOR_ACCENT_ALT);
    HAL_ST7735S_FillRect(&g_app_tft, (uint16)(car_x + 3U), (uint16)(car_y + 2U),
                         (uint16)(car_w - 6U), 4U, APP_COLOR_TEXT);
    HAL_ST7735S_FillRect(&g_app_tft, obs_x, (uint16)obs_screen_y, car_w, car_h, APP_COLOR_DANGER);
    HAL_ST7735S_FillRect(&g_app_tft, (uint16)(obs_x + 3U), (uint16)(obs_screen_y + 2U),
                         (uint16)(car_w - 6U), 4U, APP_COLOR_PANEL_DARK);

    BESO_IntToStr((sint32)g_car.score, buf, sizeof(buf));
    HAL_ST7735S_FillRect(&g_app_tft, (uint16)(APP_TFT_WIDTH - 46U), 2U,
                         44U, (uint16)(APP_UI_HEADER_H - 4U), APP_COLOR_PANEL_DARK);
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(APP_TFT_WIDTH - 44U), 6U, "S:",
                           APP_COLOR_MUTED, APP_COLOR_PANEL_DARK);
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(APP_TFT_WIDTH - 30U), 6U, buf,
                           APP_COLOR_TEXT, APP_COLOR_PANEL_DARK);
}
