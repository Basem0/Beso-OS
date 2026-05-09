/*
 * MCAL_USART.h
 *
 *  Created on: Apr 13, 2026
 *  Author: Ahmed Basem
 */

#ifndef MCAL_USART_H
#define MCAL_USART_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "PLATFORM_TYPES.h"
#include "STM32F401RC.h"

/* ================================================================ */
/* ========================= USART Macros ========================= */
/* ================================================================ */
#define MCAL_USART_INSTANCE_INDEX_INVALID   (0xFFU)

#define MCAL_USART_SR_PE_POS                (0U)
#define MCAL_USART_SR_FE_POS                (1U)
#define MCAL_USART_SR_NF_POS                (2U)
#define MCAL_USART_SR_ORE_POS               (3U)
#define MCAL_USART_SR_IDLE_POS              (4U)
#define MCAL_USART_SR_RXNE_POS              (5U)
#define MCAL_USART_SR_TC_POS                (6U)
#define MCAL_USART_SR_TXE_POS               (7U)
#define MCAL_USART_SR_LBD_POS               (8U)
#define MCAL_USART_SR_CTS_POS               (9U)

#define MCAL_USART_CR1_SBK_POS              (0U)
#define MCAL_USART_CR1_RWU_POS              (1U)
#define MCAL_USART_CR1_RE_POS               (2U)
#define MCAL_USART_CR1_TE_POS               (3U)
#define MCAL_USART_CR1_IDLEIE_POS           (4U)
#define MCAL_USART_CR1_RXNEIE_POS           (5U)
#define MCAL_USART_CR1_TCIE_POS             (6U)
#define MCAL_USART_CR1_TXEIE_POS            (7U)
#define MCAL_USART_CR1_PEIE_POS             (8U)
#define MCAL_USART_CR1_PS_POS               (9U)
#define MCAL_USART_CR1_PCE_POS              (10U)
#define MCAL_USART_CR1_WAKE_POS             (11U)
#define MCAL_USART_CR1_M_POS                (12U)
#define MCAL_USART_CR1_UE_POS               (13U)
#define MCAL_USART_CR1_OVER8_POS            (15U)

#define MCAL_USART_CR2_ADD_POS              (0U)
#define MCAL_USART_CR2_LBDL_POS             (5U)
#define MCAL_USART_CR2_LBDIE_POS            (6U)
#define MCAL_USART_CR2_LBCL_POS             (8U)
#define MCAL_USART_CR2_CPHA_POS             (9U)
#define MCAL_USART_CR2_CPOL_POS             (10U)
#define MCAL_USART_CR2_CLKEN_POS            (11U)
#define MCAL_USART_CR2_STOP_POS             (12U)
#define MCAL_USART_CR2_STOP_MASK            (0x3UL)
#define MCAL_USART_CR2_LINEN_POS            (14U)

#define MCAL_USART_CR3_EIE_POS              (0U)
#define MCAL_USART_CR3_IREN_POS             (1U)
#define MCAL_USART_CR3_IRLP_POS             (2U)
#define MCAL_USART_CR3_HDSEL_POS            (3U)
#define MCAL_USART_CR3_NACK_POS             (4U)
#define MCAL_USART_CR3_SCEN_POS             (5U)
#define MCAL_USART_CR3_DMAR_POS             (6U)
#define MCAL_USART_CR3_DMAT_POS             (7U)
#define MCAL_USART_CR3_RTSE_POS             (8U)
#define MCAL_USART_CR3_CTSE_POS             (9U)
#define MCAL_USART_CR3_CTSIE_POS            (10U)
#define MCAL_USART_CR3_ONEBIT_POS           (11U)

#define MCAL_USART_GTPR_PSC_POS             (0U)
#define MCAL_USART_GTPR_PSC_MASK            (0xFFUL)
#define MCAL_USART_GTPR_GT_POS              (8U)
#define MCAL_USART_GTPR_GT_MASK             (0xFFUL)

#define MCAL_USART_TIMEOUT_VALUE            (1000000UL)

/* ================================================================ */
/* =============== Macros Configuration References ================ */
/* ================================================================ */
typedef enum
{
    MCAL_USART_STATUS_OK = 0U,
    MCAL_USART_STATUS_ERROR,
    MCAL_USART_STATUS_TIMEOUT,
    MCAL_USART_STATUS_BUSY,
    MCAL_USART_STATUS_INVALID_PARAM
} mcal_usart_status_t;

typedef enum
{
    MCAL_USART_MODE_RX = 0U,
    MCAL_USART_MODE_TX,
    MCAL_USART_MODE_TX_RX
} mcal_usart_mode_t;

typedef enum
{
    MCAL_USART_WORDLENGTH_8BIT = 0U,
    MCAL_USART_WORDLENGTH_9BIT
} mcal_usart_wordlength_t;

typedef enum
{
    MCAL_USART_STOPBITS_1 = 0U,
    MCAL_USART_STOPBITS_0_5 = 1U,
    MCAL_USART_STOPBITS_2 = 2U,
    MCAL_USART_STOPBITS_1_5 = 3U
} mcal_usart_stopbits_t;

typedef enum
{
    MCAL_USART_PARITY_NONE = 0U,
    MCAL_USART_PARITY_EVEN,
    MCAL_USART_PARITY_ODD
} mcal_usart_parity_t;

typedef enum
{
    MCAL_USART_HWFLOW_NONE = 0U,
    MCAL_USART_HWFLOW_RTS,
    MCAL_USART_HWFLOW_CTS,
    MCAL_USART_HWFLOW_RTS_CTS
} mcal_usart_hwflow_t;

typedef enum
{
    MCAL_USART_OVERSAMPLING_16 = 0U,
    MCAL_USART_OVERSAMPLING_8
} mcal_usart_oversampling_t;

typedef enum
{
    MCAL_USART_SYNC_CLOCK_DISABLE = 0U,
    MCAL_USART_SYNC_CLOCK_ENABLE
} mcal_usart_sync_clock_t;

typedef enum
{
    MCAL_USART_CLOCK_POLARITY_LOW = 0U,
    MCAL_USART_CLOCK_POLARITY_HIGH
} mcal_usart_clock_polarity_t;

typedef enum
{
    MCAL_USART_CLOCK_PHASE_1EDGE = 0U,
    MCAL_USART_CLOCK_PHASE_2EDGE
} mcal_usart_clock_phase_t;

typedef enum
{
    MCAL_USART_LASTBITCLOCK_DISABLE = 0U,
    MCAL_USART_LASTBITCLOCK_ENABLE
} mcal_usart_lastbitclock_t;

typedef enum
{
    MCAL_USART_LIN_DISABLE = 0U,
    MCAL_USART_LIN_ENABLE
} mcal_usart_lin_t;

typedef enum
{
    MCAL_USART_HALFDUPLEX_DISABLE = 0U,
    MCAL_USART_HALFDUPLEX_ENABLE
} mcal_usart_halfduplex_t;

typedef enum
{
    MCAL_USART_IRDA_DISABLE = 0U,
    MCAL_USART_IRDA_ENABLE
} mcal_usart_irda_t;

typedef enum
{
    MCAL_USART_IRDA_NORMAL = 0U,
    MCAL_USART_IRDA_LOW_POWER
} mcal_usart_irda_power_t;

typedef enum
{
    MCAL_USART_SMARTCARD_DISABLE = 0U,
    MCAL_USART_SMARTCARD_ENABLE
} mcal_usart_smartcard_t;

typedef enum
{
    MCAL_USART_DMA_RX_DISABLE = 0U,
    MCAL_USART_DMA_RX_ENABLE
} mcal_usart_dma_rx_t;

typedef enum
{
    MCAL_USART_DMA_TX_DISABLE = 0U,
    MCAL_USART_DMA_TX_ENABLE
} mcal_usart_dma_tx_t;

typedef enum
{
    MCAL_USART_ONEBIT_DISABLE = 0U,
    MCAL_USART_ONEBIT_ENABLE
} mcal_usart_onebit_t;

typedef enum
{
    MCAL_USART_FLAG_PE = MCAL_USART_SR_PE_POS,
    MCAL_USART_FLAG_FE = MCAL_USART_SR_FE_POS,
    MCAL_USART_FLAG_NF = MCAL_USART_SR_NF_POS,
    MCAL_USART_FLAG_ORE = MCAL_USART_SR_ORE_POS,
    MCAL_USART_FLAG_IDLE = MCAL_USART_SR_IDLE_POS,
    MCAL_USART_FLAG_RXNE = MCAL_USART_SR_RXNE_POS,
    MCAL_USART_FLAG_TC = MCAL_USART_SR_TC_POS,
    MCAL_USART_FLAG_TXE = MCAL_USART_SR_TXE_POS,
    MCAL_USART_FLAG_LBD = MCAL_USART_SR_LBD_POS,
    MCAL_USART_FLAG_CTS = MCAL_USART_SR_CTS_POS
} mcal_usart_flag_t;

typedef enum
{
    MCAL_USART_INTERRUPT_PE = 0U,
    MCAL_USART_INTERRUPT_TXE,
    MCAL_USART_INTERRUPT_TC,
    MCAL_USART_INTERRUPT_RXNE,
    MCAL_USART_INTERRUPT_IDLE,
    MCAL_USART_INTERRUPT_LBD,
    MCAL_USART_INTERRUPT_CTS,
    MCAL_USART_INTERRUPT_ERROR
} mcal_usart_interrupt_t;

typedef enum
{
    MCAL_USART_EVENT_TXE = 0U,
    MCAL_USART_EVENT_TC,
    MCAL_USART_EVENT_RXNE,
    MCAL_USART_EVENT_IDLE,
    MCAL_USART_EVENT_PE,
    MCAL_USART_EVENT_LBD,
    MCAL_USART_EVENT_CTS,
    MCAL_USART_EVENT_ERROR
} mcal_usart_event_t;

typedef void (*mcal_usart_callback_t)(USART_RegDef_t *usart, mcal_usart_event_t event);

typedef struct
{
    mcal_usart_mode_t mode;
    uint32 baud_rate;
    uint32 pclk_hz;

    mcal_usart_wordlength_t word_length;
    mcal_usart_stopbits_t stop_bits;
    mcal_usart_parity_t parity;
    mcal_usart_hwflow_t hw_flow_control;
    mcal_usart_oversampling_t oversampling;

    mcal_usart_sync_clock_t sync_clock;
    mcal_usart_clock_polarity_t clock_polarity;
    mcal_usart_clock_phase_t clock_phase;
    mcal_usart_lastbitclock_t last_bit_clock;

    mcal_usart_lin_t lin_mode;
    mcal_usart_halfduplex_t half_duplex;
    mcal_usart_irda_t irda_mode;
    mcal_usart_irda_power_t irda_power;
    mcal_usart_smartcard_t smartcard_mode;

    mcal_usart_dma_rx_t dma_rx;
    mcal_usart_dma_tx_t dma_tx;
    mcal_usart_onebit_t one_bit_sampling;
} mcal_usart_config_t;

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
mcal_usart_status_t MCAL_USART_Init(USART_RegDef_t *usart, const mcal_usart_config_t *config);
void MCAL_USART_DeInit(USART_RegDef_t *usart);
void MCAL_USART_Enable(USART_RegDef_t *usart);
void MCAL_USART_Disable(USART_RegDef_t *usart);

mcal_usart_status_t MCAL_USART_SetBaudRate(USART_RegDef_t *usart,
                                           uint32 pclk_hz,
                                           uint32 baud_rate,
                                           mcal_usart_oversampling_t oversampling);

mcal_usart_status_t MCAL_USART_SendData(USART_RegDef_t *usart, const uint8 *data, uint16 length);
mcal_usart_status_t MCAL_USART_ReceiveData(USART_RegDef_t *usart, uint8 *data, uint16 length);
mcal_usart_status_t MCAL_USART_SendString(USART_RegDef_t *usart, const uint8 *string);

uint8 MCAL_USART_GetFlagStatus(USART_RegDef_t *usart, mcal_usart_flag_t flag);
void MCAL_USART_ClearFlag(USART_RegDef_t *usart, mcal_usart_flag_t flag);

void MCAL_USART_EnableInterrupt(USART_RegDef_t *usart, mcal_usart_interrupt_t interrupt_source);
void MCAL_USART_DisableInterrupt(USART_RegDef_t *usart, mcal_usart_interrupt_t interrupt_source);
mcal_usart_status_t MCAL_USART_SetCallback(USART_RegDef_t *usart, mcal_usart_callback_t callback);
void MCAL_USART_IRQHandler(USART_RegDef_t *usart);

#endif
