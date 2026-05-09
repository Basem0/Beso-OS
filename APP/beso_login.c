/*
 * beso_login.c
 *
 * BESO OS login screen.
 */

#include "beso_os_private.h"

typedef struct
{
    uint16 pin;
    uint8 digits;
    uint8 error;
    uint8 dirty;
} beso_login_t;

static beso_login_t g_login = { 0U, 0U, 0U, 1U };

static void BESO_Login_Draw(uint8 force);

void BESO_Login_Reset(void)
{
    g_login.pin = 0U;
    g_login.digits = 0U;
    g_login.error = 0U;
    g_login.dirty = 1U;
    BESO_OS_SetScreen(APP_SCREEN_LOGIN);
    BESO_Login_Draw(1U);
}

void BESO_Login_Update(uint32 now_ms, uint8 key)
{
    (void)now_ms;

    if (key == 0U)
    {
        BESO_Login_Draw(0U);
        return;
    }

    if ((key >= '0') && (key <= '9'))
    {
        if (g_login.digits < 4U)
        {
            g_login.pin = (uint16)((g_login.pin * 10U) + (uint16)(key - '0'));
            g_login.digits++;
        }
        g_login.error = 0U;
        g_login.dirty = 1U;
    }
    else if (key == '*')
    {
        g_login.pin = 0U;
        g_login.digits = 0U;
        g_login.error = 0U;
        g_login.dirty = 1U;
    }
    else if ((key == '#') || (key == 'C') || (key == 'D'))
    {
        if ((g_login.digits == 4U) && (g_login.pin == APP_LOGIN_PIN))
        {
            BESO_OS_ShowMenu();
            return;
        }
        g_login.pin = 0U;
        g_login.digits = 0U;
        g_login.error = 1U;
        g_login.dirty = 1U;
    }

    BESO_Login_Draw(0U);
}

static void BESO_Login_Draw(uint8 force)
{
    uint16 panel_x = 22U;
    uint16 panel_y = 39U;
    uint16 panel_w = 116U;
    uint16 panel_h = 48U;
    uint8 i;
    char dots[5];

    if ((force == 0U) && (g_login.dirty == 0U))
    {
        return;
    }

    HAL_ST7735S_FillScreen(&g_app_tft, APP_COLOR_BG);
    BESO_UI_DrawHeader("BESO OS", APP_COLOR_ACCENT);
    BESO_UI_DrawFooter("PIN 1234  #OK  *CLR");

    HAL_ST7735S_DrawPanel(&g_app_tft, panel_x, panel_y, panel_w, panel_h,
                          (g_login.error != 0U) ? APP_COLOR_DANGER : APP_COLOR_ACCENT,
                          APP_COLOR_PANEL_DARK,
                          APP_COLOR_PANEL);
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(panel_x + 8U), (uint16)(panel_y + 8U),
                           "LOGIN", APP_COLOR_MUTED, APP_COLOR_PANEL_DARK);

    for (i = 0U; i < 4U; i++)
    {
        dots[i] = (i < g_login.digits) ? '*' : '-';
    }
    dots[4] = '\0';

    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(panel_x + 42U), (uint16)(panel_y + 22U),
                           dots, APP_COLOR_TEXT, APP_COLOR_PANEL_DARK);
    if (g_login.error != 0U)
    {
        HAL_ST7735S_DrawString(&g_app_tft, (uint16)(panel_x + 8U), (uint16)(panel_y + 36U),
                               "TRY AGAIN", APP_COLOR_DANGER, APP_COLOR_PANEL_DARK);
    }
    else
    {
        HAL_ST7735S_DrawString(&g_app_tft, (uint16)(panel_x + 8U), (uint16)(panel_y + 36U),
                               "SECURE START", APP_COLOR_ACCENT_ALT, APP_COLOR_PANEL_DARK);
    }

    g_login.dirty = 0U;
}
