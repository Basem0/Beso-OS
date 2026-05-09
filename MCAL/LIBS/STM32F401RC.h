/*
 * STM32F401RC.h
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

#ifndef STM32F401RC_H
#define STM32F401RC_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */

#include "PLATFORM_TYPES.h"

/* ================================================================ */
/* ====================== Base Address Macros ===================== */
/* ================================================================ */
#define FLASH_BASE_ADDR         (0x08000000UL) /* FLASH base address in the alias region */
#define SRAM1_BASE_ADDR         (0x20000000UL) /* SRAM base address in the alias region */
#define PERIPH_BASE_ADDR        (0x40000000UL) /* Peripheral base address in the alias region */

#define AHB1PERIPH_BASE_ADDR    (PERIPH_BASE_ADDR + 0x00020000UL)
#define APB1PERIPH_BASE_ADDR    (PERIPH_BASE_ADDR + 0x00000000UL)
#define APB2PERIPH_BASE_ADDR    (PERIPH_BASE_ADDR + 0x00010000UL)

#define GPIOA_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0000UL)
#define GPIOB_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0400UL)
#define GPIOC_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0800UL)
#define RCC_BASE_ADDR           (AHB1PERIPH_BASE_ADDR + 0x3800UL)

#define SPI2_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x3800UL)
#define SPI3_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x3C00UL)
#define USART2_BASE_ADDR        (APB1PERIPH_BASE_ADDR + 0x4400UL)
#define UART4_BASE_ADDR         (APB1PERIPH_BASE_ADDR + 0x4C00UL)
#define UART5_BASE_ADDR         (APB1PERIPH_BASE_ADDR + 0x5000UL)

#define SPI1_BASE_ADDR          (APB2PERIPH_BASE_ADDR + 0x3000UL)
#define SPI4_BASE_ADDR          (APB2PERIPH_BASE_ADDR + 0x3400UL)
#define SYSCFG_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x3800UL)
#define EXTI_BASE_ADDR          (APB2PERIPH_BASE_ADDR + 0x3C00UL)
#define USART1_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x1000UL)
#define USART6_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x1400UL)

/* ================================================================ */
/* ========================= GPIO Register ======================== */
/* ================================================================ */

typedef struct
{
	vuint32 MODER;
	vuint32 OTYPER;
	vuint32 OSPEEDR;
	vuint32 PUPDR;
	vuint32 IDR;
	vuint32 ODR;
	vuint32 BSRR;
	vuint32 LCKR;
	vuint32 AFRL;
	vuint32 AFRH;
} GPIO_RegDef_t;

/* ================================================================ */
/* ========================== RCC Register ======================== */
/* ================================================================ */

typedef struct
{
	vuint32 CR;
	vuint32 PLLCFGR;
	vuint32 CFGR;
	vuint32 CIR;
	vuint32 AHB1RSTR;
	vuint32 AHB2RSTR;
	vuint32 RESERVED0[2];
	vuint32 APB1RSTR;
	vuint32 APB2RSTR;
	vuint32 RESERVED1[2];
	vuint32 AHB1ENR;
	vuint32 AHB2ENR;
	vuint32 RESERVED2[2];
	vuint32 APB1ENR;
	vuint32 APB2ENR;
	vuint32 RESERVED3[2];
	vuint32 AHB1LPENR;
	vuint32 AHB2LPENR;
	vuint32 RESERVED4[2];
	vuint32 APB1LPENR;
	vuint32 APB2LPENR;
	vuint32 RESERVED5[2];
	vuint32 BDCR;
	vuint32 CSR;
	vuint32 RESERVED6[2];
	vuint32 SSCGR;
	vuint32 PLLI2SCFGR;
	vuint32 RESERVED7;
	vuint32 DCKCFGR;
} RCC_RegDef_t;

/* ================================================================ */
/* ========================= USART Register ======================= */
/* ================================================================ */

typedef struct
{
	vuint32 SR;
	vuint32 DR;
	vuint32 BRR;
	vuint32 CR1;
	vuint32 CR2;
	vuint32 CR3;
	vuint32 GTPR;
} USART_RegDef_t;

/* ================================================================ */
/* ========================== SPI Register ======================== */
/* ================================================================ */

typedef struct
{
	vuint32 CR1;
	vuint32 CR2;
	vuint32 SR;
	vuint32 DR;
	vuint32 CRCPR;
	vuint32 RXCRCR;
	vuint32 TXCRCR;
	vuint32 I2SCFGR;
	vuint32 I2SPR;
} SPI_RegDef_t;

/* ================================================================ */
/* ======================== SYSCFG Register ======================= */
/* ================================================================ */

typedef struct
{
	vuint32 MEMRMP;
	vuint32 PMC;
	vuint32 EXTICR1;
	vuint32 EXTICR2;
	vuint32 EXTICR3;
	vuint32 EXTICR4;
	vuint32 RESERVED[2];
	vuint32 CMPCR;
} SYSCFG_RegDef_t;

/* ================================================================ */
/* ========================= EXTI Register ======================== */
/* ================================================================ */

typedef struct
{
	vuint32 IMR;
	vuint32 EMR;
	vuint32 RTSR;
	vuint32 FTSR;
	vuint32 SWIER;
	vuint32 PR;
} EXTI_RegDef_t;

/* ================================================================ */
/* ====================== Peripheral Instances ==================== */
/* ================================================================ */
#define GPIOA ((GPIO_RegDef_t *)GPIOA_BASE_ADDR)
#define GPIOB ((GPIO_RegDef_t *)GPIOB_BASE_ADDR)
#define GPIOC ((GPIO_RegDef_t *)GPIOC_BASE_ADDR)
#define RCC   ((RCC_RegDef_t *)RCC_BASE_ADDR)

#define USART1 ((USART_RegDef_t *)USART1_BASE_ADDR)
#define USART2 ((USART_RegDef_t *)USART2_BASE_ADDR)
#define USART6 ((USART_RegDef_t *)USART6_BASE_ADDR)
#define UART4  ((USART_RegDef_t *)UART4_BASE_ADDR)
#define UART5  ((USART_RegDef_t *)UART5_BASE_ADDR)

#define SPI1 ((SPI_RegDef_t *)SPI1_BASE_ADDR)
#define SPI2 ((SPI_RegDef_t *)SPI2_BASE_ADDR)
#define SPI3 ((SPI_RegDef_t *)SPI3_BASE_ADDR)
#define SPI4 ((SPI_RegDef_t *)SPI4_BASE_ADDR)

#define SYSCFG ((SYSCFG_RegDef_t *)SYSCFG_BASE_ADDR)
#define EXTI   ((EXTI_RegDef_t *)EXTI_BASE_ADDR)

#endif
