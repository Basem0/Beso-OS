/*
 * MCAL_SPI.c
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "MCAL_SPI.h"

#include "UTILS.h"

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
static mcal_spi_callback_t g_mcal_spi_callbacks[4] = {NULL, NULL, NULL, NULL};

static uint8 MCAL_SPI_GetInstanceIndex(SPI_RegDef_t *spi);
static mcal_spi_status_t MCAL_SPI_WaitFlagState(SPI_RegDef_t *spi,
                                                 mcal_spi_flag_t flag,
                                                 uint8 expected_state);

/* ================================================================ */
/* ================= Static Function Definitions ================== */
/* ================================================================ */
static uint8 MCAL_SPI_GetInstanceIndex(SPI_RegDef_t *spi)
{
    if (spi == SPI1)
    {
        return 0U;
    }

    if (spi == SPI2)
    {
        return 1U;
    }

    if (spi == SPI3)
    {
        return 2U;
    }

    if (spi == SPI4)
    {
        return 3U;
    }

    return MCAL_SPI_INSTANCE_INDEX_INVALID;
}

static mcal_spi_status_t MCAL_SPI_WaitFlagState(SPI_RegDef_t *spi,
                                                 mcal_spi_flag_t flag,
                                                 uint8 expected_state)
{
    uint32 timeout;

    if (spi == NULL)
    {
        return MCAL_SPI_STATUS_INVALID_PARAM;
    }

    timeout = MCAL_SPI_TIMEOUT_VALUE;

    while (timeout > 0U)
    {
        if (MCAL_SPI_GetFlagStatus(spi, flag) == expected_state)
        {
            return MCAL_SPI_STATUS_OK;
        }

        timeout--;
    }

    return MCAL_SPI_STATUS_TIMEOUT;
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
mcal_spi_status_t MCAL_SPI_Init(SPI_RegDef_t *spi, const mcal_spi_config_t *config)
{
    uint32 cr1;
    uint32 cr2;

    if ((spi == NULL) || (config == NULL))
    {
        return MCAL_SPI_STATUS_INVALID_PARAM;
    }

    cr1 = 0U;
    cr2 = 0U;

    if (config->device_mode == MCAL_SPI_DEVICE_MASTER)
    {
        SET_BIT(cr1, MCAL_SPI_CR1_MSTR_POS);
    }

    if (config->clock_phase == MCAL_SPI_CPHA_SECOND_EDGE)
    {
        SET_BIT(cr1, MCAL_SPI_CR1_CPHA_POS);
    }

    if (config->clock_polarity == MCAL_SPI_CPOL_HIGH)
    {
        SET_BIT(cr1, MCAL_SPI_CR1_CPOL_POS);
    }

    cr1 |= (((uint32)config->baud_prescaler & MCAL_SPI_CR1_BR_MASK) << MCAL_SPI_CR1_BR_POS);

    if (config->first_bit == MCAL_SPI_FIRSTBIT_LSB)
    {
        SET_BIT(cr1, MCAL_SPI_CR1_LSBFIRST_POS);
    }

    if (config->data_frame == MCAL_SPI_DFF_16BIT)
    {
        SET_BIT(cr1, MCAL_SPI_CR1_DFF_POS);
    }

    if (config->software_slave_management == MCAL_SPI_SSM_ENABLE)
    {
        SET_BIT(cr1, MCAL_SPI_CR1_SSM_POS);

        if (config->nss_mode == MCAL_SPI_NSS_SOFTWARE_INTERNAL)
        {
            SET_BIT(cr1, MCAL_SPI_CR1_SSI_POS);
        }
    }
    else
    {
        CLR_BIT(cr1, MCAL_SPI_CR1_SSM_POS);

        if ((config->device_mode == MCAL_SPI_DEVICE_MASTER) &&
            (config->nss_mode == MCAL_SPI_NSS_HARDWARE))
        {
            SET_BIT(cr2, MCAL_SPI_CR2_SSOE_POS);
        }
    }

    switch (config->bus_config)
    {
        case MCAL_SPI_BUS_FULL_DUPLEX:
            CLR_BIT(cr1, MCAL_SPI_CR1_BIDIMODE_POS);
            CLR_BIT(cr1, MCAL_SPI_CR1_RXONLY_POS);
            break;

        case MCAL_SPI_BUS_SIMPLEX_RXONLY:
            CLR_BIT(cr1, MCAL_SPI_CR1_BIDIMODE_POS);
            SET_BIT(cr1, MCAL_SPI_CR1_RXONLY_POS);
            break;

        case MCAL_SPI_BUS_HALF_DUPLEX:
            SET_BIT(cr1, MCAL_SPI_CR1_BIDIMODE_POS);

            if (config->device_mode == MCAL_SPI_DEVICE_MASTER)
            {
                SET_BIT(cr1, MCAL_SPI_CR1_BIDIOE_POS);
            }
            else
            {
                CLR_BIT(cr1, MCAL_SPI_CR1_BIDIOE_POS);
            }
            break;

        default:
            return MCAL_SPI_STATUS_INVALID_PARAM;
    }

    if (config->crc_calculation == MCAL_SPI_CRC_ENABLE)
    {
        SET_BIT(cr1, MCAL_SPI_CR1_CRCEN_POS);
    }

    if (config->ti_mode == MCAL_SPI_TI_MODE_ENABLE)
    {
        SET_BIT(cr2, MCAL_SPI_CR2_FRF_POS);
    }

    if (config->dma_rx == MCAL_SPI_DMA_RX_ENABLE)
    {
        SET_BIT(cr2, MCAL_SPI_CR2_RXDMAEN_POS);
    }

    if (config->dma_tx == MCAL_SPI_DMA_TX_ENABLE)
    {
        SET_BIT(cr2, MCAL_SPI_CR2_TXDMAEN_POS);
    }

    spi->CR1 = cr1;
    spi->CR2 = cr2;

    return MCAL_SPI_STATUS_OK;
}

void MCAL_SPI_DeInit(SPI_RegDef_t *spi)
{
    uint8 index;

    if (spi == NULL)
    {
        return;
    }

    MCAL_SPI_Disable(spi);

    if (spi == SPI1)
    {
        SET_BIT(RCC->APB2RSTR, 12U);
        CLR_BIT(RCC->APB2RSTR, 12U);
    }
    else if (spi == SPI2)
    {
        SET_BIT(RCC->APB1RSTR, 14U);
        CLR_BIT(RCC->APB1RSTR, 14U);
    }
    else if (spi == SPI3)
    {
        SET_BIT(RCC->APB1RSTR, 15U);
        CLR_BIT(RCC->APB1RSTR, 15U);
    }
    else if (spi == SPI4)
    {
        SET_BIT(RCC->APB2RSTR, 13U);
        CLR_BIT(RCC->APB2RSTR, 13U);
    }
    else
    {
    }

    index = MCAL_SPI_GetInstanceIndex(spi);
    if (index != MCAL_SPI_INSTANCE_INDEX_INVALID)
    {
        g_mcal_spi_callbacks[index] = NULL;
    }
}

void MCAL_SPI_Enable(SPI_RegDef_t *spi)
{
    if (spi == NULL)
    {
        return;
    }

    SET_BIT(spi->CR1, MCAL_SPI_CR1_SPE_POS);
}

void MCAL_SPI_Disable(SPI_RegDef_t *spi)
{
    if (spi == NULL)
    {
        return;
    }

    CLR_BIT(spi->CR1, MCAL_SPI_CR1_SPE_POS);
}

mcal_spi_status_t MCAL_SPI_SendData(SPI_RegDef_t *spi, const uint8 *data, uint16 length)
{
    uint16 index;
    uint16 frame16;
    mcal_spi_status_t status;

    if ((spi == NULL) || (data == NULL) || (length == 0U))
    {
        return MCAL_SPI_STATUS_INVALID_PARAM;
    }

    index = 0U;

    while (index < length)
    {
        status = MCAL_SPI_WaitFlagState(spi, MCAL_SPI_FLAG_TXE, 1U);
        if (status != MCAL_SPI_STATUS_OK)
        {
            return status;
        }

        if (GET_BIT(spi->CR1, MCAL_SPI_CR1_DFF_POS) == 1U)
        {
            frame16 = data[index];

            if ((index + 1U) < length)
            {
                frame16 |= ((uint16)data[index + 1U] << 8U);
            }

            spi->DR = frame16;
            index += 2U;
        }
        else
        {
            spi->DR = data[index];
            index++;
        }
    }

    status = MCAL_SPI_WaitFlagState(spi, MCAL_SPI_FLAG_BSY, 0U);
    if (status != MCAL_SPI_STATUS_OK)
    {
        return status;
    }

    return MCAL_SPI_STATUS_OK;
}

mcal_spi_status_t MCAL_SPI_ReceiveData(SPI_RegDef_t *spi, uint8 *data, uint16 length)
{
    uint16 index;
    uint16 frame16;
    mcal_spi_status_t status;

    if ((spi == NULL) || (data == NULL) || (length == 0U))
    {
        return MCAL_SPI_STATUS_INVALID_PARAM;
    }

    index = 0U;

    while (index < length)
    {
        status = MCAL_SPI_WaitFlagState(spi, MCAL_SPI_FLAG_RXNE, 1U);
        if (status != MCAL_SPI_STATUS_OK)
        {
            return status;
        }

        if (GET_BIT(spi->CR1, MCAL_SPI_CR1_DFF_POS) == 1U)
        {
            frame16 = (uint16)(spi->DR & 0xFFFFU);
            data[index] = (uint8)(frame16 & 0xFFU);

            if ((index + 1U) < length)
            {
                data[index + 1U] = (uint8)((frame16 >> 8U) & 0xFFU);
            }

            index += 2U;
        }
        else
        {
            data[index] = (uint8)(spi->DR & 0xFFU);
            index++;
        }
    }

    return MCAL_SPI_STATUS_OK;
}

mcal_spi_status_t MCAL_SPI_TransmitReceive(SPI_RegDef_t *spi,
                                           const uint8 *tx_data,
                                           uint8 *rx_data,
                                           uint16 length)
{
    uint16 index;
    mcal_spi_status_t status;

    if ((spi == NULL) || (tx_data == NULL) || (rx_data == NULL) || (length == 0U))
    {
        return MCAL_SPI_STATUS_INVALID_PARAM;
    }

    for (index = 0U; index < length; index++)
    {
        status = MCAL_SPI_WaitFlagState(spi, MCAL_SPI_FLAG_TXE, 1U);
        if (status != MCAL_SPI_STATUS_OK)
        {
            return status;
        }

        spi->DR = tx_data[index];

        status = MCAL_SPI_WaitFlagState(spi, MCAL_SPI_FLAG_RXNE, 1U);
        if (status != MCAL_SPI_STATUS_OK)
        {
            return status;
        }

        rx_data[index] = (uint8)(spi->DR & 0xFFU);
    }

    status = MCAL_SPI_WaitFlagState(spi, MCAL_SPI_FLAG_BSY, 0U);
    if (status != MCAL_SPI_STATUS_OK)
    {
        return status;
    }

    return MCAL_SPI_STATUS_OK;
}

uint8 MCAL_SPI_GetFlagStatus(SPI_RegDef_t *spi, mcal_spi_flag_t flag)
{
    if (spi == NULL)
    {
        return 0U;
    }

    return (uint8)GET_BIT(spi->SR, (uint8)flag);
}

void MCAL_SPI_ClearOVRFlag(SPI_RegDef_t *spi)
{
    volatile uint32 dummy;

    if (spi == NULL)
    {
        return;
    }

    dummy = spi->DR;
    dummy = spi->SR;
    (void)dummy;
}

void MCAL_SPI_EnableInterrupt(SPI_RegDef_t *spi, mcal_spi_interrupt_t interrupt_source)
{
    if (spi == NULL)
    {
        return;
    }

    switch (interrupt_source)
    {
        case MCAL_SPI_INTERRUPT_TXE:
            SET_BIT(spi->CR2, MCAL_SPI_CR2_TXEIE_POS);
            break;

        case MCAL_SPI_INTERRUPT_RXNE:
            SET_BIT(spi->CR2, MCAL_SPI_CR2_RXNEIE_POS);
            break;

        case MCAL_SPI_INTERRUPT_ERR:
            SET_BIT(spi->CR2, MCAL_SPI_CR2_ERRIE_POS);
            break;

        default:
            break;
    }
}

void MCAL_SPI_DisableInterrupt(SPI_RegDef_t *spi, mcal_spi_interrupt_t interrupt_source)
{
    if (spi == NULL)
    {
        return;
    }

    switch (interrupt_source)
    {
        case MCAL_SPI_INTERRUPT_TXE:
            CLR_BIT(spi->CR2, MCAL_SPI_CR2_TXEIE_POS);
            break;

        case MCAL_SPI_INTERRUPT_RXNE:
            CLR_BIT(spi->CR2, MCAL_SPI_CR2_RXNEIE_POS);
            break;

        case MCAL_SPI_INTERRUPT_ERR:
            CLR_BIT(spi->CR2, MCAL_SPI_CR2_ERRIE_POS);
            break;

        default:
            break;
    }
}

mcal_spi_status_t MCAL_SPI_SetCallback(SPI_RegDef_t *spi, mcal_spi_callback_t callback)
{
    uint8 index;

    if (spi == NULL)
    {
        return MCAL_SPI_STATUS_INVALID_PARAM;
    }

    index = MCAL_SPI_GetInstanceIndex(spi);
    if (index == MCAL_SPI_INSTANCE_INDEX_INVALID)
    {
        return MCAL_SPI_STATUS_INVALID_PARAM;
    }

    g_mcal_spi_callbacks[index] = callback;

    return MCAL_SPI_STATUS_OK;
}

void MCAL_SPI_IRQHandler(SPI_RegDef_t *spi)
{
    uint8 index;
    mcal_spi_callback_t callback;

    if (spi == NULL)
    {
        return;
    }

    index = MCAL_SPI_GetInstanceIndex(spi);
    if (index == MCAL_SPI_INSTANCE_INDEX_INVALID)
    {
        return;
    }

    callback = g_mcal_spi_callbacks[index];
    if (callback == NULL)
    {
        return;
    }

    if ((GET_BIT(spi->SR, MCAL_SPI_SR_TXE_POS) == 1U) &&
        (GET_BIT(spi->CR2, MCAL_SPI_CR2_TXEIE_POS) == 1U))
    {
        callback(spi, MCAL_SPI_EVENT_TXE);
    }

    if ((GET_BIT(spi->SR, MCAL_SPI_SR_RXNE_POS) == 1U) &&
        (GET_BIT(spi->CR2, MCAL_SPI_CR2_RXNEIE_POS) == 1U))
    {
        callback(spi, MCAL_SPI_EVENT_RXNE);
    }

    if ((GET_BIT(spi->CR2, MCAL_SPI_CR2_ERRIE_POS) == 1U) &&
        (GET_BIT(spi->SR, MCAL_SPI_SR_OVR_POS) == 1U))
    {
        MCAL_SPI_ClearOVRFlag(spi);
        callback(spi, MCAL_SPI_EVENT_OVR);
    }

    if ((GET_BIT(spi->CR2, MCAL_SPI_CR2_ERRIE_POS) == 1U) &&
        (GET_BIT(spi->SR, MCAL_SPI_SR_MODF_POS) == 1U))
    {
        callback(spi, MCAL_SPI_EVENT_MODF);
    }

    if ((GET_BIT(spi->CR2, MCAL_SPI_CR2_ERRIE_POS) == 1U) &&
        (GET_BIT(spi->SR, MCAL_SPI_SR_CRCERR_POS) == 1U))
    {
        CLR_BIT(spi->SR, MCAL_SPI_SR_CRCERR_POS);
        callback(spi, MCAL_SPI_EVENT_CRCERR);
    }
}
