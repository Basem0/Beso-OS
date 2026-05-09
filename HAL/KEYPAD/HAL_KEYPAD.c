/*
 * HAL_KEYPAD.c
 *
 * 4x4 matrix keypad driver.
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "HAL_KEYPAD.h"

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
static const uint8 g_hal_keypad_default_map[HAL_KEYPAD_MAX_ROWS][HAL_KEYPAD_MAX_COLS] = {
	{ '1', '2', '3', 'A' },
	{ '4', '5', '6', 'B' },
	{ '7', '8', '9', 'C' },
	{ '*', '0', '#', 'D' }
};

static hal_keypad_status_t HAL_KEYPAD_ValidateConfig(const hal_keypad_config_t *config);
static void HAL_KEYPAD_SetAllRowsInactive(hal_keypad_t *keypad);
static void HAL_KEYPAD_WaitMs(hal_keypad_t *keypad, uint32 milliseconds);

/* ================================================================ */
/* ================= Static Function Definitions ================== */
/* ================================================================ */
static hal_keypad_status_t HAL_KEYPAD_ValidateConfig(const hal_keypad_config_t *config)
{
	uint8 index;

	if ((config == NULL) ||
		(config->rows == NULL) ||
		(config->cols == NULL) ||
		(config->delay_ms == NULL) ||
		(config->row_count == 0U) ||
		(config->row_count > HAL_KEYPAD_MAX_ROWS) ||
		(config->col_count == 0U) ||
		(config->col_count > HAL_KEYPAD_MAX_COLS))
	{
		return HAL_KEYPAD_STATUS_INVALID_PARAM;
	}

	for (index = 0U; index < config->row_count; index++)
	{
		if ((config->rows[index].port == NULL) || (config->rows[index].pin > MCAL_GPIO_PIN_15))
		{
			return HAL_KEYPAD_STATUS_INVALID_PARAM;
		}
	}

	for (index = 0U; index < config->col_count; index++)
	{
		if ((config->cols[index].port == NULL) || (config->cols[index].pin > MCAL_GPIO_PIN_15))
		{
			return HAL_KEYPAD_STATUS_INVALID_PARAM;
		}
	}

	return HAL_KEYPAD_STATUS_OK;
}

static void HAL_KEYPAD_SetAllRowsInactive(hal_keypad_t *keypad)
{
	uint8 row;

	for (row = 0U; row < keypad->row_count; row++)
	{
		MCAL_GPIO_WritePin(keypad->rows[row].port, keypad->rows[row].pin, GPIO_PIN_SET);
	}
}

static void HAL_KEYPAD_WaitMs(hal_keypad_t *keypad, uint32 milliseconds)
{
	if ((keypad != NULL) && (keypad->delay_ms != NULL) && (milliseconds > 0U))
	{
		keypad->delay_ms(milliseconds);
	}
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
hal_keypad_status_t HAL_KEYPAD_Init(hal_keypad_t *keypad, const hal_keypad_config_t *config)
{
	uint8 row;
	uint8 col;
	hal_keypad_status_t status;

	if (keypad == NULL)
	{
		return HAL_KEYPAD_STATUS_INVALID_PARAM;
	}

	status = HAL_KEYPAD_ValidateConfig(config);
	if (status != HAL_KEYPAD_STATUS_OK)
	{
		keypad->is_initialized = 0U;
		return status;
	}

	keypad->row_count = config->row_count;
	keypad->col_count = config->col_count;
	keypad->debounce_ms = config->debounce_ms;
	keypad->delay_ms = config->delay_ms;
	keypad->is_initialized = 1U;

	for (row = 0U; row < keypad->row_count; row++)
	{
		keypad->rows[row] = config->rows[row];
		MCAL_GPIO_Init(keypad->rows[row].port,
					   keypad->rows[row].pin,
					   MCAL_GPIO_MODE_OUTPUT_PP,
					   MCAL_GPIO_SPEED_LOW);
		MCAL_GPIO_WritePin(keypad->rows[row].port, keypad->rows[row].pin, GPIO_PIN_SET);
	}

	for (col = 0U; col < keypad->col_count; col++)
	{
		keypad->cols[col] = config->cols[col];
		MCAL_GPIO_Init(keypad->cols[col].port,
					   keypad->cols[col].pin,
					   MCAL_GPIO_MODE_INPUT_PULLUP,
					   MCAL_GPIO_SPEED_LOW);
	}

	return HAL_KEYPAD_STATUS_OK;
}

hal_keypad_status_t HAL_KEYPAD_GetKey(hal_keypad_t *keypad, uint8 *key)
{
	uint8 row;
	uint8 col;

	if ((keypad == NULL) || (key == NULL) || (keypad->is_initialized == 0U))
	{
		return HAL_KEYPAD_STATUS_INVALID_PARAM;
	}

	*key = 0U;

	for (row = 0U; row < keypad->row_count; row++)
	{
		HAL_KEYPAD_SetAllRowsInactive(keypad);
		MCAL_GPIO_WritePin(keypad->rows[row].port, keypad->rows[row].pin, GPIO_PIN_RESET);
		HAL_KEYPAD_WaitMs(keypad, 1U);

		for (col = 0U; col < keypad->col_count; col++)
		{
			if (MCAL_GPIO_ReadPin(keypad->cols[col].port, keypad->cols[col].pin) == GPIO_PIN_RESET)
			{
				HAL_KEYPAD_WaitMs(keypad, keypad->debounce_ms);

				if (MCAL_GPIO_ReadPin(keypad->cols[col].port, keypad->cols[col].pin) == GPIO_PIN_RESET)
				{
					*key = g_hal_keypad_default_map[row][col];

					while (MCAL_GPIO_ReadPin(keypad->cols[col].port, keypad->cols[col].pin) == GPIO_PIN_RESET)
					{
						HAL_KEYPAD_WaitMs(keypad, 1U);
					}

					HAL_KEYPAD_WaitMs(keypad, keypad->debounce_ms);
					HAL_KEYPAD_SetAllRowsInactive(keypad);
					return HAL_KEYPAD_STATUS_OK;
				}
			}
		}
	}

	HAL_KEYPAD_SetAllRowsInactive(keypad);
	return HAL_KEYPAD_STATUS_OK;
}

hal_keypad_status_t HAL_KEYPAD_Scan(hal_keypad_t *keypad, uint8 *key)
{
	uint8 row;
	uint8 col;

	if ((keypad == NULL) || (key == NULL) || (keypad->is_initialized == 0U))
	{
		return HAL_KEYPAD_STATUS_INVALID_PARAM;
	}

	*key = 0U;

	for (row = 0U; row < keypad->row_count; row++)
	{
		HAL_KEYPAD_SetAllRowsInactive(keypad);
		MCAL_GPIO_WritePin(keypad->rows[row].port, keypad->rows[row].pin, GPIO_PIN_RESET);

		for (col = 0U; col < keypad->col_count; col++)
		{
			if (MCAL_GPIO_ReadPin(keypad->cols[col].port, keypad->cols[col].pin) == GPIO_PIN_RESET)
			{
				*key = g_hal_keypad_default_map[row][col];
				HAL_KEYPAD_SetAllRowsInactive(keypad);
				return HAL_KEYPAD_STATUS_OK;
			}
		}
	}

	HAL_KEYPAD_SetAllRowsInactive(keypad);
	return HAL_KEYPAD_STATUS_OK;
}

hal_keypad_status_t HAL_KEYPAD_ReadMatrix(hal_keypad_t *keypad, uint8 *row_mask, uint8 settle_ms)
{
	uint8 row;
	uint8 col;

	if ((keypad == NULL) || (row_mask == NULL) || (keypad->is_initialized == 0U))
	{
		return HAL_KEYPAD_STATUS_INVALID_PARAM;
	}

	HAL_KEYPAD_SetAllRowsInactive(keypad);

	for (row = 0U; row < keypad->row_count; row++)
	{
		uint8 mask = 0U;
		MCAL_GPIO_WritePin(keypad->rows[row].port, keypad->rows[row].pin, GPIO_PIN_RESET);
		HAL_KEYPAD_WaitMs(keypad, settle_ms);

		for (col = 0U; col < keypad->col_count; col++)
		{
			if (MCAL_GPIO_ReadPin(keypad->cols[col].port, keypad->cols[col].pin) == GPIO_PIN_RESET)
			{
				mask |= (uint8)(1U << col);
			}
		}

		row_mask[row] = mask;
		MCAL_GPIO_WritePin(keypad->rows[row].port, keypad->rows[row].pin, GPIO_PIN_SET);
	}

	return HAL_KEYPAD_STATUS_OK;
}

hal_keypad_status_t HAL_KEYPAD_KeyFromMatrix(hal_keypad_t *keypad,
											 const uint8 *row_mask,
											 uint8 *key)
{
	uint8 row;
	uint8 col;

	if ((keypad == NULL) || (row_mask == NULL) || (key == NULL) || (keypad->is_initialized == 0U))
	{
		return HAL_KEYPAD_STATUS_INVALID_PARAM;
	}

	*key = 0U;

	for (row = 0U; row < keypad->row_count; row++)
	{
		if (row_mask[row] != 0U)
		{
			for (col = 0U; col < keypad->col_count; col++)
			{
				if ((row_mask[row] & (uint8)(1U << col)) != 0U)
				{
					*key = g_hal_keypad_default_map[row][col];
					return HAL_KEYPAD_STATUS_OK;
				}
			}
		}
	}

	return HAL_KEYPAD_STATUS_OK;
}
