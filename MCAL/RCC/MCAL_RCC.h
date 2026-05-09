/*
 * MCAL_RCC.h
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

#ifndef MCAL_RCC_H
#define MCAL_RCC_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "PLATFORM_TYPES.h"
#include "STM32F401RC.h"

/* ================================================================ */
/* ========================== RCC Macros ========================== */
/* ================================================================ */
#define MCAL_RCC_TIMEOUT_VALUE          (1000000UL)

#define MCAL_RCC_CR_HSION_POS           (0U)
#define MCAL_RCC_CR_HSIRDY_POS          (1U)
#define MCAL_RCC_CR_HSEON_POS           (16U)
#define MCAL_RCC_CR_HSERDY_POS          (17U)
#define MCAL_RCC_CR_HSEBYP_POS          (18U)
#define MCAL_RCC_CR_PLLON_POS           (24U)
#define MCAL_RCC_CR_PLLRDY_POS          (25U)

#define MCAL_RCC_CFGR_SW_POS            (0U)
#define MCAL_RCC_CFGR_SW_MASK           (0x3UL)
#define MCAL_RCC_CFGR_SWS_POS           (2U)
#define MCAL_RCC_CFGR_SWS_MASK          (0x3UL)
#define MCAL_RCC_CFGR_HPRE_POS          (4U)
#define MCAL_RCC_CFGR_HPRE_MASK         (0xFUL)
#define MCAL_RCC_CFGR_PPRE1_POS         (10U)
#define MCAL_RCC_CFGR_PPRE1_MASK        (0x7UL)
#define MCAL_RCC_CFGR_PPRE2_POS         (13U)
#define MCAL_RCC_CFGR_PPRE2_MASK        (0x7UL)

#define MCAL_RCC_PLLCFGR_PLLM_POS       (0U)
#define MCAL_RCC_PLLCFGR_PLLM_MASK      (0x3FUL)
#define MCAL_RCC_PLLCFGR_PLLN_POS       (6U)
#define MCAL_RCC_PLLCFGR_PLLN_MASK      (0x1FFUL)
#define MCAL_RCC_PLLCFGR_PLLP_POS       (16U)
#define MCAL_RCC_PLLCFGR_PLLP_MASK      (0x3UL)
#define MCAL_RCC_PLLCFGR_PLLSRC_POS     (22U)
#define MCAL_RCC_PLLCFGR_PLLQ_POS       (24U)
#define MCAL_RCC_PLLCFGR_PLLQ_MASK      (0xFUL)

/* ================================================================ */
/* =============== Macros Configuration References ================ */
/* ================================================================ */
typedef enum
{
	MCAL_RCC_STATUS_OK = 0U,
	MCAL_RCC_STATUS_ERROR,
	MCAL_RCC_STATUS_TIMEOUT,
	MCAL_RCC_STATUS_INVALID_PARAM
} mcal_rcc_status_t;

typedef enum
{
	MCAL_RCC_CLOCK_OFF = 0U,
	MCAL_RCC_CLOCK_ON = 1U
} mcal_rcc_clock_state_t;

typedef enum
{
	MCAL_RCC_HSE_CRYSTAL = 0U,
	MCAL_RCC_HSE_BYPASS = 1U
} mcal_rcc_hse_mode_t;

typedef enum
{
	MCAL_RCC_SYSCLK_HSI = 0U,
	MCAL_RCC_SYSCLK_HSE = 1U,
	MCAL_RCC_SYSCLK_PLL = 2U
} mcal_rcc_sysclk_source_t;

typedef enum
{
	MCAL_RCC_PLL_SOURCE_HSI = 0U,
	MCAL_RCC_PLL_SOURCE_HSE = 1U
} mcal_rcc_pll_source_t;

typedef enum
{
	MCAL_RCC_PLLP_DIV2 = 0U,
	MCAL_RCC_PLLP_DIV4,
	MCAL_RCC_PLLP_DIV6,
	MCAL_RCC_PLLP_DIV8
} mcal_rcc_pll_p_div_t;

typedef enum
{
	MCAL_RCC_AHB_DIV1 = 0x0U,
	MCAL_RCC_AHB_DIV2 = 0x8U,
	MCAL_RCC_AHB_DIV4 = 0x9U,
	MCAL_RCC_AHB_DIV8 = 0xAU,
	MCAL_RCC_AHB_DIV16 = 0xBU,
	MCAL_RCC_AHB_DIV64 = 0xCU,
	MCAL_RCC_AHB_DIV128 = 0xDU,
	MCAL_RCC_AHB_DIV256 = 0xEU,
	MCAL_RCC_AHB_DIV512 = 0xFU
} mcal_rcc_ahb_prescaler_t;

typedef enum
{
	MCAL_RCC_APB_DIV1 = 0x0U,
	MCAL_RCC_APB_DIV2 = 0x4U,
	MCAL_RCC_APB_DIV4 = 0x5U,
	MCAL_RCC_APB_DIV8 = 0x6U,
	MCAL_RCC_APB_DIV16 = 0x7U
} mcal_rcc_apb_prescaler_t;

typedef enum
{
	MCAL_RCC_GPIO_PORT_A = 0U,
	MCAL_RCC_GPIO_PORT_B = 1U,
	MCAL_RCC_GPIO_PORT_C = 2U
} mcal_rcc_gpio_port_t;

typedef enum
{
	MCAL_RCC_AHB1_GPIOA = 0U,
	MCAL_RCC_AHB1_GPIOB = 1U,
	MCAL_RCC_AHB1_GPIOC = 2U,
	MCAL_RCC_AHB1_GPIOH = 7U,
	MCAL_RCC_AHB1_CRC = 12U,
	MCAL_RCC_AHB1_DMA1 = 21U,
	MCAL_RCC_AHB1_DMA2 = 22U
} mcal_rcc_ahb1_peripheral_t;

typedef enum
{
	MCAL_RCC_AHB2_OTGFS = 7U
} mcal_rcc_ahb2_peripheral_t;

typedef enum
{
	MCAL_RCC_APB1_TIM2 = 0U,
	MCAL_RCC_APB1_TIM3 = 1U,
	MCAL_RCC_APB1_TIM4 = 2U,
	MCAL_RCC_APB1_TIM5 = 3U,
	MCAL_RCC_APB1_WWDG = 11U,
	MCAL_RCC_APB1_SPI2 = 14U,
	MCAL_RCC_APB1_SPI3 = 15U,
	MCAL_RCC_APB1_USART2 = 17U,
	MCAL_RCC_APB1_UART4 = 19U,
	MCAL_RCC_APB1_UART5 = 20U,
	MCAL_RCC_APB1_I2C1 = 21U,
	MCAL_RCC_APB1_I2C2 = 22U,
	MCAL_RCC_APB1_I2C3 = 23U,
	MCAL_RCC_APB1_PWR = 28U
} mcal_rcc_apb1_peripheral_t;

typedef enum
{
	MCAL_RCC_APB2_TIM1 = 0U,
	MCAL_RCC_APB2_USART1 = 4U,
	MCAL_RCC_APB2_USART6 = 5U,
	MCAL_RCC_APB2_ADC1 = 8U,
	MCAL_RCC_APB2_SDIO = 11U,
	MCAL_RCC_APB2_SPI1 = 12U,
	MCAL_RCC_APB2_SPI4 = 13U,
	MCAL_RCC_APB2_SYSCFG = 14U,
	MCAL_RCC_APB2_TIM9 = 16U,
	MCAL_RCC_APB2_TIM10 = 17U,
	MCAL_RCC_APB2_TIM11 = 18U
} mcal_rcc_apb2_peripheral_t;

typedef struct
{
	mcal_rcc_clock_state_t hsi_state;
	mcal_rcc_clock_state_t hse_state;
	mcal_rcc_hse_mode_t hse_mode;

	mcal_rcc_clock_state_t pll_state;
	mcal_rcc_pll_source_t pll_source;
	uint8 pll_m;
	uint16 pll_n;
	mcal_rcc_pll_p_div_t pll_p;
	uint8 pll_q;

	mcal_rcc_sysclk_source_t sysclk_source;
	mcal_rcc_ahb_prescaler_t ahb_prescaler;
	mcal_rcc_apb_prescaler_t apb1_prescaler;
	mcal_rcc_apb_prescaler_t apb2_prescaler;
} mcal_rcc_clock_config_t;

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
mcal_rcc_status_t MCAL_RCC_GetDefaultClockConfig(mcal_rcc_clock_config_t *clock_config);
mcal_rcc_status_t MCAL_RCC_InitClock(const mcal_rcc_clock_config_t *clock_config);

void MCAL_RCC_SetAHBPrescaler(mcal_rcc_ahb_prescaler_t prescaler);
void MCAL_RCC_SetAPB1Prescaler(mcal_rcc_apb_prescaler_t prescaler);
void MCAL_RCC_SetAPB2Prescaler(mcal_rcc_apb_prescaler_t prescaler);
mcal_rcc_sysclk_source_t MCAL_RCC_GetSystemClockSource(void);

void MCAL_RCC_EnableAHB1PeripheralClock(mcal_rcc_ahb1_peripheral_t peripheral);
void MCAL_RCC_DisableAHB1PeripheralClock(mcal_rcc_ahb1_peripheral_t peripheral);
void MCAL_RCC_EnableAHB2PeripheralClock(mcal_rcc_ahb2_peripheral_t peripheral);
void MCAL_RCC_DisableAHB2PeripheralClock(mcal_rcc_ahb2_peripheral_t peripheral);
void MCAL_RCC_EnableAPB1PeripheralClock(mcal_rcc_apb1_peripheral_t peripheral);
void MCAL_RCC_DisableAPB1PeripheralClock(mcal_rcc_apb1_peripheral_t peripheral);
void MCAL_RCC_EnableAPB2PeripheralClock(mcal_rcc_apb2_peripheral_t peripheral);
void MCAL_RCC_DisableAPB2PeripheralClock(mcal_rcc_apb2_peripheral_t peripheral);

void MCAL_RCC_EnableGPIOClock(mcal_rcc_gpio_port_t gpio_port);
void MCAL_RCC_EnableGPIOAClock(void);

#endif
