
#ifndef INC_SX1262_H_
#define INC_SX1262_H_

#include "main.h"
#include <stdint.h>

/* GetStatus (datasheet 13.5.1) */
#define SX1262_OP_GET_STATUS 0xC0
#define SX1262_OP_NOP        0x00

/* SetPacketType (datasheet 13.4.2) */
#define SX1262_OP_SET_PACKET_TYPE 0x8A
#define SX1262_PACKET_TYPE_GFSK   0x00
#define SX1262_PACKET_TYPE_LORA   0x01

/* SetRfFrequency (datasheet 13.4.1) */
#define SX1262_OP_SET_RF_FREQUENCY 0x86
#define SX1262_XTAL_HZ              32000000UL

/* SetModulationParams (datasheet 13.4.5) */
#define SX1262_OP_SET_MODULATION_PARAMS 0x8B

/* LoRa ModParam1 - SF (datasheet Table 13-47) */
#define SX1262_LORA_SF5  0x05
#define SX1262_LORA_SF6  0x06
#define SX1262_LORA_SF7  0x07
#define SX1262_LORA_SF8  0x08
#define SX1262_LORA_SF9  0x09
#define SX1262_LORA_SF10 0x0A
#define SX1262_LORA_SF11 0x0B
#define SX1262_LORA_SF12 0x0C

/* LoRa ModParam2 - BW (datasheet Table 13-48) */
#define SX1262_LORA_BW_7   0x00
#define SX1262_LORA_BW_10  0x08
#define SX1262_LORA_BW_15  0x01
#define SX1262_LORA_BW_20  0x09
#define SX1262_LORA_BW_31  0x02
#define SX1262_LORA_BW_41  0x0A
#define SX1262_LORA_BW_62  0x03
#define SX1262_LORA_BW_125 0x04
#define SX1262_LORA_BW_250 0x05
#define SX1262_LORA_BW_500 0x06

/* LoRa ModParam3 - CR (datasheet Table 13-49) */
#define SX1262_LORA_CR_4_5 0x01
#define SX1262_LORA_CR_4_6 0x02
#define SX1262_LORA_CR_4_7 0x03
#define SX1262_LORA_CR_4_8 0x04

/* LoRa ModParam4 - LowDataRateOptimize (datasheet Table 13-50) */
#define SX1262_LORA_LDRO_OFF 0x00
#define SX1262_LORA_LDRO_ON  0x01

#define SX1262_SPI_TIMEOUT_MS  100
#define SX1262_BUSY_TIMEOUT_MS 1000

typedef struct {
	SPI_HandleTypeDef *port;
	GPIO_PIN_DEF(cs_port, cs_pin);
	GPIO_PIN_DEF(reset_port, reset_pin);
	GPIO_PIN_DEF(busy_port, busy_pin);
} sx1262_t;

/**
 * sx1262_init
 * @param p - Pointer to an sx1262_t struct to initialize
 * @param in_port - SPI handle the SX1262 is wired to
 * @param cs_port, cs_pin - NSS (chip select) GPIO, active low
 * @param reset_port, reset_pin - NRESET GPIO, active low
 * @param busy_port, busy_pin - BUSY GPIO (input)
 *
 * Stores the SPI handle and GPIO pins used to talk to the SX1262. Does not
 * touch any pins or perform any SPI transactions.
 */
void sx1262_init(
	sx1262_t *p,
	SPI_HandleTypeDef *in_port,
	GPIO_TypeDef *cs_port, uint16_t cs_pin,
	GPIO_TypeDef *reset_port, uint16_t reset_pin,
	GPIO_TypeDef *busy_port, uint16_t busy_pin
);

/**
 * sx1262_wait_busy
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Polls the BUSY line until it reads low, or SX1262_BUSY_TIMEOUT_MS
 * elapses. The SX1262 drives BUSY high while processing a command and
 * while booting after reset; SPI transactions must not be started while
 * BUSY is high.
 *
 * @return HAL_OK once BUSY is low, or HAL_TIMEOUT if it does not clear in
 *         time.
 */
HAL_StatusTypeDef sx1262_wait_busy(sx1262_t *p);

/**
 * sx1262_reset
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Pulses NRESET low then high, then waits for BUSY to clear as the SX1262
 * completes its boot sequence.
 *
 * @return HAL_OK once the SX1262 is ready, or HAL_TIMEOUT if BUSY does not
 *         clear in time.
 */
HAL_StatusTypeDef sx1262_reset(sx1262_t *p);

/**
 * sx1262_get_status
 * @param p - Pointer to an initialized sx1262_t struct
 * @param out_status - Receives the status byte returned by the SX1262
 *
 * Sends the GetStatus (0xC0) command over SPI and reads back the status
 * byte. Bench test for basic SPI framing, NSS and BUSY handling.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_get_status(sx1262_t *p, uint8_t *out_status);

/**
 * sx1262_set_packet_type
 * @param p - Pointer to an initialized sx1262_t struct
 * @param packet_type - SX1262_PACKET_TYPE_GFSK or SX1262_PACKET_TYPE_LORA
 *
 * Sends the SetPacketType (0x8A) command. Per the datasheet this must be
 * the first command issued before going to Rx or Tx and before setting
 * the RF frequency, modulation or packet parameters.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_packet_type(sx1262_t *p, uint8_t packet_type);

/**
 * sx1262_set_rf_frequency
 * @param p - Pointer to an initialized sx1262_t struct
 * @param freq_hz - Target RF frequency in Hz
 *
 * Sends the SetRfFrequency (0x86) command. freq_hz is converted to the
 * SX1262's 32-bit RF frequency register value using
 * RFfreq = freq_hz * 2^25 / SX1262_XTAL_HZ.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_rf_frequency(sx1262_t *p, uint32_t freq_hz);

/**
 * sx1262_set_modulation_params_lora
 * @param p - Pointer to an initialized sx1262_t struct
 * @param sf - Spreading factor, one of SX1262_LORA_SF5..SX1262_LORA_SF12
 * @param bw - Bandwidth, one of SX1262_LORA_BW_*
 * @param cr - Coding rate, one of SX1262_LORA_CR_4_5..SX1262_LORA_CR_4_8
 * @param ldro - Low data rate optimize, SX1262_LORA_LDRO_OFF or _ON
 *
 * Sends the SetModulationParams (0x8B) command for LoRa packet type.
 * ModParam5-8 are reserved and sent as 0x00. Only valid after
 * sx1262_set_packet_type(SX1262_PACKET_TYPE_LORA).
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_modulation_params_lora(sx1262_t *p, uint8_t sf, uint8_t bw, uint8_t cr, uint8_t ldro);

#endif /* INC_SX1262_H_ */
