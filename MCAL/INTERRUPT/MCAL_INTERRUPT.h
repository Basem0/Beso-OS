/*
 * MCAL_Interrupt.h
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

#ifndef MCAL_INTERRUPT_H
#define MCAL_INTERRUPT_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "PLATFORM_TYPES.h"
#include "STM32F401RC.h"

/* ================================================================ */
/* ====================== Interrupt Macros ======================== */
/* ================================================================ */
#define MCAL_NVIC_ISER_BASE                 ((volatile vuint32 *)0xE000E100UL)
#define MCAL_NVIC_ICER_BASE                 ((volatile vuint32 *)0xE000E180UL)
#define MCAL_NVIC_ISPR_BASE                 ((volatile vuint32 *)0xE000E200UL)
#define MCAL_NVIC_ICPR_BASE                 ((volatile vuint32 *)0xE000E280UL)
#define MCAL_NVIC_IABR_BASE                 ((volatile vuint32 *)0xE000E300UL)
#define MCAL_NVIC_IPR_BASE                  ((volatile vuint8  *)0xE000E400UL)
#define MCAL_SCB_AIRCR                      (*(volatile vuint32 *)0xE000ED0CUL)

#define MCAL_SCB_AIRCR_VECTKEY              (0x5FAUL << 16U)
#define MCAL_SCB_AIRCR_PRIGROUP_POS         (8U)
#define MCAL_SCB_AIRCR_PRIGROUP_MASK        (0x7UL)

#define MCAL_NVIC_MAX_IRQS                  (96U)
#define MCAL_NVIC_PRIORITY_BITS             (4U)
#define MCAL_EXTI_LINES_COUNT               (23U)

/* ================================================================ */
/* =============== Macros Configuration References ================ */
/* ================================================================ */
typedef enum
{
    MCAL_INTERRUPT_STATUS_OK = 0U,
    MCAL_INTERRUPT_STATUS_ERROR,
    MCAL_INTERRUPT_STATUS_INVALID_PARAM
} mcal_interrupt_status_t;

typedef enum
{
    MCAL_IRQ_WWDG = 0U,
    MCAL_IRQ_PVD,
    MCAL_IRQ_TAMP_STAMP,
    MCAL_IRQ_RTC_WKUP,
    MCAL_IRQ_FLASH,
    MCAL_IRQ_RCC,
    MCAL_IRQ_EXTI0,
    MCAL_IRQ_EXTI1,
    MCAL_IRQ_EXTI2,
    MCAL_IRQ_EXTI3,
    MCAL_IRQ_EXTI4,
    MCAL_IRQ_DMA1_STREAM0,
    MCAL_IRQ_DMA1_STREAM1,
    MCAL_IRQ_DMA1_STREAM2,
    MCAL_IRQ_DMA1_STREAM3,
    MCAL_IRQ_DMA1_STREAM4,
    MCAL_IRQ_DMA1_STREAM5,
    MCAL_IRQ_DMA1_STREAM6,
    MCAL_IRQ_ADC = 18U,
    MCAL_IRQ_EXTI9_5 = 23U,
    MCAL_IRQ_TIM1_BRK_TIM9,
    MCAL_IRQ_TIM1_UP_TIM10,
    MCAL_IRQ_TIM1_TRG_COM_TIM11,
    MCAL_IRQ_TIM1_CC,
    MCAL_IRQ_TIM2,
    MCAL_IRQ_TIM3,
    MCAL_IRQ_TIM4,
    MCAL_IRQ_I2C1_EV,
    MCAL_IRQ_I2C1_ER,
    MCAL_IRQ_I2C2_EV,
    MCAL_IRQ_I2C2_ER,
    MCAL_IRQ_SPI1,
    MCAL_IRQ_SPI2,
    MCAL_IRQ_USART1,
    MCAL_IRQ_USART2,
    MCAL_IRQ_EXTI15_10 = 40U,
    MCAL_IRQ_RTC_ALARM,
    MCAL_IRQ_OTG_FS_WKUP,
    MCAL_IRQ_DMA1_STREAM7 = 47U,
    MCAL_IRQ_SDIO = 49U,
    MCAL_IRQ_TIM5,
    MCAL_IRQ_SPI3,
    MCAL_IRQ_DMA2_STREAM0 = 56U,
    MCAL_IRQ_DMA2_STREAM1,
    MCAL_IRQ_DMA2_STREAM2,
    MCAL_IRQ_DMA2_STREAM3,
    MCAL_IRQ_DMA2_STREAM4,
    MCAL_IRQ_OTG_FS = 67U,
    MCAL_IRQ_DMA2_STREAM5,
    MCAL_IRQ_DMA2_STREAM6,
    MCAL_IRQ_DMA2_STREAM7,
    MCAL_IRQ_USART6,
    MCAL_IRQ_I2C3_EV,
    MCAL_IRQ_I2C3_ER,
    MCAL_IRQ_SPI4 = 84U
} mcal_irqn_t;

typedef enum
{
    MCAL_NVIC_PRIORITY_GROUP_4_SUB_0 = 3U,
    MCAL_NVIC_PRIORITY_GROUP_3_SUB_1 = 4U,
    MCAL_NVIC_PRIORITY_GROUP_2_SUB_2 = 5U,
    MCAL_NVIC_PRIORITY_GROUP_1_SUB_3 = 6U,
    MCAL_NVIC_PRIORITY_GROUP_0_SUB_4 = 7U
} mcal_interrupt_priority_group_t;

typedef enum
{
    MCAL_EXTI_LINE_0 = 0U,
    MCAL_EXTI_LINE_1,
    MCAL_EXTI_LINE_2,
    MCAL_EXTI_LINE_3,
    MCAL_EXTI_LINE_4,
    MCAL_EXTI_LINE_5,
    MCAL_EXTI_LINE_6,
    MCAL_EXTI_LINE_7,
    MCAL_EXTI_LINE_8,
    MCAL_EXTI_LINE_9,
    MCAL_EXTI_LINE_10,
    MCAL_EXTI_LINE_11,
    MCAL_EXTI_LINE_12,
    MCAL_EXTI_LINE_13,
    MCAL_EXTI_LINE_14,
    MCAL_EXTI_LINE_15,
    MCAL_EXTI_LINE_16,
    MCAL_EXTI_LINE_17,
    MCAL_EXTI_LINE_18,
    MCAL_EXTI_LINE_19,
    MCAL_EXTI_LINE_20,
    MCAL_EXTI_LINE_21,
    MCAL_EXTI_LINE_22
} mcal_exti_line_t;

typedef enum
{
    MCAL_EXTI_PORT_A = 0U,
    MCAL_EXTI_PORT_B = 1U,
    MCAL_EXTI_PORT_C = 2U,
    MCAL_EXTI_PORT_H = 7U
} mcal_exti_port_t;

typedef enum
{
    MCAL_EXTI_MODE_INTERRUPT = 0U,
    MCAL_EXTI_MODE_EVENT,
    MCAL_EXTI_MODE_INTERRUPT_EVENT
} mcal_exti_mode_t;

typedef enum
{
    MCAL_EXTI_TRIGGER_RISING = 0U,
    MCAL_EXTI_TRIGGER_FALLING,
    MCAL_EXTI_TRIGGER_BOTH
} mcal_exti_trigger_t;

typedef void (*mcal_exti_callback_t)(void);

typedef struct
{
    mcal_exti_line_t line;
    mcal_exti_port_t port;
    mcal_exti_mode_t mode;
    mcal_exti_trigger_t trigger;
    mcal_exti_callback_t callback;
} mcal_exti_config_t;

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
void MCAL_Interrupt_EnableIRQ(mcal_irqn_t irqn);
void MCAL_Interrupt_DisableIRQ(mcal_irqn_t irqn);
void MCAL_Interrupt_SetPendingIRQ(mcal_irqn_t irqn);
void MCAL_Interrupt_ClearPendingIRQ(mcal_irqn_t irqn);
uint8 MCAL_Interrupt_GetActiveIRQ(mcal_irqn_t irqn);

void MCAL_Interrupt_SetPriorityGrouping(mcal_interrupt_priority_group_t priority_group);
void MCAL_Interrupt_SetPriority(mcal_irqn_t irqn,
                                uint8 preempt_priority,
                                uint8 sub_priority,
                                mcal_interrupt_priority_group_t priority_group);

void MCAL_Interrupt_EnableGlobal(void);
void MCAL_Interrupt_DisableGlobal(void);

mcal_interrupt_status_t MCAL_EXTI_Init(const mcal_exti_config_t *exti_config);
mcal_interrupt_status_t MCAL_EXTI_Update(const mcal_exti_config_t *exti_config);
void MCAL_EXTI_DeInit(mcal_exti_line_t line);
void MCAL_EXTI_GenerateSWInterrupt(mcal_exti_line_t line);
void MCAL_EXTI_ClearPendingFlag(mcal_exti_line_t line);
uint8 MCAL_EXTI_GetPendingFlag(mcal_exti_line_t line);
mcal_interrupt_status_t MCAL_EXTI_SetCallback(mcal_exti_line_t line, mcal_exti_callback_t callback);
void MCAL_EXTI_IRQHandler(mcal_exti_line_t line);

#endif
