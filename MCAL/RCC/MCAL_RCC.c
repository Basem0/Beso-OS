/*
 * MCAL_RCC.c
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "MCAL_RCC.h"
#include "UTILS.h"

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
static mcal_rcc_status_t MCAL_RCC_WaitForBitState(volatile vuint32 *reg,
                                                   uint8 bit_position,
                                                   mcal_rcc_clock_state_t expected_state);
static mcal_rcc_status_t MCAL_RCC_ValidateClockConfig(const mcal_rcc_clock_config_t *clock_config);

/* ================================================================ */
/* ================= Static Function Definitions ================== */
/* ================================================================ */
static mcal_rcc_status_t MCAL_RCC_WaitForBitState(volatile vuint32 *reg,
                                                   uint8 bit_position,
                                                   mcal_rcc_clock_state_t expected_state)
{
    uint32 timeout;

    if (reg == NULL)
    {
        return MCAL_RCC_STATUS_INVALID_PARAM;
    }

    timeout = MCAL_RCC_TIMEOUT_VALUE;

    while (timeout > 0U)
    {
        if ((uint8)GET_BIT((*reg), bit_position) == (uint8)expected_state)
        {
            return MCAL_RCC_STATUS_OK;
        }

        timeout--;
    }

    return MCAL_RCC_STATUS_TIMEOUT;
}

static mcal_rcc_status_t MCAL_RCC_ValidateClockConfig(const mcal_rcc_clock_config_t *clock_config)
{
    if (clock_config == NULL)
    {
        return MCAL_RCC_STATUS_INVALID_PARAM;
    }

    if (clock_config->pll_state == MCAL_RCC_CLOCK_ON)
    {
        if ((clock_config->pll_m < 2U) || (clock_config->pll_m > 63U))
        {
            return MCAL_RCC_STATUS_INVALID_PARAM;
        }

        if ((clock_config->pll_n < 50U) || (clock_config->pll_n > 432U))
        {
            return MCAL_RCC_STATUS_INVALID_PARAM;
        }

        if ((clock_config->pll_q < 2U) || (clock_config->pll_q > 15U))
        {
            return MCAL_RCC_STATUS_INVALID_PARAM;
        }
    }

    if ((clock_config->sysclk_source == MCAL_RCC_SYSCLK_PLL) &&
        (clock_config->pll_state != MCAL_RCC_CLOCK_ON))
    {
        return MCAL_RCC_STATUS_INVALID_PARAM;
    }

    if ((clock_config->sysclk_source == MCAL_RCC_SYSCLK_HSE) &&
        (clock_config->hse_state != MCAL_RCC_CLOCK_ON))
    {
        return MCAL_RCC_STATUS_INVALID_PARAM;
    }

    if ((clock_config->sysclk_source == MCAL_RCC_SYSCLK_HSI) &&
        (clock_config->hsi_state != MCAL_RCC_CLOCK_ON))
    {
        return MCAL_RCC_STATUS_INVALID_PARAM;
    }

    return MCAL_RCC_STATUS_OK;
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
mcal_rcc_status_t MCAL_RCC_GetDefaultClockConfig(mcal_rcc_clock_config_t *clock_config)
{
    if (clock_config == NULL)
    {
        return MCAL_RCC_STATUS_INVALID_PARAM;
    }

    clock_config->hsi_state = MCAL_RCC_CLOCK_ON;
    clock_config->hse_state = MCAL_RCC_CLOCK_OFF;
    clock_config->hse_mode = MCAL_RCC_HSE_CRYSTAL;

    clock_config->pll_state = MCAL_RCC_CLOCK_OFF;
    clock_config->pll_source = MCAL_RCC_PLL_SOURCE_HSI;
    clock_config->pll_m = 16U;
    clock_config->pll_n = 336U;
    clock_config->pll_p = MCAL_RCC_PLLP_DIV2;
    clock_config->pll_q = 7U;

    clock_config->sysclk_source = MCAL_RCC_SYSCLK_HSI;
    clock_config->ahb_prescaler = MCAL_RCC_AHB_DIV1;
    clock_config->apb1_prescaler = MCAL_RCC_APB_DIV1;
    clock_config->apb2_prescaler = MCAL_RCC_APB_DIV1;

    return MCAL_RCC_STATUS_OK;
}

mcal_rcc_status_t MCAL_RCC_InitClock(const mcal_rcc_clock_config_t *clock_config)
{
    mcal_rcc_status_t status;
    uint32 pllcfgr;
    uint32 cfgr;
    uint32 timeout;

    status = MCAL_RCC_ValidateClockConfig(clock_config);
    if (status != MCAL_RCC_STATUS_OK)
    {
        return status;
    }

    if (clock_config->hsi_state == MCAL_RCC_CLOCK_ON)
    {
        SET_BIT(RCC->CR, MCAL_RCC_CR_HSION_POS);
        status = MCAL_RCC_WaitForBitState(&RCC->CR, MCAL_RCC_CR_HSIRDY_POS, MCAL_RCC_CLOCK_ON);
        if (status != MCAL_RCC_STATUS_OK)
        {
            return status;
        }
    }
    else
    {
        CLR_BIT(RCC->CR, MCAL_RCC_CR_HSION_POS);
    }

    if (clock_config->hse_mode == MCAL_RCC_HSE_BYPASS)
    {
        SET_BIT(RCC->CR, MCAL_RCC_CR_HSEBYP_POS);
    }
    else
    {
        CLR_BIT(RCC->CR, MCAL_RCC_CR_HSEBYP_POS);
    }

    if (clock_config->hse_state == MCAL_RCC_CLOCK_ON)
    {
        SET_BIT(RCC->CR, MCAL_RCC_CR_HSEON_POS);
        status = MCAL_RCC_WaitForBitState(&RCC->CR, MCAL_RCC_CR_HSERDY_POS, MCAL_RCC_CLOCK_ON);
        if (status != MCAL_RCC_STATUS_OK)
        {
            return status;
        }
    }
    else
    {
        CLR_BIT(RCC->CR, MCAL_RCC_CR_HSEON_POS);
    }

    CLR_BIT(RCC->CR, MCAL_RCC_CR_PLLON_POS);
    status = MCAL_RCC_WaitForBitState(&RCC->CR, MCAL_RCC_CR_PLLRDY_POS, MCAL_RCC_CLOCK_OFF);
    if ((status != MCAL_RCC_STATUS_OK) && (status != MCAL_RCC_STATUS_TIMEOUT))
    {
        return status;
    }

    pllcfgr = RCC->PLLCFGR;
    pllcfgr &= ~((MCAL_RCC_PLLCFGR_PLLM_MASK << MCAL_RCC_PLLCFGR_PLLM_POS) |
                 (MCAL_RCC_PLLCFGR_PLLN_MASK << MCAL_RCC_PLLCFGR_PLLN_POS) |
                 (MCAL_RCC_PLLCFGR_PLLP_MASK << MCAL_RCC_PLLCFGR_PLLP_POS) |
                 (1UL << MCAL_RCC_PLLCFGR_PLLSRC_POS) |
                 (MCAL_RCC_PLLCFGR_PLLQ_MASK << MCAL_RCC_PLLCFGR_PLLQ_POS));

    pllcfgr |= (((uint32)clock_config->pll_m & MCAL_RCC_PLLCFGR_PLLM_MASK) << MCAL_RCC_PLLCFGR_PLLM_POS);
    pllcfgr |= (((uint32)clock_config->pll_n & MCAL_RCC_PLLCFGR_PLLN_MASK) << MCAL_RCC_PLLCFGR_PLLN_POS);
    pllcfgr |= (((uint32)clock_config->pll_p & MCAL_RCC_PLLCFGR_PLLP_MASK) << MCAL_RCC_PLLCFGR_PLLP_POS);
    pllcfgr |= (((uint32)clock_config->pll_q & MCAL_RCC_PLLCFGR_PLLQ_MASK) << MCAL_RCC_PLLCFGR_PLLQ_POS);

    if (clock_config->pll_source == MCAL_RCC_PLL_SOURCE_HSE)
    {
        SET_BIT(pllcfgr, MCAL_RCC_PLLCFGR_PLLSRC_POS);
    }

    RCC->PLLCFGR = pllcfgr;

    if (clock_config->pll_state == MCAL_RCC_CLOCK_ON)
    {
        SET_BIT(RCC->CR, MCAL_RCC_CR_PLLON_POS);
        status = MCAL_RCC_WaitForBitState(&RCC->CR, MCAL_RCC_CR_PLLRDY_POS, MCAL_RCC_CLOCK_ON);
        if (status != MCAL_RCC_STATUS_OK)
        {
            return status;
        }
    }

    MCAL_RCC_SetAHBPrescaler(clock_config->ahb_prescaler);
    MCAL_RCC_SetAPB1Prescaler(clock_config->apb1_prescaler);
    MCAL_RCC_SetAPB2Prescaler(clock_config->apb2_prescaler);

    cfgr = RCC->CFGR;
    cfgr &= ~(MCAL_RCC_CFGR_SW_MASK << MCAL_RCC_CFGR_SW_POS);
    cfgr |= (((uint32)clock_config->sysclk_source & MCAL_RCC_CFGR_SW_MASK) << MCAL_RCC_CFGR_SW_POS);
    RCC->CFGR = cfgr;

    timeout = MCAL_RCC_TIMEOUT_VALUE;
    while (timeout > 0U)
    {
        if ((((RCC->CFGR >> MCAL_RCC_CFGR_SWS_POS) & MCAL_RCC_CFGR_SWS_MASK) ==
             (uint32)clock_config->sysclk_source))
        {
            return MCAL_RCC_STATUS_OK;
        }

        timeout--;
    }

    return MCAL_RCC_STATUS_TIMEOUT;
}

void MCAL_RCC_SetAHBPrescaler(mcal_rcc_ahb_prescaler_t prescaler)
{
    RCC->CFGR &= ~(MCAL_RCC_CFGR_HPRE_MASK << MCAL_RCC_CFGR_HPRE_POS);
    RCC->CFGR |= ((uint32)prescaler << MCAL_RCC_CFGR_HPRE_POS);
}

void MCAL_RCC_SetAPB1Prescaler(mcal_rcc_apb_prescaler_t prescaler)
{
    RCC->CFGR &= ~(MCAL_RCC_CFGR_PPRE1_MASK << MCAL_RCC_CFGR_PPRE1_POS);
    RCC->CFGR |= ((uint32)prescaler << MCAL_RCC_CFGR_PPRE1_POS);
}

void MCAL_RCC_SetAPB2Prescaler(mcal_rcc_apb_prescaler_t prescaler)
{
    RCC->CFGR &= ~(MCAL_RCC_CFGR_PPRE2_MASK << MCAL_RCC_CFGR_PPRE2_POS);
    RCC->CFGR |= ((uint32)prescaler << MCAL_RCC_CFGR_PPRE2_POS);
}

mcal_rcc_sysclk_source_t MCAL_RCC_GetSystemClockSource(void)
{
    return (mcal_rcc_sysclk_source_t)((RCC->CFGR >> MCAL_RCC_CFGR_SWS_POS) & MCAL_RCC_CFGR_SWS_MASK);
}

void MCAL_RCC_EnableAHB1PeripheralClock(mcal_rcc_ahb1_peripheral_t peripheral)
{
    if ((uint8)peripheral < 32U)
    {
        SET_BIT(RCC->AHB1ENR, (uint8)peripheral);
    }
}

void MCAL_RCC_DisableAHB1PeripheralClock(mcal_rcc_ahb1_peripheral_t peripheral)
{
    if ((uint8)peripheral < 32U)
    {
        CLR_BIT(RCC->AHB1ENR, (uint8)peripheral);
    }
}

void MCAL_RCC_EnableAHB2PeripheralClock(mcal_rcc_ahb2_peripheral_t peripheral)
{
    if ((uint8)peripheral < 32U)
    {
        SET_BIT(RCC->AHB2ENR, (uint8)peripheral);
    }
}

void MCAL_RCC_DisableAHB2PeripheralClock(mcal_rcc_ahb2_peripheral_t peripheral)
{
    if ((uint8)peripheral < 32U)
    {
        CLR_BIT(RCC->AHB2ENR, (uint8)peripheral);
    }
}

void MCAL_RCC_EnableAPB1PeripheralClock(mcal_rcc_apb1_peripheral_t peripheral)
{
    if ((uint8)peripheral < 32U)
    {
        SET_BIT(RCC->APB1ENR, (uint8)peripheral);
    }
}

void MCAL_RCC_DisableAPB1PeripheralClock(mcal_rcc_apb1_peripheral_t peripheral)
{
    if ((uint8)peripheral < 32U)
    {
        CLR_BIT(RCC->APB1ENR, (uint8)peripheral);
    }
}

void MCAL_RCC_EnableAPB2PeripheralClock(mcal_rcc_apb2_peripheral_t peripheral)
{
    if ((uint8)peripheral < 32U)
    {
        SET_BIT(RCC->APB2ENR, (uint8)peripheral);
    }
}

void MCAL_RCC_DisableAPB2PeripheralClock(mcal_rcc_apb2_peripheral_t peripheral)
{
    if ((uint8)peripheral < 32U)
    {
        CLR_BIT(RCC->APB2ENR, (uint8)peripheral);
    }
}

void MCAL_RCC_EnableGPIOClock(mcal_rcc_gpio_port_t gpio_port)
{
    if (gpio_port <= MCAL_RCC_GPIO_PORT_C)
    {
        MCAL_RCC_EnableAHB1PeripheralClock((mcal_rcc_ahb1_peripheral_t)gpio_port);
        (void)RCC->AHB1ENR;
    }
}

void MCAL_RCC_EnableGPIOAClock(void)
{
    MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_A);
}
