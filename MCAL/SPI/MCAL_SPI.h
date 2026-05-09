/*
 * MCAL_SPI.h
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

#ifndef MCAL_SPI_H
#define MCAL_SPI_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "PLATFORM_TYPES.h"
#include "STM32F401RC.h"

/* ================================================================ */
/* ========================== SPI Macros ========================== */
/* ================================================================ */
#define MCAL_SPI_INSTANCE_INDEX_INVALID     (0xFFU)
#define MCAL_SPI_TIMEOUT_VALUE              (1000000UL)

#define MCAL_SPI_CR1_CPHA_POS               (0U)
#define MCAL_SPI_CR1_CPOL_POS               (1U)
#define MCAL_SPI_CR1_MSTR_POS               (2U)
#define MCAL_SPI_CR1_BR_POS                 (3U)
#define MCAL_SPI_CR1_BR_MASK                (0x7UL)
#define MCAL_SPI_CR1_SPE_POS                (6U)
#define MCAL_SPI_CR1_LSBFIRST_POS           (7U)
#define MCAL_SPI_CR1_SSI_POS                (8U)
#define MCAL_SPI_CR1_SSM_POS                (9U)
#define MCAL_SPI_CR1_RXONLY_POS             (10U)
#define MCAL_SPI_CR1_DFF_POS                (11U)
#define MCAL_SPI_CR1_CRCNEXT_POS            (12U)
#define MCAL_SPI_CR1_CRCEN_POS              (13U)
#define MCAL_SPI_CR1_BIDIOE_POS             (14U)
#define MCAL_SPI_CR1_BIDIMODE_POS           (15U)

#define MCAL_SPI_CR2_RXDMAEN_POS            (0U)
#define MCAL_SPI_CR2_TXDMAEN_POS            (1U)
#define MCAL_SPI_CR2_SSOE_POS               (2U)
#define MCAL_SPI_CR2_ERRIE_POS              (5U)
#define MCAL_SPI_CR2_RXNEIE_POS             (6U)
#define MCAL_SPI_CR2_TXEIE_POS              (7U)
#define MCAL_SPI_CR2_FRF_POS                (4U)

#define MCAL_SPI_SR_RXNE_POS                (0U)
#define MCAL_SPI_SR_TXE_POS                 (1U)
#define MCAL_SPI_SR_CRCERR_POS              (4U)
#define MCAL_SPI_SR_MODF_POS                (5U)
#define MCAL_SPI_SR_OVR_POS                 (6U)
#define MCAL_SPI_SR_BSY_POS                 (7U)

/* ================================================================ */
/* =============== Macros Configuration References ================ */
/* ================================================================ */
typedef enum
{
    MCAL_SPI_STATUS_OK = 0U,
    MCAL_SPI_STATUS_ERROR,
    MCAL_SPI_STATUS_TIMEOUT,
    MCAL_SPI_STATUS_BUSY,
    MCAL_SPI_STATUS_INVALID_PARAM
} mcal_spi_status_t;

typedef enum
{
    MCAL_SPI_DEVICE_SLAVE = 0U,
    MCAL_SPI_DEVICE_MASTER
} mcal_spi_device_mode_t;

typedef enum
{
    MCAL_SPI_BUS_FULL_DUPLEX = 0U,
    MCAL_SPI_BUS_HALF_DUPLEX,
    MCAL_SPI_BUS_SIMPLEX_RXONLY
} mcal_spi_bus_config_t;

typedef enum
{
    MCAL_SPI_DFF_8BIT = 0U,
    MCAL_SPI_DFF_16BIT
} mcal_spi_data_frame_t;

typedef enum
{
    MCAL_SPI_CPOL_LOW = 0U,
    MCAL_SPI_CPOL_HIGH
} mcal_spi_clock_polarity_t;

typedef enum
{
    MCAL_SPI_CPHA_FIRST_EDGE = 0U,
    MCAL_SPI_CPHA_SECOND_EDGE
} mcal_spi_clock_phase_t;

typedef enum
{
    MCAL_SPI_BAUD_DIV2 = 0U,
    MCAL_SPI_BAUD_DIV4,
    MCAL_SPI_BAUD_DIV8,
    MCAL_SPI_BAUD_DIV16,
    MCAL_SPI_BAUD_DIV32,
    MCAL_SPI_BAUD_DIV64,
    MCAL_SPI_BAUD_DIV128,
    MCAL_SPI_BAUD_DIV256
} mcal_spi_baud_prescaler_t;

typedef enum
{
    MCAL_SPI_FIRSTBIT_MSB = 0U,
    MCAL_SPI_FIRSTBIT_LSB
} mcal_spi_firstbit_t;

typedef enum
{
    MCAL_SPI_SSM_DISABLE = 0U,
    MCAL_SPI_SSM_ENABLE
} mcal_spi_ssm_t;

typedef enum
{
    MCAL_SPI_NSS_HARDWARE = 0U,
    MCAL_SPI_NSS_SOFTWARE_INTERNAL
} mcal_spi_nss_mode_t;

typedef enum
{
    MCAL_SPI_CRC_DISABLE = 0U,
    MCAL_SPI_CRC_ENABLE
} mcal_spi_crc_t;

typedef enum
{
    MCAL_SPI_TI_MODE_DISABLE = 0U,
    MCAL_SPI_TI_MODE_ENABLE
} mcal_spi_ti_mode_t;

typedef enum
{
    MCAL_SPI_DMA_RX_DISABLE = 0U,
    MCAL_SPI_DMA_RX_ENABLE
} mcal_spi_dma_rx_t;

typedef enum
{
    MCAL_SPI_DMA_TX_DISABLE = 0U,
    MCAL_SPI_DMA_TX_ENABLE
} mcal_spi_dma_tx_t;

typedef enum
{
    MCAL_SPI_FLAG_RXNE = MCAL_SPI_SR_RXNE_POS,
    MCAL_SPI_FLAG_TXE = MCAL_SPI_SR_TXE_POS,
    MCAL_SPI_FLAG_CRCERR = MCAL_SPI_SR_CRCERR_POS,
    MCAL_SPI_FLAG_MODF = MCAL_SPI_SR_MODF_POS,
    MCAL_SPI_FLAG_OVR = MCAL_SPI_SR_OVR_POS,
    MCAL_SPI_FLAG_BSY = MCAL_SPI_SR_BSY_POS
} mcal_spi_flag_t;

typedef enum
{
    MCAL_SPI_INTERRUPT_TXE = 0U,
    MCAL_SPI_INTERRUPT_RXNE,
    MCAL_SPI_INTERRUPT_ERR
} mcal_spi_interrupt_t;

typedef enum
{
    MCAL_SPI_EVENT_TXE = 0U,
    MCAL_SPI_EVENT_RXNE,
    MCAL_SPI_EVENT_OVR,
    MCAL_SPI_EVENT_MODF,
    MCAL_SPI_EVENT_CRCERR,
    MCAL_SPI_EVENT_ERROR
} mcal_spi_event_t;

typedef void (*mcal_spi_callback_t)(SPI_RegDef_t *spi, mcal_spi_event_t event);

typedef struct
{
    mcal_spi_device_mode_t device_mode;
    mcal_spi_bus_config_t bus_config;
    mcal_spi_data_frame_t data_frame;
    mcal_spi_clock_polarity_t clock_polarity;
    mcal_spi_clock_phase_t clock_phase;
    mcal_spi_baud_prescaler_t baud_prescaler;
    mcal_spi_firstbit_t first_bit;
    mcal_spi_ssm_t software_slave_management;
    mcal_spi_nss_mode_t nss_mode;
    mcal_spi_crc_t crc_calculation;
    mcal_spi_ti_mode_t ti_mode;
    mcal_spi_dma_rx_t dma_rx;
    mcal_spi_dma_tx_t dma_tx;
} mcal_spi_config_t;

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
mcal_spi_status_t MCAL_SPI_Init(SPI_RegDef_t *spi, const mcal_spi_config_t *config);
void MCAL_SPI_DeInit(SPI_RegDef_t *spi);
void MCAL_SPI_Enable(SPI_RegDef_t *spi);
void MCAL_SPI_Disable(SPI_RegDef_t *spi);

mcal_spi_status_t MCAL_SPI_SendData(SPI_RegDef_t *spi, const uint8 *data, uint16 length);
mcal_spi_status_t MCAL_SPI_ReceiveData(SPI_RegDef_t *spi, uint8 *data, uint16 length);
mcal_spi_status_t MCAL_SPI_TransmitReceive(SPI_RegDef_t *spi,
                                           const uint8 *tx_data,
                                           uint8 *rx_data,
                                           uint16 length);

uint8 MCAL_SPI_GetFlagStatus(SPI_RegDef_t *spi, mcal_spi_flag_t flag);
void MCAL_SPI_ClearOVRFlag(SPI_RegDef_t *spi);

void MCAL_SPI_EnableInterrupt(SPI_RegDef_t *spi, mcal_spi_interrupt_t interrupt_source);
void MCAL_SPI_DisableInterrupt(SPI_RegDef_t *spi, mcal_spi_interrupt_t interrupt_source);
mcal_spi_status_t MCAL_SPI_SetCallback(SPI_RegDef_t *spi, mcal_spi_callback_t callback);
void MCAL_SPI_IRQHandler(SPI_RegDef_t *spi);

#endif
