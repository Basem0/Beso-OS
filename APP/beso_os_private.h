/*
 * beso_os_private.h
 *
 * Internal BESO OS screen APIs shared by the OS core and app modules.
 */

#ifndef BESO_OS_PRIVATE_H
#define BESO_OS_PRIVATE_H

#include "HAL_ST7735S.h"

#define APP_TFT_WIDTH        (160U)
#define APP_TFT_HEIGHT       (128U)

#define APP_UI_MARGIN        (4U)
#define APP_UI_HEADER_H      (18U)
#define APP_UI_FOOTER_H      (14U)
#define APP_UI_CARD_GAP      (4U)
#define APP_UI_TILE_SIZE     (42U)
#define APP_UI_TILE_GAP      (8U)

#define APP_FONT_W           (5U)
#define APP_FONT_SPACING     (1U)

#define APP_COLOR_BG         (0x0208U)
#define APP_COLOR_PANEL      (0x1B2CU)
#define APP_COLOR_PANEL_DARK (0x0965U)
#define APP_COLOR_ACCENT     (0xFE60U)
#define APP_COLOR_ACCENT_ALT (0x05FFU)
#define APP_COLOR_TEXT       (0xFFFFU)
#define APP_COLOR_MUTED      (0xA596U)
#define APP_COLOR_SUCCESS    (0x67E0U)
#define APP_COLOR_DANGER     (0xF9E7U)

#define APP_LOGIN_PIN        (6666U)

#define APP_GAME_X           (0U)
#define APP_GAME_Y           (APP_UI_HEADER_H)
#define APP_GAME_W           (APP_TFT_WIDTH)
#define APP_GAME_H           (APP_TFT_HEIGHT - APP_UI_HEADER_H - APP_UI_FOOTER_H)

#define APP_SNAKE_CELL       (8U)
#define APP_SNAKE_COLS       (APP_GAME_W / APP_SNAKE_CELL)
#define APP_SNAKE_ROWS       (APP_GAME_H / APP_SNAKE_CELL)
#define APP_SNAKE_OFFSET_X   (APP_GAME_X + ((APP_GAME_W - (APP_SNAKE_COLS * APP_SNAKE_CELL)) / 2U))
#define APP_SNAKE_OFFSET_Y   (APP_GAME_Y + ((APP_GAME_H - (APP_SNAKE_ROWS * APP_SNAKE_CELL)) / 2U))
#define APP_SNAKE_MAX_LEN    (48U)

typedef enum
{
    APP_SCREEN_LOGIN = 0U,
    APP_SCREEN_MENU,
    APP_SCREEN_CALC,
    APP_SCREEN_SNAKE,
    APP_SCREEN_CAR,
    APP_SCREEN_FLAPPY
} app_screen_t;

extern hal_st7735s_t g_app_tft;

void BESO_OS_SetScreen(app_screen_t screen);
void BESO_OS_ShowMenu(void);
uint32 BESO_OS_Rand(void);

void BESO_UI_DrawHeader(const char *title, uint16 accent);
void BESO_UI_DrawFooter(const char *text);
void BESO_UI_ClearGameArea(uint16 color);
uint16 BESO_UI_TextWidth(const char *text);
void BESO_IntToStr(sint32 value, char *out, uint8 out_len);

void BESO_Login_Reset(void);
void BESO_Login_Update(uint32 now_ms, uint8 key);

void BESO_Calc_Reset(void);
void BESO_Calc_Update(uint32 now_ms, uint8 key);

void BESO_Snake_Reset(void);
void BESO_Snake_Update(uint32 now_ms, uint8 key);

void BESO_Car_Reset(void);
void BESO_Car_Update(uint32 now_ms, uint8 key);

void BESO_Flappy_Reset(void);
void BESO_Flappy_Update(uint32 now_ms, uint8 key);

#endif
