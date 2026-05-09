/*
 * HAL_KEYPAD.h
 *
 * 4x4 matrix keypad driver.
 */

#ifndef HAL_KEYPAD_H
#define HAL_KEYPAD_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "PLATFORM_TYPES.h"
#include "mcal_gpio.h"

/* ================================================================ */
/* =========================== Macros ============================= */
/* ================================================================ */
#define HAL_KEYPAD_MAX_ROWS (4U)
#define HAL_KEYPAD_MAX_COLS (4U)

/* ================================================================ */
/* ======================= Data Structures ======================== */
/* ================================================================ */
typedef enum
{
	HAL_KEYPAD_STATUS_OK = 0U,
	HAL_KEYPAD_STATUS_ERROR,
	HAL_KEYPAD_STATUS_INVALID_PARAM
} hal_keypad_status_t;

typedef struct
{
	GPIO_RegDef_t *port;
	mcal_gpio_pin_t pin;
} hal_keypad_pin_t;

typedef void (*hal_keypad_delay_ms_t)(uint32 milliseconds);

typedef struct
{
	const hal_keypad_pin_t *rows;
	const hal_keypad_pin_t *cols;
	uint8 row_count;
	uint8 col_count;
	uint8 debounce_ms;
	hal_keypad_delay_ms_t delay_ms;
} hal_keypad_config_t;

typedef struct
{
	hal_keypad_pin_t rows[HAL_KEYPAD_MAX_ROWS];
	hal_keypad_pin_t cols[HAL_KEYPAD_MAX_COLS];
	uint8 row_count;
	uint8 col_count;
	uint8 debounce_ms;
	uint8 is_initialized;
	hal_keypad_delay_ms_t delay_ms;
} hal_keypad_t;

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
hal_keypad_status_t HAL_KEYPAD_Init(hal_keypad_t *keypad, const hal_keypad_config_t *config);
hal_keypad_status_t HAL_KEYPAD_Scan(hal_keypad_t *keypad, uint8 *key);
hal_keypad_status_t HAL_KEYPAD_GetKey(hal_keypad_t *keypad, uint8 *key);
hal_keypad_status_t HAL_KEYPAD_ReadMatrix(hal_keypad_t *keypad, uint8 *row_mask, uint8 settle_ms);
hal_keypad_status_t HAL_KEYPAD_KeyFromMatrix(hal_keypad_t *keypad,
                                             const uint8 *row_mask,
                                             uint8 *key);

#endif
