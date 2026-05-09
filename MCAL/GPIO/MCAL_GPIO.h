/*
 * mcal_gpio.h
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

#ifndef MCAL_GPIO_H
#define MCAL_GPIO_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "PLATFORM_TYPES.h"
#include "STM32F401RC.h"

/* ================================================================ */
/* ========================= Private Macros ======================= */
/* ================================================================ */
#define MCAL_GPIO_INVALID_PORT_INDEX (0xFFU)
#define MCAL_GPIO_LOCK_KEY_BIT       (16U)
#define MCAL_GPIO_MODE_MASK          (0x03U)
#define MCAL_GPIO_OTYPE_MASK         (0x04U)
#define MCAL_GPIO_PUPD_MASK          (0x18U)

/* ================================================================ */
/* =============== Macros Configuration References ================ */
/* ================================================================ */
/**
 * @brief GPIO pin numbers.
 */
typedef enum
{
    MCAL_GPIO_PIN_0 = 0U,
    MCAL_GPIO_PIN_1,
    MCAL_GPIO_PIN_2,
    MCAL_GPIO_PIN_3,
    MCAL_GPIO_PIN_4,
    MCAL_GPIO_PIN_5,
    MCAL_GPIO_PIN_6,
    MCAL_GPIO_PIN_7,
    MCAL_GPIO_PIN_8,
    MCAL_GPIO_PIN_9,
    MCAL_GPIO_PIN_10,
    MCAL_GPIO_PIN_11,
    MCAL_GPIO_PIN_12,
    MCAL_GPIO_PIN_13,
    MCAL_GPIO_PIN_14,
    MCAL_GPIO_PIN_15
} mcal_gpio_pin_t;

/**
 * @brief GPIO output speed options.
 */
typedef enum
{
    MCAL_GPIO_SPEED_LOW = 0x0U,
    MCAL_GPIO_SPEED_MEDIUM = 0x1U,
    MCAL_GPIO_SPEED_HIGH = 0x2U,
    MCAL_GPIO_SPEED_VERY_HIGH = 0x3U
} mcal_gpio_speed_t;

/**
 * @brief GPIO operating modes including pull configuration.
 */
typedef enum
{
    MCAL_GPIO_MODE_INPUT_FLOATING = 0x00U,
    MCAL_GPIO_MODE_INPUT_PULLUP = 0x08U,
    MCAL_GPIO_MODE_INPUT_PULLDOWN = 0x10U,

    MCAL_GPIO_MODE_OUTPUT_PP = 0x01U,
    MCAL_GPIO_MODE_OUTPUT_OD = 0x05U,
    MCAL_GPIO_MODE_OUTPUT_PP_PULLUP = 0x09U,
    MCAL_GPIO_MODE_OUTPUT_PP_PULLDOWN = 0x11U,
    MCAL_GPIO_MODE_OUTPUT_OD_PULLUP = 0x0DU,
    MCAL_GPIO_MODE_OUTPUT_OD_PULLDOWN = 0x15U,

    MCAL_GPIO_MODE_AF_PP = 0x02U,
    MCAL_GPIO_MODE_AF_OD = 0x06U,
    MCAL_GPIO_MODE_AF_PP_PULLUP = 0x0AU,
    MCAL_GPIO_MODE_AF_PP_PULLDOWN = 0x12U,
    MCAL_GPIO_MODE_AF_OD_PULLUP = 0x0EU,
    MCAL_GPIO_MODE_AF_OD_PULLDOWN = 0x16U,

    MCAL_GPIO_MODE_ANALOG = 0x03U
} mcal_gpio_mode_t;

/**
 * @brief GPIO output pin state.
 * @ref GPIO_PIN_state
 */
typedef enum
{
    GPIO_PIN_RESET = 0U,
    GPIO_PIN_SET = 1U
} mcal_gpio_pin_state_t;

/**
 * @brief GPIO pin lock return state.
 * @ref GPIO_RETURN_LOCK
 */
typedef enum
{
    GPIO_RETURN_LOCK_ERROR = 0U,
    GPIO_RETURN_LOCK_Enabled = 1U
} mcal_gpio_lock_return_t;


/* ================================================================ */
/* ============== APIs Supported by "MCAL GPIO DRIVER" ============ */
/* ================================================================ */

/**
 * @brief Initialize one GPIO pin with mode and speed.
 * @param gpio_port GPIO port base pointer.
 * @param pin GPIO pin number.
 * @param mode GPIO mode including pull type.
 * @param speed GPIO output speed.
 */
void MCAL_GPIO_Init(GPIO_RegDef_t *gpio_port,
                    mcal_gpio_pin_t pin,
                    mcal_gpio_mode_t mode,
                    mcal_gpio_speed_t speed);

/**
 * @brief Reset a full GPIO port registers through RCC reset.
 * @param gpio_port GPIO port base pointer.
 */
void MCAL_GPIO_DeInit(GPIO_RegDef_t *gpio_port);

/**
 * @brief Read one GPIO input pin state.
 * @param gpio_port GPIO port base pointer.
 * @param pin GPIO pin number.
 * @return Current pin logic state (GPIO_PIN_RESET or GPIO_PIN_SET).
 */
uint8 MCAL_GPIO_ReadPin(GPIO_RegDef_t *gpio_port, mcal_gpio_pin_t pin);

/**
 * @brief Write one GPIO output pin state.
 * @param gpio_port GPIO port base pointer.
 * @param pin GPIO pin number.
 * @param value Output state (GPIO_PIN_RESET or GPIO_PIN_SET).
 */
void MCAL_GPIO_WritePin(GPIO_RegDef_t *gpio_port,
                        mcal_gpio_pin_t pin,
                        mcal_gpio_pin_state_t value);

/**
 * @brief Toggle one GPIO output pin.
 * @param gpio_port GPIO port base pointer.
 * @param pin GPIO pin number.
 */
void MCAL_GPIO_TogglePin(GPIO_RegDef_t *gpio_port, mcal_gpio_pin_t pin);

/**
 * @brief Lock one GPIO pin configuration until reset.
 * @param gpio_port GPIO port base pointer.
 * @param pin GPIO pin number.
 * @return Lock state result.
 */
mcal_gpio_lock_return_t MCAL_GPIO_LockPin(GPIO_RegDef_t *gpio_port, mcal_gpio_pin_t pin);

/**
 * @brief Read full GPIO port input data register lower 16 bits.
 * @param gpio_port GPIO port base pointer.
 * @return Current 16-bit port value.
 */
uint16 MCAL_GPIO_ReadPort(GPIO_RegDef_t *gpio_port);

/**
 * @brief Write full GPIO output data register lower 16 bits.
 * @param gpio_port GPIO port base pointer.
 * @param value 16-bit value to write.
 */
void MCAL_GPIO_WritePort(GPIO_RegDef_t *gpio_port, uint16 value);

#endif

