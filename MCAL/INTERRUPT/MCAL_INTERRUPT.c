/*
 * MCAL_Interrupt.c
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "MCAL_Interrupt.h"

#include "MCAL_RCC.h"
#include "UTILS.h"

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
static mcal_exti_callback_t g_mcal_exti_callbacks[MCAL_EXTI_LINES_COUNT] =
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static uint8 MCAL_Interrupt_IsValidIRQ(mcal_irqn_t irqn);
static uint8 MCAL_EXTI_IsValidLine(mcal_exti_line_t line);
static volatile vuint32 *MCAL_EXTI_GetConfigRegister(mcal_exti_line_t line);
static void MCAL_EXTI_ConfigurePortMapping(mcal_exti_line_t line, mcal_exti_port_t port);

/* ================================================================ */
/* ================= Static Function Definitions ================== */
/* ================================================================ */
static uint8 MCAL_Interrupt_IsValidIRQ(mcal_irqn_t irqn)
{
    if ((uint32)irqn < MCAL_NVIC_MAX_IRQS)
    {
        return 1U;
    }

    return 0U;
}

static uint8 MCAL_EXTI_IsValidLine(mcal_exti_line_t line)
{
    if ((uint32)line < MCAL_EXTI_LINES_COUNT)
    {
        return 1U;
    }

    return 0U;
}

static volatile vuint32 *MCAL_EXTI_GetConfigRegister(mcal_exti_line_t line)
{
    uint8 index;

    index = (uint8)line / 4U;

    if (index == 0U)
    {
        return &SYSCFG->EXTICR1;
    }

    if (index == 1U)
    {
        return &SYSCFG->EXTICR2;
    }

    if (index == 2U)
    {
        return &SYSCFG->EXTICR3;
    }

    return &SYSCFG->EXTICR4;
}

static void MCAL_EXTI_ConfigurePortMapping(mcal_exti_line_t line, mcal_exti_port_t port)
{
    uint8 shift;
    volatile vuint32 *exticr;

    if (line > MCAL_EXTI_LINE_15)
    {
        return;
    }

    exticr = MCAL_EXTI_GetConfigRegister(line);
    shift = ((uint8)line % 4U) * 4U;

    (*exticr) &= ~(0xFUL << shift);
    (*exticr) |= ((uint32)port << shift);
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
void MCAL_Interrupt_EnableIRQ(mcal_irqn_t irqn)
{
    uint32 reg_index;
    uint32 bit_pos;

    if (MCAL_Interrupt_IsValidIRQ(irqn) == 0U)
    {
        return;
    }

    reg_index = ((uint32)irqn) / 32U;
    bit_pos = ((uint32)irqn) % 32U;

    MCAL_NVIC_ISER_BASE[reg_index] = (1UL << bit_pos);
}

void MCAL_Interrupt_DisableIRQ(mcal_irqn_t irqn)
{
    uint32 reg_index;
    uint32 bit_pos;

    if (MCAL_Interrupt_IsValidIRQ(irqn) == 0U)
    {
        return;
    }

    reg_index = ((uint32)irqn) / 32U;
    bit_pos = ((uint32)irqn) % 32U;

    MCAL_NVIC_ICER_BASE[reg_index] = (1UL << bit_pos);
}

void MCAL_Interrupt_SetPendingIRQ(mcal_irqn_t irqn)
{
    uint32 reg_index;
    uint32 bit_pos;

    if (MCAL_Interrupt_IsValidIRQ(irqn) == 0U)
    {
        return;
    }

    reg_index = ((uint32)irqn) / 32U;
    bit_pos = ((uint32)irqn) % 32U;

    MCAL_NVIC_ISPR_BASE[reg_index] = (1UL << bit_pos);
}

void MCAL_Interrupt_ClearPendingIRQ(mcal_irqn_t irqn)
{
    uint32 reg_index;
    uint32 bit_pos;

    if (MCAL_Interrupt_IsValidIRQ(irqn) == 0U)
    {
        return;
    }

    reg_index = ((uint32)irqn) / 32U;
    bit_pos = ((uint32)irqn) % 32U;

    MCAL_NVIC_ICPR_BASE[reg_index] = (1UL << bit_pos);
}

uint8 MCAL_Interrupt_GetActiveIRQ(mcal_irqn_t irqn)
{
    uint32 reg_index;
    uint32 bit_pos;

    if (MCAL_Interrupt_IsValidIRQ(irqn) == 0U)
    {
        return 0U;
    }

    reg_index = ((uint32)irqn) / 32U;
    bit_pos = ((uint32)irqn) % 32U;

    return (uint8)((MCAL_NVIC_IABR_BASE[reg_index] >> bit_pos) & 0x1UL);
}

void MCAL_Interrupt_SetPriorityGrouping(mcal_interrupt_priority_group_t priority_group)
{
    uint32 aircr;

    aircr = MCAL_SCB_AIRCR;
    aircr &= ~(MCAL_SCB_AIRCR_PRIGROUP_MASK << MCAL_SCB_AIRCR_PRIGROUP_POS);
    aircr = MCAL_SCB_AIRCR_VECTKEY |
            aircr |
            (((uint32)priority_group & MCAL_SCB_AIRCR_PRIGROUP_MASK) << MCAL_SCB_AIRCR_PRIGROUP_POS);

    MCAL_SCB_AIRCR = aircr;
}

void MCAL_Interrupt_SetPriority(mcal_irqn_t irqn,
                                uint8 preempt_priority,
                                uint8 sub_priority,
                                mcal_interrupt_priority_group_t priority_group)
{
    uint8 preempt_bits;
    uint8 sub_bits;
    uint8 priority_value;
    uint8 preempt_mask;
    uint8 sub_mask;

    if (MCAL_Interrupt_IsValidIRQ(irqn) == 0U)
    {
        return;
    }

    switch (priority_group)
    {
        case MCAL_NVIC_PRIORITY_GROUP_4_SUB_0:
            preempt_bits = 4U;
            sub_bits = 0U;
            break;

        case MCAL_NVIC_PRIORITY_GROUP_3_SUB_1:
            preempt_bits = 3U;
            sub_bits = 1U;
            break;

        case MCAL_NVIC_PRIORITY_GROUP_2_SUB_2:
            preempt_bits = 2U;
            sub_bits = 2U;
            break;

        case MCAL_NVIC_PRIORITY_GROUP_1_SUB_3:
            preempt_bits = 1U;
            sub_bits = 3U;
            break;

        case MCAL_NVIC_PRIORITY_GROUP_0_SUB_4:
        default:
            preempt_bits = 0U;
            sub_bits = 4U;
            break;
    }

    preempt_mask = (preempt_bits == 0U) ? 0U : (uint8)((1U << preempt_bits) - 1U);
    sub_mask = (sub_bits == 0U) ? 0U : (uint8)((1U << sub_bits) - 1U);

    priority_value = (uint8)(((preempt_priority & preempt_mask) << sub_bits) |
                             (sub_priority & sub_mask));

    MCAL_NVIC_IPR_BASE[(uint32)irqn] = (uint8)(priority_value << (8U - MCAL_NVIC_PRIORITY_BITS));
}

void MCAL_Interrupt_EnableGlobal(void)
{
    __asm volatile ("cpsie i");
}

void MCAL_Interrupt_DisableGlobal(void)
{
    __asm volatile ("cpsid i");
}

mcal_interrupt_status_t MCAL_EXTI_Init(const mcal_exti_config_t *exti_config)
{
    if (exti_config == NULL)
    {
        return MCAL_INTERRUPT_STATUS_INVALID_PARAM;
    }

    if (MCAL_EXTI_IsValidLine(exti_config->line) == 0U)
    {
        return MCAL_INTERRUPT_STATUS_INVALID_PARAM;
    }

    if (exti_config->line <= MCAL_EXTI_LINE_15)
    {
        MCAL_RCC_EnableAPB2PeripheralClock(MCAL_RCC_APB2_SYSCFG);
        MCAL_EXTI_ConfigurePortMapping(exti_config->line, exti_config->port);
    }

    CLR_BIT(EXTI->IMR, (uint8)exti_config->line);
    CLR_BIT(EXTI->EMR, (uint8)exti_config->line);

    if ((exti_config->mode == MCAL_EXTI_MODE_INTERRUPT) ||
        (exti_config->mode == MCAL_EXTI_MODE_INTERRUPT_EVENT))
    {
        SET_BIT(EXTI->IMR, (uint8)exti_config->line);
    }

    if ((exti_config->mode == MCAL_EXTI_MODE_EVENT) ||
        (exti_config->mode == MCAL_EXTI_MODE_INTERRUPT_EVENT))
    {
        SET_BIT(EXTI->EMR, (uint8)exti_config->line);
    }

    CLR_BIT(EXTI->RTSR, (uint8)exti_config->line);
    CLR_BIT(EXTI->FTSR, (uint8)exti_config->line);

    if ((exti_config->trigger == MCAL_EXTI_TRIGGER_RISING) ||
        (exti_config->trigger == MCAL_EXTI_TRIGGER_BOTH))
    {
        SET_BIT(EXTI->RTSR, (uint8)exti_config->line);
    }

    if ((exti_config->trigger == MCAL_EXTI_TRIGGER_FALLING) ||
        (exti_config->trigger == MCAL_EXTI_TRIGGER_BOTH))
    {
        SET_BIT(EXTI->FTSR, (uint8)exti_config->line);
    }

    g_mcal_exti_callbacks[(uint8)exti_config->line] = exti_config->callback;

    return MCAL_INTERRUPT_STATUS_OK;
}

mcal_interrupt_status_t MCAL_EXTI_Update(const mcal_exti_config_t *exti_config)
{
    return MCAL_EXTI_Init(exti_config);
}

void MCAL_EXTI_DeInit(mcal_exti_line_t line)
{
    volatile vuint32 *exticr;
    uint8 shift;

    if (MCAL_EXTI_IsValidLine(line) == 0U)
    {
        return;
    }

    CLR_BIT(EXTI->IMR, (uint8)line);
    CLR_BIT(EXTI->EMR, (uint8)line);
    CLR_BIT(EXTI->RTSR, (uint8)line);
    CLR_BIT(EXTI->FTSR, (uint8)line);
    CLR_BIT(EXTI->SWIER, (uint8)line);
    SET_BIT(EXTI->PR, (uint8)line);

    if (line <= MCAL_EXTI_LINE_15)
    {
        exticr = MCAL_EXTI_GetConfigRegister(line);
        shift = ((uint8)line % 4U) * 4U;
        (*exticr) &= ~(0xFUL << shift);
    }

    g_mcal_exti_callbacks[(uint8)line] = NULL;
}

void MCAL_EXTI_GenerateSWInterrupt(mcal_exti_line_t line)
{
    if (MCAL_EXTI_IsValidLine(line) == 0U)
    {
        return;
    }

    SET_BIT(EXTI->SWIER, (uint8)line);
}

void MCAL_EXTI_ClearPendingFlag(mcal_exti_line_t line)
{
    if (MCAL_EXTI_IsValidLine(line) == 0U)
    {
        return;
    }

    SET_BIT(EXTI->PR, (uint8)line);
}

uint8 MCAL_EXTI_GetPendingFlag(mcal_exti_line_t line)
{
    if (MCAL_EXTI_IsValidLine(line) == 0U)
    {
        return 0U;
    }

    return (uint8)GET_BIT(EXTI->PR, (uint8)line);
}

mcal_interrupt_status_t MCAL_EXTI_SetCallback(mcal_exti_line_t line, mcal_exti_callback_t callback)
{
    if (MCAL_EXTI_IsValidLine(line) == 0U)
    {
        return MCAL_INTERRUPT_STATUS_INVALID_PARAM;
    }

    g_mcal_exti_callbacks[(uint8)line] = callback;

    return MCAL_INTERRUPT_STATUS_OK;
}

void MCAL_EXTI_IRQHandler(mcal_exti_line_t line)
{
    if (MCAL_EXTI_IsValidLine(line) == 0U)
    {
        return;
    }

    if ((MCAL_EXTI_GetPendingFlag(line) == 1U) &&
        (GET_BIT(EXTI->IMR, (uint8)line) == 1U))
    {
        MCAL_EXTI_ClearPendingFlag(line);

        if (g_mcal_exti_callbacks[(uint8)line] != NULL)
        {
            g_mcal_exti_callbacks[(uint8)line]();
        }
    }
}
