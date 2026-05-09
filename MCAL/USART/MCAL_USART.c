/*
 * MCAL_USART.c
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "MCAL_USART.h"

#include "UTILS.h"

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
static mcal_usart_callback_t g_mcal_usart_callbacks[5] = {NULL, NULL, NULL, NULL, NULL};

static uint8 MCAL_USART_GetInstanceIndex(USART_RegDef_t *usart);
static mcal_usart_status_t MCAL_USART_WaitFlagState(USART_RegDef_t *usart,
                                                     mcal_usart_flag_t flag,
                                                     uint8 expected_state);

/* ================================================================ */
/* ================= Static Function Definitions ================== */
/* ================================================================ */
static uint8 MCAL_USART_GetInstanceIndex(USART_RegDef_t *usart)
{
    if (usart == USART1)
    {
        return 0U;
    }

    if (usart == USART2)
    {
        return 1U;
    }

    if (usart == USART6)
    {
        return 2U;
    }

    if (usart == UART4)
    {
        return 3U;
    }

    if (usart == UART5)
    {
        return 4U;
    }

    return MCAL_USART_INSTANCE_INDEX_INVALID;
}

static mcal_usart_status_t MCAL_USART_WaitFlagState(USART_RegDef_t *usart,
                                                     mcal_usart_flag_t flag,
                                                     uint8 expected_state)
{
    uint32 timeout;

    if (usart == NULL)
    {
        return MCAL_USART_STATUS_INVALID_PARAM;
    }

    timeout = MCAL_USART_TIMEOUT_VALUE;

    while (timeout > 0U)
    {
        if (MCAL_USART_GetFlagStatus(usart, flag) == expected_state)
        {
            return MCAL_USART_STATUS_OK;
        }

        timeout--;
    }

    return MCAL_USART_STATUS_TIMEOUT;
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
mcal_usart_status_t MCAL_USART_Init(USART_RegDef_t *usart, const mcal_usart_config_t *config)
{
    uint32 cr1;
    uint32 cr2;
    uint32 cr3;
    mcal_usart_status_t status;

    if ((usart == NULL) || (config == NULL) || (config->baud_rate == 0U) || (config->pclk_hz == 0U))
    {
        return MCAL_USART_STATUS_INVALID_PARAM;
    }

    cr1 = 0U;
    cr2 = 0U;
    cr3 = 0U;

    if ((config->mode == MCAL_USART_MODE_TX) || (config->mode == MCAL_USART_MODE_TX_RX))
    {
        SET_BIT(cr1, MCAL_USART_CR1_TE_POS);
    }

    if ((config->mode == MCAL_USART_MODE_RX) || (config->mode == MCAL_USART_MODE_TX_RX))
    {
        SET_BIT(cr1, MCAL_USART_CR1_RE_POS);
    }

    if (config->word_length == MCAL_USART_WORDLENGTH_9BIT)
    {
        SET_BIT(cr1, MCAL_USART_CR1_M_POS);
    }

    if (config->parity != MCAL_USART_PARITY_NONE)
    {
        SET_BIT(cr1, MCAL_USART_CR1_PCE_POS);

        if (config->parity == MCAL_USART_PARITY_ODD)
        {
            SET_BIT(cr1, MCAL_USART_CR1_PS_POS);
        }
    }

    if (config->oversampling == MCAL_USART_OVERSAMPLING_8)
    {
        SET_BIT(cr1, MCAL_USART_CR1_OVER8_POS);
    }

    cr2 |= (((uint32)config->stop_bits & MCAL_USART_CR2_STOP_MASK) << MCAL_USART_CR2_STOP_POS);

    if (config->sync_clock == MCAL_USART_SYNC_CLOCK_ENABLE)
    {
        SET_BIT(cr2, MCAL_USART_CR2_CLKEN_POS);

        if (config->clock_polarity == MCAL_USART_CLOCK_POLARITY_HIGH)
        {
            SET_BIT(cr2, MCAL_USART_CR2_CPOL_POS);
        }

        if (config->clock_phase == MCAL_USART_CLOCK_PHASE_2EDGE)
        {
            SET_BIT(cr2, MCAL_USART_CR2_CPHA_POS);
        }

        if (config->last_bit_clock == MCAL_USART_LASTBITCLOCK_ENABLE)
        {
            SET_BIT(cr2, MCAL_USART_CR2_LBCL_POS);
        }
    }

    if (config->lin_mode == MCAL_USART_LIN_ENABLE)
    {
        SET_BIT(cr2, MCAL_USART_CR2_LINEN_POS);
    }

    if ((config->hw_flow_control == MCAL_USART_HWFLOW_RTS) ||
        (config->hw_flow_control == MCAL_USART_HWFLOW_RTS_CTS))
    {
        SET_BIT(cr3, MCAL_USART_CR3_RTSE_POS);
    }

    if ((config->hw_flow_control == MCAL_USART_HWFLOW_CTS) ||
        (config->hw_flow_control == MCAL_USART_HWFLOW_RTS_CTS))
    {
        SET_BIT(cr3, MCAL_USART_CR3_CTSE_POS);
    }

    if (config->half_duplex == MCAL_USART_HALFDUPLEX_ENABLE)
    {
        SET_BIT(cr3, MCAL_USART_CR3_HDSEL_POS);
    }

    if (config->irda_mode == MCAL_USART_IRDA_ENABLE)
    {
        SET_BIT(cr3, MCAL_USART_CR3_IREN_POS);

        if (config->irda_power == MCAL_USART_IRDA_LOW_POWER)
        {
            SET_BIT(cr3, MCAL_USART_CR3_IRLP_POS);
        }
    }

    if (config->smartcard_mode == MCAL_USART_SMARTCARD_ENABLE)
    {
        SET_BIT(cr3, MCAL_USART_CR3_SCEN_POS);
        SET_BIT(cr3, MCAL_USART_CR3_NACK_POS);
    }

    if (config->dma_rx == MCAL_USART_DMA_RX_ENABLE)
    {
        SET_BIT(cr3, MCAL_USART_CR3_DMAR_POS);
    }

    if (config->dma_tx == MCAL_USART_DMA_TX_ENABLE)
    {
        SET_BIT(cr3, MCAL_USART_CR3_DMAT_POS);
    }

    if (config->one_bit_sampling == MCAL_USART_ONEBIT_ENABLE)
    {
        SET_BIT(cr3, MCAL_USART_CR3_ONEBIT_POS);
    }

    CLR_BIT(usart->CR1, MCAL_USART_CR1_UE_POS);

    usart->CR1 = cr1;
    usart->CR2 = cr2;
    usart->CR3 = cr3;

    status = MCAL_USART_SetBaudRate(usart, config->pclk_hz, config->baud_rate, config->oversampling);
    if (status != MCAL_USART_STATUS_OK)
    {
        return status;
    }

    SET_BIT(usart->CR1, MCAL_USART_CR1_UE_POS);

    return MCAL_USART_STATUS_OK;
}

void MCAL_USART_DeInit(USART_RegDef_t *usart)
{
    uint8 index;

    if (usart == NULL)
    {
        return;
    }

    MCAL_USART_Disable(usart);

    if (usart == USART1)
    {
        SET_BIT(RCC->APB2RSTR, 4U);
        CLR_BIT(RCC->APB2RSTR, 4U);
    }
    else if (usart == USART2)
    {
        SET_BIT(RCC->APB1RSTR, 17U);
        CLR_BIT(RCC->APB1RSTR, 17U);
    }
    else if (usart == USART6)
    {
        SET_BIT(RCC->APB2RSTR, 5U);
        CLR_BIT(RCC->APB2RSTR, 5U);
    }
    else if (usart == UART4)
    {
        SET_BIT(RCC->APB1RSTR, 19U);
        CLR_BIT(RCC->APB1RSTR, 19U);
    }
    else if (usart == UART5)
    {
        SET_BIT(RCC->APB1RSTR, 20U);
        CLR_BIT(RCC->APB1RSTR, 20U);
    }
    else
    {
    }

    index = MCAL_USART_GetInstanceIndex(usart);
    if (index != MCAL_USART_INSTANCE_INDEX_INVALID)
    {
        g_mcal_usart_callbacks[index] = NULL;
    }
}

void MCAL_USART_Enable(USART_RegDef_t *usart)
{
    if (usart == NULL)
    {
        return;
    }

    SET_BIT(usart->CR1, MCAL_USART_CR1_UE_POS);
}

void MCAL_USART_Disable(USART_RegDef_t *usart)
{
    if (usart == NULL)
    {
        return;
    }

    CLR_BIT(usart->CR1, MCAL_USART_CR1_UE_POS);
}

mcal_usart_status_t MCAL_USART_SetBaudRate(USART_RegDef_t *usart,
                                           uint32 pclk_hz,
                                           uint32 baud_rate,
                                           mcal_usart_oversampling_t oversampling)
{
    uint32 usartdiv_x100;
    uint32 mantissa;
    uint32 fraction;
    uint32 brr;

    if ((usart == NULL) || (pclk_hz == 0U) || (baud_rate == 0U))
    {
        return MCAL_USART_STATUS_INVALID_PARAM;
    }

    if (oversampling == MCAL_USART_OVERSAMPLING_8)
    {
        SET_BIT(usart->CR1, MCAL_USART_CR1_OVER8_POS);

        usartdiv_x100 = (25U * pclk_hz) / (2U * baud_rate);
        mantissa = usartdiv_x100 / 100U;
        fraction = (((usartdiv_x100 - (mantissa * 100U)) * 8U) + 50U) / 100U;

        if (fraction > 7U)
        {
            mantissa++;
            fraction = 0U;
        }

        brr = (mantissa << 4U) | (fraction & 0x7U);
    }
    else
    {
        CLR_BIT(usart->CR1, MCAL_USART_CR1_OVER8_POS);

        usartdiv_x100 = (25U * pclk_hz) / (4U * baud_rate);
        mantissa = usartdiv_x100 / 100U;
        fraction = (((usartdiv_x100 - (mantissa * 100U)) * 16U) + 50U) / 100U;

        if (fraction > 15U)
        {
            mantissa++;
            fraction = 0U;
        }

        brr = (mantissa << 4U) | (fraction & 0xFU);
    }

    usart->BRR = brr;

    return MCAL_USART_STATUS_OK;
}

mcal_usart_status_t MCAL_USART_SendData(USART_RegDef_t *usart, const uint8 *data, uint16 length)
{
    uint16 index;
    mcal_usart_status_t status;

    if ((usart == NULL) || (data == NULL) || (length == 0U))
    {
        return MCAL_USART_STATUS_INVALID_PARAM;
    }

    for (index = 0U; index < length; index++)
    {
        status = MCAL_USART_WaitFlagState(usart, MCAL_USART_FLAG_TXE, 1U);
        if (status != MCAL_USART_STATUS_OK)
        {
            return status;
        }

        usart->DR = data[index];
    }

    status = MCAL_USART_WaitFlagState(usart, MCAL_USART_FLAG_TC, 1U);
    if (status != MCAL_USART_STATUS_OK)
    {
        return status;
    }

    return MCAL_USART_STATUS_OK;
}

mcal_usart_status_t MCAL_USART_ReceiveData(USART_RegDef_t *usart, uint8 *data, uint16 length)
{
    uint16 index;
    mcal_usart_status_t status;

    if ((usart == NULL) || (data == NULL) || (length == 0U))
    {
        return MCAL_USART_STATUS_INVALID_PARAM;
    }

    for (index = 0U; index < length; index++)
    {
        status = MCAL_USART_WaitFlagState(usart, MCAL_USART_FLAG_RXNE, 1U);
        if (status != MCAL_USART_STATUS_OK)
        {
            return status;
        }

        data[index] = (uint8)(usart->DR & 0xFFU);
    }

    return MCAL_USART_STATUS_OK;
}

mcal_usart_status_t MCAL_USART_SendString(USART_RegDef_t *usart, const uint8 *string)
{
    mcal_usart_status_t status;
    uint16 index;

    if ((usart == NULL) || (string == NULL))
    {
        return MCAL_USART_STATUS_INVALID_PARAM;
    }

    index = 0U;
    while (string[index] != (uint8)'\0')
    {
        status = MCAL_USART_SendData(usart, &string[index], 1U);
        if (status != MCAL_USART_STATUS_OK)
        {
            return status;
        }

        index++;
    }

    return MCAL_USART_STATUS_OK;
}

uint8 MCAL_USART_GetFlagStatus(USART_RegDef_t *usart, mcal_usart_flag_t flag)
{
    if (usart == NULL)
    {
        return 0U;
    }

    return (uint8)GET_BIT(usart->SR, (uint8)flag);
}

void MCAL_USART_ClearFlag(USART_RegDef_t *usart, mcal_usart_flag_t flag)
{
    volatile uint32 dummy;

    if (usart == NULL)
    {
        return;
    }

    switch (flag)
    {
        case MCAL_USART_FLAG_PE:
        case MCAL_USART_FLAG_FE:
        case MCAL_USART_FLAG_NF:
        case MCAL_USART_FLAG_ORE:
        case MCAL_USART_FLAG_IDLE:
            dummy = usart->SR;
            dummy = usart->DR;
            (void)dummy;
            break;

        case MCAL_USART_FLAG_TC:
        case MCAL_USART_FLAG_LBD:
        case MCAL_USART_FLAG_CTS:
            CLR_BIT(usart->SR, (uint8)flag);
            break;

        default:
            break;
    }
}

void MCAL_USART_EnableInterrupt(USART_RegDef_t *usart, mcal_usart_interrupt_t interrupt_source)
{
    if (usart == NULL)
    {
        return;
    }

    switch (interrupt_source)
    {
        case MCAL_USART_INTERRUPT_PE:
            SET_BIT(usart->CR1, MCAL_USART_CR1_PEIE_POS);
            break;

        case MCAL_USART_INTERRUPT_TXE:
            SET_BIT(usart->CR1, MCAL_USART_CR1_TXEIE_POS);
            break;

        case MCAL_USART_INTERRUPT_TC:
            SET_BIT(usart->CR1, MCAL_USART_CR1_TCIE_POS);
            break;

        case MCAL_USART_INTERRUPT_RXNE:
            SET_BIT(usart->CR1, MCAL_USART_CR1_RXNEIE_POS);
            break;

        case MCAL_USART_INTERRUPT_IDLE:
            SET_BIT(usart->CR1, MCAL_USART_CR1_IDLEIE_POS);
            break;

        case MCAL_USART_INTERRUPT_LBD:
            SET_BIT(usart->CR2, MCAL_USART_CR2_LBDIE_POS);
            break;

        case MCAL_USART_INTERRUPT_CTS:
            SET_BIT(usart->CR3, MCAL_USART_CR3_CTSIE_POS);
            break;

        case MCAL_USART_INTERRUPT_ERROR:
            SET_BIT(usart->CR3, MCAL_USART_CR3_EIE_POS);
            break;

        default:
            break;
    }
}

void MCAL_USART_DisableInterrupt(USART_RegDef_t *usart, mcal_usart_interrupt_t interrupt_source)
{
    if (usart == NULL)
    {
        return;
    }

    switch (interrupt_source)
    {
        case MCAL_USART_INTERRUPT_PE:
            CLR_BIT(usart->CR1, MCAL_USART_CR1_PEIE_POS);
            break;

        case MCAL_USART_INTERRUPT_TXE:
            CLR_BIT(usart->CR1, MCAL_USART_CR1_TXEIE_POS);
            break;

        case MCAL_USART_INTERRUPT_TC:
            CLR_BIT(usart->CR1, MCAL_USART_CR1_TCIE_POS);
            break;

        case MCAL_USART_INTERRUPT_RXNE:
            CLR_BIT(usart->CR1, MCAL_USART_CR1_RXNEIE_POS);
            break;

        case MCAL_USART_INTERRUPT_IDLE:
            CLR_BIT(usart->CR1, MCAL_USART_CR1_IDLEIE_POS);
            break;

        case MCAL_USART_INTERRUPT_LBD:
            CLR_BIT(usart->CR2, MCAL_USART_CR2_LBDIE_POS);
            break;

        case MCAL_USART_INTERRUPT_CTS:
            CLR_BIT(usart->CR3, MCAL_USART_CR3_CTSIE_POS);
            break;

        case MCAL_USART_INTERRUPT_ERROR:
            CLR_BIT(usart->CR3, MCAL_USART_CR3_EIE_POS);
            break;

        default:
            break;
    }
}

mcal_usart_status_t MCAL_USART_SetCallback(USART_RegDef_t *usart, mcal_usart_callback_t callback)
{
    uint8 index;

    if (usart == NULL)
    {
        return MCAL_USART_STATUS_INVALID_PARAM;
    }

    index = MCAL_USART_GetInstanceIndex(usart);
    if (index == MCAL_USART_INSTANCE_INDEX_INVALID)
    {
        return MCAL_USART_STATUS_INVALID_PARAM;
    }

    g_mcal_usart_callbacks[index] = callback;

    return MCAL_USART_STATUS_OK;
}

void MCAL_USART_IRQHandler(USART_RegDef_t *usart)
{
    uint8 index;
    mcal_usart_callback_t callback;

    if (usart == NULL)
    {
        return;
    }

    index = MCAL_USART_GetInstanceIndex(usart);
    if (index == MCAL_USART_INSTANCE_INDEX_INVALID)
    {
        return;
    }

    callback = g_mcal_usart_callbacks[index];
    if (callback == NULL)
    {
        return;
    }

    if ((GET_BIT(usart->SR, MCAL_USART_SR_RXNE_POS) == 1U) &&
        (GET_BIT(usart->CR1, MCAL_USART_CR1_RXNEIE_POS) == 1U))
    {
        callback(usart, MCAL_USART_EVENT_RXNE);
    }

    if ((GET_BIT(usart->SR, MCAL_USART_SR_TXE_POS) == 1U) &&
        (GET_BIT(usart->CR1, MCAL_USART_CR1_TXEIE_POS) == 1U))
    {
        callback(usart, MCAL_USART_EVENT_TXE);
    }

    if ((GET_BIT(usart->SR, MCAL_USART_SR_TC_POS) == 1U) &&
        (GET_BIT(usart->CR1, MCAL_USART_CR1_TCIE_POS) == 1U))
    {
        callback(usart, MCAL_USART_EVENT_TC);
    }

    if ((GET_BIT(usart->SR, MCAL_USART_SR_IDLE_POS) == 1U) &&
        (GET_BIT(usart->CR1, MCAL_USART_CR1_IDLEIE_POS) == 1U))
    {
        MCAL_USART_ClearFlag(usart, MCAL_USART_FLAG_IDLE);
        callback(usart, MCAL_USART_EVENT_IDLE);
    }

    if ((GET_BIT(usart->SR, MCAL_USART_SR_PE_POS) == 1U) &&
        (GET_BIT(usart->CR1, MCAL_USART_CR1_PEIE_POS) == 1U))
    {
        MCAL_USART_ClearFlag(usart, MCAL_USART_FLAG_PE);
        callback(usart, MCAL_USART_EVENT_PE);
    }

    if ((GET_BIT(usart->SR, MCAL_USART_SR_LBD_POS) == 1U) &&
        (GET_BIT(usart->CR2, MCAL_USART_CR2_LBDIE_POS) == 1U))
    {
        MCAL_USART_ClearFlag(usart, MCAL_USART_FLAG_LBD);
        callback(usart, MCAL_USART_EVENT_LBD);
    }

    if ((GET_BIT(usart->SR, MCAL_USART_SR_CTS_POS) == 1U) &&
        (GET_BIT(usart->CR3, MCAL_USART_CR3_CTSIE_POS) == 1U))
    {
        MCAL_USART_ClearFlag(usart, MCAL_USART_FLAG_CTS);
        callback(usart, MCAL_USART_EVENT_CTS);
    }

    if (((GET_BIT(usart->SR, MCAL_USART_SR_ORE_POS) == 1U) ||
         (GET_BIT(usart->SR, MCAL_USART_SR_FE_POS) == 1U) ||
         (GET_BIT(usart->SR, MCAL_USART_SR_NF_POS) == 1U)) &&
        (GET_BIT(usart->CR3, MCAL_USART_CR3_EIE_POS) == 1U))
    {
        MCAL_USART_ClearFlag(usart, MCAL_USART_FLAG_ORE);
        callback(usart, MCAL_USART_EVENT_ERROR);
    }
}
