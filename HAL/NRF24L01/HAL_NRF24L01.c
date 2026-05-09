/*
 * HAL_NRF24L01.c
 *
 *  Created on: May 1, 2026
 *  Author: Ahmed Basem
 */

/* ================================================================ */
/* =========================== Includes =========================== */
/* ================================================================ */
#include "HAL_NRF24L01.h"
#include "MCAL_RCC.h"
#include "UTILS.h"

/* ================================================================ */
/* ===================== Static Declarations ====================== */
/* ================================================================ */
static void HAL_NRF24_DelayCycles(volatile uint32 cycles);
static void HAL_NRF24_DelayMs(uint32 milliseconds);
static hal_nrf24_status_t HAL_NRF24_InitGPIOClock(GPIO_RegDef_t *gpio_port);
static hal_nrf24_status_t HAL_NRF24_InitPin(const hal_nrf24_t *nrf24, const hal_nrf24_pin_t *pin);
static hal_nrf24_status_t HAL_NRF24_WritePinValue(const hal_nrf24_pin_t *pin, mcal_gpio_pin_state_t value);
static hal_nrf24_status_t HAL_NRF24_SPI_Transfer(hal_nrf24_t *nrf24, const uint8 *tx_data, uint8 *rx_data, uint8 len);

/* ================================================================ */
/* ================= Static Function Definitions ================== */
/* ================================================================ */
static void HAL_NRF24_DelayCycles(volatile uint32 cycles)
{
    while (cycles > 0U)
    {
        __asm volatile ("nop");
        cycles--;
    }
}

static void HAL_NRF24_DelayMs(uint32 milliseconds)
{
    while (milliseconds > 0U)
    {
        HAL_NRF24_DelayCycles(4000UL);
        milliseconds--;
    }
}

static hal_nrf24_status_t HAL_NRF24_InitGPIOClock(GPIO_RegDef_t *gpio_port)
{
    if (gpio_port == GPIOA)
    {
        MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_A);
        return HAL_NRF24_STATUS_OK;
    }

    if (gpio_port == GPIOB)
    {
        MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_B);
        return HAL_NRF24_STATUS_OK;
    }

    if (gpio_port == GPIOC)
    {
        MCAL_RCC_EnableGPIOClock(MCAL_RCC_GPIO_PORT_C);
        return HAL_NRF24_STATUS_OK;
    }

    return HAL_NRF24_STATUS_INVALID_PARAM;
}

static hal_nrf24_status_t HAL_NRF24_InitPin(const hal_nrf24_t *nrf24, const hal_nrf24_pin_t *pin)
{
    hal_nrf24_status_t status;

    if ((nrf24 == NULL) || (pin == NULL) || (pin->port == NULL) || (pin->pin > MCAL_GPIO_PIN_15))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    status = HAL_NRF24_InitGPIOClock(pin->port);
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    MCAL_GPIO_Init(pin->port, pin->pin, MCAL_GPIO_MODE_OUTPUT_PP, MCAL_GPIO_SPEED_VERY_HIGH);
    MCAL_GPIO_WritePin(pin->port, pin->pin, GPIO_PIN_RESET);

    return HAL_NRF24_STATUS_OK;
}

static hal_nrf24_status_t HAL_NRF24_WritePinValue(const hal_nrf24_pin_t *pin, mcal_gpio_pin_state_t value)
{
    if ((pin == NULL) || (pin->port == NULL) || (pin->pin > MCAL_GPIO_PIN_15))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    MCAL_GPIO_WritePin(pin->port, pin->pin, value);
    return HAL_NRF24_STATUS_OK;
}

static hal_nrf24_status_t HAL_NRF24_SPI_Transfer(hal_nrf24_t *nrf24, const uint8 *tx_data, uint8 *rx_data, uint8 len)
{
	uint8 index;
	uint8 dummy_rx;
	mcal_spi_status_t spi_status;

    if ((nrf24 == NULL) || (len == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    if (tx_data == NULL)
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

	MCAL_SPI_ClearOVRFlag(nrf24->spi_port);

	/* CSN Low */
	HAL_NRF24_WritePinValue(&nrf24->csn_pin, GPIO_PIN_RESET);
	HAL_NRF24_DelayMs(1U);

	/* Transfer data */
	for (index = 0U; index < len; index++)
	{
		if (rx_data != NULL)
		{
			spi_status = MCAL_SPI_TransmitReceive(nrf24->spi_port, (uint8 *)&tx_data[index], &rx_data[index], 1U);
		}
		else
		{
			spi_status = MCAL_SPI_TransmitReceive(nrf24->spi_port, (uint8 *)&tx_data[index], &dummy_rx, 1U);
		}

		if (spi_status != MCAL_SPI_STATUS_OK)
		{
			HAL_NRF24_WritePinValue(&nrf24->csn_pin, GPIO_PIN_SET);
			return HAL_NRF24_STATUS_ERROR;
		}
	}

    HAL_NRF24_DelayMs(1U);
    /* CSN High */
    HAL_NRF24_WritePinValue(&nrf24->csn_pin, GPIO_PIN_SET);

    return HAL_NRF24_STATUS_OK;
}

/* ================================================================ */
/* ============================ APIs ============================== */
/* ================================================================ */
hal_nrf24_status_t HAL_NRF24_Init(hal_nrf24_t *nrf24, const hal_nrf24_config_t *config)
{
    hal_nrf24_status_t status;
    uint8 reg_value;

    if ((nrf24 == NULL) || (config == NULL))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    /* Initialize structure */
    nrf24->spi_port = config->spi_port;
    nrf24->ce_pin = config->ce_pin;
    nrf24->csn_pin = config->csn_pin;
    nrf24->irq_pin = config->irq_pin;
    nrf24->channel = config->channel;
    nrf24->payload_width = config->payload_width;
    nrf24->is_initialized = 1U;

    /* Initialize GPIO pins */
    status = HAL_NRF24_InitPin(nrf24, &nrf24->ce_pin);
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    status = HAL_NRF24_InitPin(nrf24, &nrf24->csn_pin);
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    status = HAL_NRF24_InitPin(nrf24, &nrf24->irq_pin);
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    /* IRQ is input from NRF24, keep CE low and CSN high (idle) */
    MCAL_GPIO_Init(nrf24->irq_pin.port,
                   nrf24->irq_pin.pin,
                   MCAL_GPIO_MODE_INPUT_PULLUP,
                   MCAL_GPIO_SPEED_LOW);
    MCAL_GPIO_WritePin(nrf24->ce_pin.port, nrf24->ce_pin.pin, GPIO_PIN_RESET);
    MCAL_GPIO_WritePin(nrf24->csn_pin.port, nrf24->csn_pin.pin, GPIO_PIN_SET);

    /* Initialize SPI */
    HAL_NRF24_DelayMs(100U);

    /* Read status to verify SPI works */
    status = HAL_NRF24_GetStatus(nrf24, &reg_value);
    if (status != HAL_NRF24_STATUS_OK)
    {
        nrf24->is_initialized = 0U;
        return status;
    }

    /* Power down initially */
    status = HAL_NRF24_PowerDown(nrf24);
    if (status != HAL_NRF24_STATUS_OK)
    {
        nrf24->is_initialized = 0U;
        return status;
    }

    /* Configure radio */
    status = HAL_NRF24_SetChannel(nrf24, config->channel);
    if (status != HAL_NRF24_STATUS_OK)
    {
        nrf24->is_initialized = 0U;
        return status;
    }

    status = HAL_NRF24_SetPayloadWidth(nrf24, config->payload_width);
    if (status != HAL_NRF24_STATUS_OK)
    {
        nrf24->is_initialized = 0U;
        return status;
    }

    /* Enable RX on pipe 0 */
    status = HAL_NRF24_WriteReg(nrf24, NRF24_REG_EN_RXADDR, 0x01U);
    if (status != HAL_NRF24_STATUS_OK)
    {
        nrf24->is_initialized = 0U;
        return status;
    }

    /* Set address width to 5 bytes */
    status = HAL_NRF24_WriteReg(nrf24, NRF24_REG_SETUP_AW, 0x03U);
    if (status != HAL_NRF24_STATUS_OK)
    {
        nrf24->is_initialized = 0U;
        return status;
    }

    /* Clear all IRQ flags */
    status = HAL_NRF24_ClearIRQFlags(nrf24);
    if (status != HAL_NRF24_STATUS_OK)
    {
        nrf24->is_initialized = 0U;
        return status;
    }

    return HAL_NRF24_STATUS_OK;
}

hal_nrf24_status_t HAL_NRF24_DeInit(hal_nrf24_t *nrf24)
{
    if (nrf24 == NULL)
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    HAL_NRF24_PowerDown(nrf24);
    nrf24->is_initialized = 0U;

    return HAL_NRF24_STATUS_OK;
}

hal_nrf24_status_t HAL_NRF24_ReadReg(hal_nrf24_t *nrf24, uint8 reg, uint8 *value)
{
    uint8 tx_data[2];
    uint8 rx_data[2];
    hal_nrf24_status_t status;

    if ((nrf24 == NULL) || (value == NULL) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    tx_data[0] = NRF24_CMD_R_REGISTER(reg);
    tx_data[1] = NRF24_CMD_NOP;

    status = HAL_NRF24_SPI_Transfer(nrf24, tx_data, rx_data, 2U);
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    *value = rx_data[1];

    return HAL_NRF24_STATUS_OK;
}

hal_nrf24_status_t HAL_NRF24_WriteReg(hal_nrf24_t *nrf24, uint8 reg, uint8 value)
{
    uint8 tx_data[2];

    if ((nrf24 == NULL) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    tx_data[0] = NRF24_CMD_W_REGISTER(reg);
    tx_data[1] = value;

    return HAL_NRF24_SPI_Transfer(nrf24, tx_data, NULL, 2U);
}

hal_nrf24_status_t HAL_NRF24_ReadRegBytes(hal_nrf24_t *nrf24, uint8 reg, uint8 *buffer, uint8 len)
{
    uint8 tx_data[32];
    uint8 rx_data[32];
    uint8 index;
    hal_nrf24_status_t status;

    if ((nrf24 == NULL) || (buffer == NULL) || (len == 0U) || (len > 31U) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    tx_data[0] = NRF24_CMD_R_REGISTER(reg);
    for (index = 1U; index <= len; index++)
    {
        tx_data[index] = NRF24_CMD_NOP;
    }

    status = HAL_NRF24_SPI_Transfer(nrf24, tx_data, rx_data, (uint8)(len + 1U));
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    for (index = 0U; index < len; index++)
    {
        buffer[index] = rx_data[index + 1U];
    }

    return HAL_NRF24_STATUS_OK;
}

hal_nrf24_status_t HAL_NRF24_WriteRegBytes(hal_nrf24_t *nrf24, uint8 reg, const uint8 *buffer, uint8 len)
{
    uint8 tx_data[32];
    uint8 index;

    if ((nrf24 == NULL) || (buffer == NULL) || (len == 0U) || (len > 31U) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    tx_data[0] = NRF24_CMD_W_REGISTER(reg);
    for (index = 0U; index < len; index++)
    {
        tx_data[index + 1U] = buffer[index];
    }

    return HAL_NRF24_SPI_Transfer(nrf24, tx_data, NULL, (uint8)(len + 1U));
}

hal_nrf24_status_t HAL_NRF24_PowerUp(hal_nrf24_t *nrf24)
{
    uint8 config;
    hal_nrf24_status_t status;

    if ((nrf24 == NULL) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    status = HAL_NRF24_ReadReg(nrf24, NRF24_REG_CONFIG, &config);
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    config |= NRF24_CONFIG_PWR_UP;

    return HAL_NRF24_WriteReg(nrf24, NRF24_REG_CONFIG, config);
}

hal_nrf24_status_t HAL_NRF24_PowerDown(hal_nrf24_t *nrf24)
{
    uint8 config;
    hal_nrf24_status_t status;

    if ((nrf24 == NULL) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    status = HAL_NRF24_ReadReg(nrf24, NRF24_REG_CONFIG, &config);
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    config &= ~NRF24_CONFIG_PWR_UP;

    return HAL_NRF24_WriteReg(nrf24, NRF24_REG_CONFIG, config);
}

hal_nrf24_status_t HAL_NRF24_SetTXMode(hal_nrf24_t *nrf24)
{
    uint8 config;
    hal_nrf24_status_t status;

    if ((nrf24 == NULL) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    status = HAL_NRF24_ReadReg(nrf24, NRF24_REG_CONFIG, &config);
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    config &= ~NRF24_CONFIG_PRIM_RX;
    return HAL_NRF24_WriteReg(nrf24, NRF24_REG_CONFIG, config);
}

hal_nrf24_status_t HAL_NRF24_SetRXMode(hal_nrf24_t *nrf24)
{
    uint8 config;
    hal_nrf24_status_t status;

    if ((nrf24 == NULL) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    status = HAL_NRF24_ReadReg(nrf24, NRF24_REG_CONFIG, &config);
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    config |= NRF24_CONFIG_PRIM_RX;
    return HAL_NRF24_WriteReg(nrf24, NRF24_REG_CONFIG, config);
}

hal_nrf24_status_t HAL_NRF24_TransmitPayload(hal_nrf24_t *nrf24, const uint8 *data, uint8 len)
{
    uint8 tx_data[33];
    uint8 index;
    hal_nrf24_status_t status;

    if ((nrf24 == NULL) || (data == NULL) || (len == 0U) || (len > 32U) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    tx_data[0] = NRF24_CMD_W_TX_PAYLOAD;
    for (index = 0U; index < len; index++)
    {
        tx_data[index + 1U] = data[index];
    }

    status = HAL_NRF24_SPI_Transfer(nrf24, tx_data, NULL, (uint8)(len + 1U));
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    /* Pulse CE after payload is loaded */
    HAL_NRF24_WritePinValue(&nrf24->ce_pin, GPIO_PIN_SET);
    HAL_NRF24_DelayMs(1U);
    HAL_NRF24_WritePinValue(&nrf24->ce_pin, GPIO_PIN_RESET);

    return HAL_NRF24_STATUS_OK;
}

hal_nrf24_status_t HAL_NRF24_ReceivePayload(hal_nrf24_t *nrf24, uint8 *data, uint8 *len)
{
    uint8 tx_data[33];
    uint8 rx_data[33];
    uint8 fifo_status;
    uint8 index;
    hal_nrf24_status_t status;

    if ((nrf24 == NULL) || (data == NULL) || (len == NULL) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    status = HAL_NRF24_ReadReg(nrf24, NRF24_REG_FIFO_STATUS, &fifo_status);
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    if ((fifo_status & NRF24_FIFO_RX_EMPTY) != 0U)
    {
        *len = 0U;
        return HAL_NRF24_STATUS_OK;
    }

    tx_data[0] = NRF24_CMD_R_RX_PAYLOAD;
    for (index = 1U; index <= nrf24->payload_width; index++)
    {
        tx_data[index] = NRF24_CMD_NOP;
    }

    status = HAL_NRF24_SPI_Transfer(nrf24, tx_data, rx_data, (uint8)(nrf24->payload_width + 1U));
    if (status != HAL_NRF24_STATUS_OK)
    {
        return status;
    }

    for (index = 0U; index < nrf24->payload_width; index++)
    {
        data[index] = rx_data[index + 1U];
    }

    *len = nrf24->payload_width;

    return HAL_NRF24_STATUS_OK;
}

hal_nrf24_status_t HAL_NRF24_GetStatus(hal_nrf24_t *nrf24, uint8 *status)
{
    uint8 tx_data;
    uint8 rx_data;

    if ((nrf24 == NULL) || (status == NULL))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    tx_data = NRF24_CMD_NOP;
    HAL_NRF24_WritePinValue(&nrf24->csn_pin, GPIO_PIN_RESET);
    HAL_NRF24_DelayMs(1U);

    MCAL_SPI_TransmitReceive(nrf24->spi_port, &tx_data, &rx_data, 1U);

    HAL_NRF24_DelayMs(1U);
    HAL_NRF24_WritePinValue(&nrf24->csn_pin, GPIO_PIN_SET);

    *status = rx_data;

    return HAL_NRF24_STATUS_OK;
}

hal_nrf24_status_t HAL_NRF24_ClearIRQFlags(hal_nrf24_t *nrf24)
{
    if ((nrf24 == NULL) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    return HAL_NRF24_WriteReg(nrf24, NRF24_REG_STATUS, 
                              (uint8)(NRF24_STATUS_RX_DR | NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT));
}

hal_nrf24_status_t HAL_NRF24_SetChannel(hal_nrf24_t *nrf24, uint8 channel)
{
    if ((nrf24 == NULL) || (channel > 125U) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    nrf24->channel = channel;
    return HAL_NRF24_WriteReg(nrf24, NRF24_REG_RF_CH, channel);
}

hal_nrf24_status_t HAL_NRF24_SetPayloadWidth(hal_nrf24_t *nrf24, uint8 width)
{
    if ((nrf24 == NULL) || (width == 0U) || (width > 32U) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    nrf24->payload_width = width;
    return HAL_NRF24_WriteReg(nrf24, NRF24_REG_RX_PW_P0, width);
}

hal_nrf24_status_t HAL_NRF24_SetAddress(hal_nrf24_t *nrf24, const uint8 *address, uint8 address_len)
{
    if ((nrf24 == NULL) || (address == NULL) || (address_len == 0U) || (address_len > 5U) || (nrf24->is_initialized == 0U))
    {
        return HAL_NRF24_STATUS_INVALID_PARAM;
    }

    return HAL_NRF24_WriteRegBytes(nrf24, NRF24_REG_RX_ADDR_P0, address, address_len);
}
