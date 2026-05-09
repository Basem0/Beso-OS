/*
 * hal_led.c
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "hal_led.h"

#include "mcal_gpio.h"
#include "MCAL_RCC.h"

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
void HAL_LED_Init(void)
{
    MCAL_RCC_EnableGPIOAClock();
    MCAL_GPIO_Init(HAL_LED_GPIO_PORT,
                   MCAL_GPIO_PIN_5,
                   MCAL_GPIO_MODE_OUTPUT_PP,
                   MCAL_GPIO_SPEED_LOW);
}

void HAL_LED_On(void)
{
    MCAL_GPIO_WritePin(HAL_LED_GPIO_PORT, HAL_LED_PIN, GPIO_PIN_SET);
}

void HAL_LED_Off(void)
{
    MCAL_GPIO_WritePin(HAL_LED_GPIO_PORT, HAL_LED_PIN, GPIO_PIN_RESET);
}

void HAL_LED_Toggle(void)
{
    MCAL_GPIO_TogglePin(HAL_LED_GPIO_PORT, HAL_LED_PIN);
}
