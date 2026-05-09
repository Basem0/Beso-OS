/*
 * beso_calc.c
 *
 * Calculator app for BESO OS.
 */

#include "beso_os_private.h"

typedef struct
{
    sint32 operand_a;
    sint32 operand_b;
    char op;
    uint8 entering_second;
    uint8 has_result;
    sint32 result;
    uint8 dirty;
} beso_calc_t;

static beso_calc_t g_calc = { 0, 0, '+', 0U, 0U, 0, 1U };

static void BESO_Calc_Draw(uint8 force);

void BESO_Calc_Reset(void)
{
    g_calc.operand_a = 0;
    g_calc.operand_b = 0;
    g_calc.op = '+';
    g_calc.entering_second = 0U;
    g_calc.has_result = 0U;
    g_calc.result = 0;
    g_calc.dirty = 1U;
    BESO_OS_SetScreen(APP_SCREEN_CALC);
    BESO_Calc_Draw(1U);
}

void BESO_Calc_Update(uint32 now_ms, uint8 key)
{
    (void)now_ms;

    if (key == '*')
    {
        BESO_OS_ShowMenu();
        return;
    }

    if ((key >= '0') && (key <= '9'))
    {
        sint32 *target = (g_calc.entering_second != 0U) ? &g_calc.operand_b : &g_calc.operand_a;
        if (*target < 1000000)
        {
            *target = (*target * 10) + (sint32)(key - '0');
        }
        g_calc.dirty = 1U;
    }
    else if ((key == 'A') || (key == 'B') || (key == 'C') || (key == 'D'))
    {
        if (key == 'A') g_calc.op = '+';
        if (key == 'B') g_calc.op = '-';
        if (key == 'C') g_calc.op = '*';
        if (key == 'D') g_calc.op = '/';
        g_calc.entering_second = 1U;
        g_calc.dirty = 1U;
    }
    else if (key == '#')
    {
        sint32 a = g_calc.operand_a;
        sint32 b = g_calc.operand_b;
        g_calc.has_result = 1U;

        switch (g_calc.op)
        {
            case '+': g_calc.result = a + b; break;
            case '-': g_calc.result = a - b; break;
            case '*': g_calc.result = a * b; break;
            case '/': g_calc.result = (b == 0) ? 0 : (a / b); break;
            default: g_calc.result = 0; break;
        }
        g_calc.dirty = 1U;
    }

    BESO_Calc_Draw(0U);
}

static void BESO_Calc_Draw(uint8 force)
{
    char buf[20];
    uint16 x = APP_UI_MARGIN;
    uint16 w = (uint16)(APP_TFT_WIDTH - (APP_UI_MARGIN * 2U));
    uint16 half_w = (uint16)((w - APP_UI_CARD_GAP) / 2U);
    uint16 right_x = (uint16)(x + half_w + APP_UI_CARD_GAP);
    uint16 y = (uint16)(APP_GAME_Y + 3U);
    uint16 result_h = 29U;
    uint16 field_y = (uint16)(y + result_h + 4U);
    uint16 field_h = 23U;
    uint16 op_y = (uint16)(field_y + field_h + 4U);
    uint16 op_w = (uint16)((w - 9U) / 4U);
    uint16 op_h = 24U;
    uint16 op_x;
    uint8 i;
    const char ops[4] = { '+', '-', '*', '/' };

    if ((force == 0U) && (g_calc.dirty == 0U))
    {
        return;
    }

    HAL_ST7735S_FillScreen(&g_app_tft, APP_COLOR_BG);
    BESO_UI_DrawHeader("BESO Calc", APP_COLOR_ACCENT);
    BESO_UI_DrawFooter("A+ B- Cx D/  #=  *Back");

    HAL_ST7735S_DrawPanel(&g_app_tft, x, y, w, result_h,
                          APP_COLOR_ACCENT, APP_COLOR_PANEL, APP_COLOR_PANEL);
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(x + 5U), (uint16)(y + 5U), "RESULT",
                           APP_COLOR_MUTED, APP_COLOR_PANEL);
    if (g_calc.has_result != 0U)
    {
        BESO_IntToStr(g_calc.result, buf, sizeof(buf));
    }
    else
    {
        buf[0] = '-';
        buf[1] = '-';
        buf[2] = '\0';
    }
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(x + 55U), (uint16)(y + 13U), buf,
                           APP_COLOR_TEXT, APP_COLOR_PANEL);

    HAL_ST7735S_DrawPanel(&g_app_tft, x, field_y, half_w, field_h,
                          (g_calc.entering_second == 0U) ? APP_COLOR_ACCENT_ALT : APP_COLOR_PANEL_DARK,
                          APP_COLOR_PANEL_DARK, APP_COLOR_PANEL);
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(x + 5U), (uint16)(field_y + 4U), "A",
                           APP_COLOR_ACCENT_ALT, APP_COLOR_PANEL_DARK);
    BESO_IntToStr(g_calc.operand_a, buf, sizeof(buf));
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(x + 22U), (uint16)(field_y + 11U), buf,
                           APP_COLOR_TEXT, APP_COLOR_PANEL_DARK);

    HAL_ST7735S_DrawPanel(&g_app_tft, right_x, field_y, half_w, field_h,
                          (g_calc.entering_second != 0U) ? APP_COLOR_ACCENT_ALT : APP_COLOR_PANEL_DARK,
                          APP_COLOR_PANEL_DARK, APP_COLOR_PANEL);
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(right_x + 5U), (uint16)(field_y + 4U), "B",
                           APP_COLOR_ACCENT_ALT, APP_COLOR_PANEL_DARK);
    BESO_IntToStr(g_calc.operand_b, buf, sizeof(buf));
    HAL_ST7735S_DrawString(&g_app_tft, (uint16)(right_x + 22U), (uint16)(field_y + 11U), buf,
                           APP_COLOR_TEXT, APP_COLOR_PANEL_DARK);

    for (i = 0U; i < 4U; i++)
    {
        uint16 border = (g_calc.op == ops[i]) ? APP_COLOR_ACCENT : APP_COLOR_PANEL_DARK;
        uint16 fill = (g_calc.op == ops[i]) ? APP_COLOR_PANEL : APP_COLOR_PANEL_DARK;
        op_x = (uint16)(x + (i * (op_w + 3U)));
        HAL_ST7735S_DrawPanel(&g_app_tft, op_x, op_y, op_w, op_h, border, fill, APP_COLOR_PANEL);
        buf[0] = (ops[i] == '*') ? 'x' : ops[i];
        buf[1] = '\0';
        HAL_ST7735S_DrawString(&g_app_tft, (uint16)(op_x + ((op_w - BESO_UI_TextWidth(buf)) / 2U)),
                               (uint16)(op_y + 9U), buf, APP_COLOR_TEXT, fill);
    }

    g_calc.dirty = 0U;
}
