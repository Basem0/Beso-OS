/*
 * mcal_gpio.c
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "mcal_gpio.h"

#include "UTILS.h"

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
static uint8 MCAL_GPIO_GetPortIndex(GPIO_RegDef_t *gpio_port)
{
    if (gpio_port == GPIOA)
    {
        return 0U;
    }

    if (gpio_port == GPIOB)
    {
        return 1U;
    }

    if (gpio_port == GPIOC)
    {
        return 2U;
    }

    return MCAL_GPIO_INVALID_PORT_INDEX;
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
void MCAL_GPIO_Init(GPIO_RegDef_t *gpio_port,
                    mcal_gpio_pin_t pin,
                    mcal_gpio_mode_t mode,
                    mcal_gpio_speed_t speed)
{
    uint32 shift;
    uint32 mode_bits;
    uint32 otype_bit;
    uint32 pupd_bits;

    if ((gpio_port == NULL) || (pin > MCAL_GPIO_PIN_15))
    {
        return;
    }

    shift = (uint32)pin * 2U;
    mode_bits = (uint32)mode & MCAL_GPIO_MODE_MASK;
    otype_bit = ((uint32)mode & MCAL_GPIO_OTYPE_MASK) >> 2U;
    pupd_bits = ((uint32)mode & MCAL_GPIO_PUPD_MASK) >> 3U;

    gpio_port->MODER &= ~(0x3UL << shift);
    gpio_port->MODER |= (mode_bits << shift);

    if (otype_bit == 0U)
    {
        CLR_BIT(gpio_port->OTYPER, pin);
    }
    else
    {
        SET_BIT(gpio_port->OTYPER, pin);
    }

    gpio_port->PUPDR &= ~(0x3UL << shift);
    gpio_port->PUPDR |= (pupd_bits << shift);

    gpio_port->OSPEEDR &= ~(0x3UL << shift);
    gpio_port->OSPEEDR |= ((uint32)speed << shift);
}

void MCAL_GPIO_DeInit(GPIO_RegDef_t *gpio_port)
{
    uint8 port_index = MCAL_GPIO_GetPortIndex(gpio_port);

    if (port_index == MCAL_GPIO_INVALID_PORT_INDEX)
    {
        return;
    }

    SET_BIT(RCC->AHB1RSTR, port_index);
    CLR_BIT(RCC->AHB1RSTR, port_index);
}

uint8 MCAL_GPIO_ReadPin(GPIO_RegDef_t *gpio_port, mcal_gpio_pin_t pin)
{
    if ((gpio_port == NULL) || (pin > MCAL_GPIO_PIN_15))
    {
        return 0U;
    }

    return (uint8)GET_BIT(gpio_port->IDR, pin);
}

uint16 MCAL_GPIO_ReadPort(GPIO_RegDef_t *gpio_port)
{
    if (gpio_port == NULL)
    {
        return 0U;
    }

    return (uint16)(gpio_port->IDR & 0xFFFFUL);
}

void MCAL_GPIO_WritePin(GPIO_RegDef_t *gpio_port,
                        mcal_gpio_pin_t pin,
                        mcal_gpio_pin_state_t value)
{
    if ((gpio_port == NULL) || (pin > MCAL_GPIO_PIN_15))
    {
        return;
    }

    if (value == 0U)
    {
        CLR_BIT(gpio_port->ODR, pin);
    }
    else
    {
        SET_BIT(gpio_port->ODR, pin);
    }
}

void MCAL_GPIO_WritePort(GPIO_RegDef_t *gpio_port, uint16 value)
{
    if (gpio_port == NULL)
    {
        return;
    }

    gpio_port->ODR &= 0xFFFF0000UL;
    gpio_port->ODR |= (uint32)value;
}

void MCAL_GPIO_TogglePin(GPIO_RegDef_t *gpio_port, mcal_gpio_pin_t pin)
{
    if ((gpio_port == NULL) || (pin > MCAL_GPIO_PIN_15))
    {
        return;
    }

    TOGGLE_BIT(gpio_port->ODR, pin);
}

mcal_gpio_lock_return_t MCAL_GPIO_LockPin(GPIO_RegDef_t *gpio_port, mcal_gpio_pin_t pin)
{
    uint32 temp;

    if ((gpio_port == NULL) || (pin > MCAL_GPIO_PIN_15))
    {
        return GPIO_RETURN_LOCK_ERROR;
    }

    temp = (1UL << MCAL_GPIO_LOCK_KEY_BIT) | (1UL << pin);
    gpio_port->LCKR = temp;
    gpio_port->LCKR = (1UL << pin);
    gpio_port->LCKR = temp;

    temp = gpio_port->LCKR;
    temp = gpio_port->LCKR;

    if ((temp & (1UL << MCAL_GPIO_LOCK_KEY_BIT)) != 0U)
    {
        return GPIO_RETURN_LOCK_Enabled;
    }

    return GPIO_RETURN_LOCK_ERROR;
}
