/*
 * HAL_NRF24L01.h
 *
 *  Created on: May 1, 2026
 *  Author: Ahmed Basem
 */

#ifndef HAL_NRF24L01_H
#define HAL_NRF24L01_H

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "PLATFORM_TYPES.h"
#include "mcal_gpio.h"
#include "MCAL_SPI.h"

/* ================================================================ */
/* ======================= NRF24L01 Registers ==================== */
/* ================================================================ */
#define NRF24_REG_CONFIG                        (0x00U)
#define NRF24_REG_EN_AA                         (0x01U)
#define NRF24_REG_EN_RXADDR                     (0x02U)
#define NRF24_REG_SETUP_AW                      (0x03U)
#define NRF24_REG_SETUP_RETR                    (0x04U)
#define NRF24_REG_RF_CH                         (0x05U)
#define NRF24_REG_RF_SETUP                      (0x06U)
#define NRF24_REG_STATUS                        (0x07U)
#define NRF24_REG_OBSERVE_TX                    (0x08U)
#define NRF24_REG_RPD                           (0x09U)
#define NRF24_REG_RX_ADDR_P0                    (0x0AU)
#define NRF24_REG_RX_ADDR_P1                    (0x0BU)
#define NRF24_REG_RX_ADDR_P2                    (0x0CU)
#define NRF24_REG_RX_ADDR_P3                    (0x0DU)
#define NRF24_REG_RX_ADDR_P4                    (0x0EU)
#define NRF24_REG_RX_ADDR_P5                    (0x0FU)
#define NRF24_REG_TX_ADDR                       (0x10U)
#define NRF24_REG_RX_PW_P0                      (0x11U)
#define NRF24_REG_RX_PW_P1                      (0x12U)
#define NRF24_REG_RX_PW_P2                      (0x13U)
#define NRF24_REG_RX_PW_P3                      (0x14U)
#define NRF24_REG_RX_PW_P4                      (0x15U)
#define NRF24_REG_RX_PW_P5                      (0x16U)
#define NRF24_REG_FIFO_STATUS                   (0x17U)
#define NRF24_REG_DYNPD                         (0x1CU)
#define NRF24_REG_FEATURE                       (0x1DU)

/* ================================================================ */
/* ===================== NRF24L01 Commands ======================= */
/* ================================================================ */
#define NRF24_CMD_R_REGISTER(reg)               (0x00U | (reg))
#define NRF24_CMD_W_REGISTER(reg)               (0x20U | (reg))
#define NRF24_CMD_R_RX_PAYLOAD                  (0x61U)
#define NRF24_CMD_W_TX_PAYLOAD                  (0xA0U)
#define NRF24_CMD_FLUSH_TX                      (0xE1U)
#define NRF24_CMD_FLUSH_RX                      (0xE2U)
#define NRF24_CMD_REUSE_TX_PL                   (0xE3U)
#define NRF24_CMD_R_RX_PL_WID                   (0x60U)
#define NRF24_CMD_W_ACK_PAYLOAD(pipe)           (0xA8U | (pipe))
#define NRF24_CMD_W_TX_PAYLOAD_NOACK            (0xB0U)
#define NRF24_CMD_NOP                           (0xFFU)

/* ================================================================ */
/* =============== NRF24L01 Register Bit Flags =================== */
/* ================================================================ */
#define NRF24_CONFIG_MASK_RX_DR                 (0x40U)
#define NRF24_CONFIG_MASK_TX_DS                 (0x20U)
#define NRF24_CONFIG_MASK_MAX_RT                (0x10U)
#define NRF24_CONFIG_EN_CRC                     (0x08U)
#define NRF24_CONFIG_CRCO                       (0x04U)
#define NRF24_CONFIG_PWR_UP                     (0x02U)
#define NRF24_CONFIG_PRIM_RX                    (0x01U)

#define NRF24_STATUS_RX_DR                      (0x40U)
#define NRF24_STATUS_TX_DS                      (0x20U)
#define NRF24_STATUS_MAX_RT                     (0x10U)
#define NRF24_STATUS_RX_P_NO                    (0x0EU)
#define NRF24_STATUS_TX_FULL                    (0x01U)

#define NRF24_FIFO_RX_EMPTY                     (0x01U)
#define NRF24_FIFO_RX_FULL                      (0x02U)
#define NRF24_FIFO_TX_EMPTY                     (0x10U)
#define NRF24_FIFO_TX_FULL                      (0x20U)

/* ================================================================ */
/* ==================== Data Structure & Types =================== */
/* ================================================================ */
typedef struct
{
    GPIO_RegDef_t *port;
    mcal_gpio_pin_t pin;
} hal_nrf24_pin_t;

typedef enum
{
    HAL_NRF24_STATUS_OK = 0U,
    HAL_NRF24_STATUS_ERROR,
    HAL_NRF24_STATUS_INVALID_PARAM,
    HAL_NRF24_STATUS_TIMEOUT
} hal_nrf24_status_t;

typedef struct
{
    SPI_RegDef_t *spi_port;
    hal_nrf24_pin_t ce_pin;
    hal_nrf24_pin_t csn_pin;
    hal_nrf24_pin_t irq_pin;
    
    uint8 channel;
    uint8 payload_width;
    uint8 address_width;
    uint8 power_level;
    uint8 data_rate;
} hal_nrf24_config_t;

typedef struct
{
    SPI_RegDef_t *spi_port;
    hal_nrf24_pin_t ce_pin;
    hal_nrf24_pin_t csn_pin;
    hal_nrf24_pin_t irq_pin;
    
    uint8 is_initialized;
    uint8 channel;
    uint8 payload_width;
} hal_nrf24_t;

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
hal_nrf24_status_t HAL_NRF24_Init(hal_nrf24_t *nrf24, const hal_nrf24_config_t *config);
hal_nrf24_status_t HAL_NRF24_DeInit(hal_nrf24_t *nrf24);

hal_nrf24_status_t HAL_NRF24_ReadReg(hal_nrf24_t *nrf24, uint8 reg, uint8 *value);
hal_nrf24_status_t HAL_NRF24_WriteReg(hal_nrf24_t *nrf24, uint8 reg, uint8 value);
hal_nrf24_status_t HAL_NRF24_ReadRegBytes(hal_nrf24_t *nrf24, uint8 reg, uint8 *buffer, uint8 len);
hal_nrf24_status_t HAL_NRF24_WriteRegBytes(hal_nrf24_t *nrf24, uint8 reg, const uint8 *buffer, uint8 len);

hal_nrf24_status_t HAL_NRF24_PowerUp(hal_nrf24_t *nrf24);
hal_nrf24_status_t HAL_NRF24_PowerDown(hal_nrf24_t *nrf24);

hal_nrf24_status_t HAL_NRF24_SetTXMode(hal_nrf24_t *nrf24);
hal_nrf24_status_t HAL_NRF24_SetRXMode(hal_nrf24_t *nrf24);

hal_nrf24_status_t HAL_NRF24_TransmitPayload(hal_nrf24_t *nrf24, const uint8 *data, uint8 len);
hal_nrf24_status_t HAL_NRF24_ReceivePayload(hal_nrf24_t *nrf24, uint8 *data, uint8 *len);

hal_nrf24_status_t HAL_NRF24_GetStatus(hal_nrf24_t *nrf24, uint8 *status);
hal_nrf24_status_t HAL_NRF24_ClearIRQFlags(hal_nrf24_t *nrf24);

hal_nrf24_status_t HAL_NRF24_SetChannel(hal_nrf24_t *nrf24, uint8 channel);
hal_nrf24_status_t HAL_NRF24_SetPayloadWidth(hal_nrf24_t *nrf24, uint8 width);
hal_nrf24_status_t HAL_NRF24_SetAddress(hal_nrf24_t *nrf24, const uint8 *address, uint8 address_len);

#endif
