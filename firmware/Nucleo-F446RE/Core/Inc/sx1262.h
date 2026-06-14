
#ifndef INC_SX1262_H_
#define INC_SX1262_H_

#include "main.h"
#include <stdint.h>
#include <stddef.h>

/**
 * sx1262.h - SX1262 LoRa transceiver driver
 *
 * Thin wrapper around the SX1262's SPI command set (datasheet section 13),
 * covering the subset needed for basic LoRa TX/RX: reset, configuration,
 * buffer access, TX/RX triggering and IRQ status.
 *
 * Typical bring-up sequence:
 *   sx1262_init()
 *   sx1262_reset()
 *   sx1262_set_dio3_as_tcxo_ctrl()   (Waveshare Core1262-LF only)
 *   sx1262_clear_device_errors()
 *   sx1262_set_packet_type(LORA)
 *   sx1262_set_rf_frequency()
 *   sx1262_calibrate_image()         (if operating outside 902-928MHz)
 *   sx1262_set_modulation_params_lora()
 *   sx1262_set_packet_params_lora()
 *   sx1262_set_buffer_base_address()
 *   sx1262_set_pa_config() / sx1262_set_tx_params()   (TX only)
 *   sx1262_set_dio_irq_params()
 *
 * Interrupt-driven TX/RX:
 *   DIO1 can be configured via sx1262_set_dio_irq_params() to assert on
 *   TxDone/RxDone/Timeout. The expected application pattern is:
 *     1. Route DIO1 to an EXTI line and set an atomic flag from the ISR.
 *     2. From the idle loop, start a transmission with sx1262_set_tx() or
 *        start listening with sx1262_set_rx().
 *     3. When the DIO1 flag is set, call a "service" function (e.g.
 *        test_sx1262_tx_done()/test_sx1262_rx_done()) which reads
 *        GetIrqStatus, clears it, and - only if the event was a real
 *        TxDone/RxDone rather than a Timeout - invokes the registered
 *        callback below.
 *
 * Callbacks:
 *   sx1262_set_rx_done_callback() / sx1262_set_tx_done_callback() register
 *   a void(*)(sx1262_t *p) callback, invoked with the instance pointer by
 *   the corresponding service function whenever it observes a real
 *   RxDone/TxDone (never on Timeout). Both are NULL after sx1262_init();
 *   set to NULL to disable.
 */

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

/* CalibrateImage (datasheet 13.1.13 / Table 9-2) */
#define SX1262_OP_CALIBRATE_IMAGE 0x98
#define SX1262_CAL_IMG_430_440_FREQ1 0x6B
#define SX1262_CAL_IMG_430_440_FREQ2 0x6F

/* SetDIO3AsTCXOCtrl (datasheet 13.3.6 / Table 13-35) */
#define SX1262_OP_SET_DIO3_AS_TCXO_CTRL 0x97
#define SX1262_TCXO_VOLTAGE_1_6 0x00
#define SX1262_TCXO_VOLTAGE_1_7 0x01
#define SX1262_TCXO_VOLTAGE_1_8 0x02
#define SX1262_TCXO_VOLTAGE_2_2 0x03
#define SX1262_TCXO_VOLTAGE_2_4 0x04
#define SX1262_TCXO_VOLTAGE_2_7 0x05
#define SX1262_TCXO_VOLTAGE_3_0 0x06
#define SX1262_TCXO_VOLTAGE_3_3 0x07

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

/* SetPacketParams (datasheet 13.4.6) */
#define SX1262_OP_SET_PACKET_PARAMS 0x8C

/* LoRa PacketParam3 - HeaderType (datasheet Table 13-67) */
#define SX1262_LORA_HEADER_EXPLICIT 0x00
#define SX1262_LORA_HEADER_IMPLICIT 0x01

/* LoRa PacketParam5 - CRCType (datasheet Table 13-69) */
#define SX1262_LORA_CRC_OFF 0x00
#define SX1262_LORA_CRC_ON  0x01

/* LoRa PacketParam6 - InvertIQ (datasheet Table 13-70) */
#define SX1262_LORA_IQ_STANDARD 0x00
#define SX1262_LORA_IQ_INVERTED 0x01

/* SetPaConfig (datasheet 13.1.14) */
#define SX1262_OP_SET_PA_CONFIG 0x95
#define SX1262_PA_CONFIG_SX1262 0x00
#define SX1262_PA_CONFIG_SX1261 0x01

/* SetTxParams (datasheet 13.4.4 / Table 13-41) */
#define SX1262_OP_SET_TX_PARAMS 0x8E
#define SX1262_RAMP_10U   0x00
#define SX1262_RAMP_20U   0x01
#define SX1262_RAMP_40U   0x02
#define SX1262_RAMP_80U   0x03
#define SX1262_RAMP_200U  0x04
#define SX1262_RAMP_800U  0x05
#define SX1262_RAMP_1700U 0x06
#define SX1262_RAMP_3400U 0x07

/* SetBufferBaseAddress (datasheet 13.4.8) */
#define SX1262_OP_SET_BUFFER_BASE_ADDRESS 0x8F

/* WriteBuffer / ReadBuffer (datasheet 13.2.3 / 13.2.4) */
#define SX1262_OP_WRITE_BUFFER 0x0E
#define SX1262_OP_READ_BUFFER  0x1E
#define SX1262_MAX_PAYLOAD_LEN 32

/* SetTx / SetRx (datasheet 13.1.4 / 13.1.5) */
#define SX1262_OP_SET_TX 0x83
#define SX1262_OP_SET_RX 0x82
#define SX1262_RX_TX_TIMEOUT_NONE    0x000000UL
#define SX1262_RX_TIMEOUT_CONTINUOUS 0xFFFFFFUL

/* SetDioIrqParams / GetIrqStatus / ClearIrqStatus (datasheet 13.3.1 / 13.3.1 / 13.3.3 / Table 13-29) */
#define SX1262_OP_SET_DIO_IRQ_PARAMS 0x08
#define SX1262_OP_GET_IRQ_STATUS     0x12
#define SX1262_OP_CLEAR_IRQ_STATUS   0x02

#define SX1262_IRQ_TX_DONE      (1U << 0)
#define SX1262_IRQ_RX_DONE      (1U << 1)
#define SX1262_IRQ_PREAMBLE_DET (1U << 2)
#define SX1262_IRQ_HEADER_VALID (1U << 4)
#define SX1262_IRQ_HEADER_ERR   (1U << 5)
#define SX1262_IRQ_CRC_ERR      (1U << 6)
#define SX1262_IRQ_TIMEOUT      (1U << 9)
#define SX1262_IRQ_ALL          0x03FFU

/* GetPacketStatus (datasheet 13.5.3 / Table 13-79, LoRa packet status) */
#define SX1262_OP_GET_PACKET_STATUS 0x14

/* GetDeviceErrors / ClearDeviceErrors (datasheet 13.6.1 / 13.6.2 / Table 13-85) */
#define SX1262_OP_GET_DEVICE_ERRORS   0x17
#define SX1262_OP_CLEAR_DEVICE_ERRORS 0x07

#define SX1262_OP_ERR_RC64K_CALIB (1U << 0)
#define SX1262_OP_ERR_RC13M_CALIB (1U << 1)
#define SX1262_OP_ERR_PLL_CALIB   (1U << 2)
#define SX1262_OP_ERR_ADC_CALIB   (1U << 3)
#define SX1262_OP_ERR_IMG_CALIB   (1U << 4)
#define SX1262_OP_ERR_XOSC_START  (1U << 5)
#define SX1262_OP_ERR_PLL_LOCK    (1U << 6)
#define SX1262_OP_ERR_PA_RAMP     (1U << 8)

#define SX1262_SPI_TIMEOUT_MS  100
#define SX1262_BUSY_TIMEOUT_MS 1000

typedef struct sx1262_s {
	SPI_HandleTypeDef *port;
	GPIO_PIN_DEF(cs_port, cs_pin);
	GPIO_PIN_DEF(reset_port, reset_pin);
	GPIO_PIN_DEF(busy_port, busy_pin);
	void (*rx_done)(struct sx1262_s *p);
	void (*tx_done)(struct sx1262_s *p);
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
 * sx1262_set_rx_done_callback
 * @param p - Pointer to an initialized sx1262_t struct
 * @param callback - Function to call when a packet is received, or NULL
 *                    to disable
 *
 * Registers a callback to be invoked when a valid packet (RxDone) is
 * detected. The callback receives the sx1262_t instance pointer. Stored
 * in p->rx_done; NULL by default after sx1262_init().
 */
void sx1262_set_rx_done_callback(sx1262_t *p, void (*callback)(sx1262_t *p));

/**
 * sx1262_set_tx_done_callback
 * @param p - Pointer to an initialized sx1262_t struct
 * @param callback - Function to call when a transmission completes, or
 *                    NULL to disable
 *
 * Registers a callback to be invoked when TxDone is detected. The
 * callback receives the sx1262_t instance pointer. Stored in
 * p->tx_done; NULL by default after sx1262_init().
 */
void sx1262_set_tx_done_callback(sx1262_t *p, void (*callback)(sx1262_t *p));

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
 * sx1262_calibrate_image
 * @param p - Pointer to an initialized sx1262_t struct
 * @param freq1, freq2 - Calibration band bounds, e.g.
 *                        SX1262_CAL_IMG_430_440_FREQ1/FREQ2
 *
 * Sends the CalibrateImage (0x98) command. The factory-default image
 * calibration only covers 902-928MHz; operating outside that band
 * requires calibrating the corresponding band first, or SetTx/SetRx
 * will fail (chip stays in STBY_RC, cmd status = failure to execute).
 * Must be issued from STDBY_RC mode.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_calibrate_image(sx1262_t *p, uint8_t freq1, uint8_t freq2);

/**
 * sx1262_set_dio3_as_tcxo_ctrl
 * @param p - Pointer to an initialized sx1262_t struct
 * @param tcxo_voltage - One of SX1262_TCXO_VOLTAGE_*
 * @param delay - Startup delay in steps of 15.625us before XOSC_START_ERR
 *                is flagged if 32MHz has not appeared
 *
 * Sends the SetDIO3AsTCXOCtrl (0x97) command. The Waveshare Core1262-LF
 * module's 32MHz reference is a TCXO powered from DIO3 (see
 * Core1262-LF-Schematic.pdf) rather than a passive crystal on XTA/XTB -
 * without this command DIO3 never supplies the TCXO and XOSC_START_ERR
 * is set, leaving the chip unable to leave STBY_RC for TX/RX. Should be
 * issued first, before any other configuration.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_dio3_as_tcxo_ctrl(sx1262_t *p, uint8_t tcxo_voltage, uint32_t delay);

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

/**
 * sx1262_set_packet_params_lora
 * @param p - Pointer to an initialized sx1262_t struct
 * @param preamble_len - Preamble length in symbols (Table 13-66)
 * @param header_type - SX1262_LORA_HEADER_EXPLICIT or _IMPLICIT
 * @param payload_len - Payload length in bytes (Table 13-68)
 * @param crc_type - SX1262_LORA_CRC_OFF or _ON
 * @param invert_iq - SX1262_LORA_IQ_STANDARD or _INVERTED
 *
 * Sends the SetPacketParams (0x8C) command for LoRa packet type. Only
 * valid after sx1262_set_packet_type(SX1262_PACKET_TYPE_LORA). payload_len
 * is the size used for the next Tx/Rx and would normally be re-sent
 * before each transaction.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_packet_params_lora(sx1262_t *p, uint16_t preamble_len, uint8_t header_type, uint8_t payload_len, uint8_t crc_type, uint8_t invert_iq);

/**
 * sx1262_set_pa_config
 * @param p - Pointer to an initialized sx1262_t struct
 * @param pa_duty_cycle - paDutyCycle (Table 13-21)
 * @param hp_max - hpMax, PA size for the SX1262 (Table 13-21)
 * @param device_sel - SX1262_PA_CONFIG_SX1262 or _SX1261
 *
 * Sends the SetPaConfig (0x95) command. paLut is always sent as 0x01 per
 * the datasheet.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_pa_config(sx1262_t *p, uint8_t pa_duty_cycle, uint8_t hp_max, uint8_t device_sel);

/**
 * sx1262_set_tx_params
 * @param p - Pointer to an initialized sx1262_t struct
 * @param power - Output power in dBm (Section 13.4.4)
 * @param ramp_time - One of SX1262_RAMP_*
 *
 * Sends the SetTxParams (0x8E) command.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_tx_params(sx1262_t *p, int8_t power, uint8_t ramp_time);

/**
 * sx1262_set_buffer_base_address
 * @param p - Pointer to an initialized sx1262_t struct
 * @param tx_base_addr - TX base address in the data buffer
 * @param rx_base_addr - RX base address in the data buffer
 *
 * Sends the SetBufferBaseAddress (0x8F) command.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_buffer_base_address(sx1262_t *p, uint8_t tx_base_addr, uint8_t rx_base_addr);

/**
 * sx1262_write_buffer
 * @param p - Pointer to an initialized sx1262_t struct
 * @param offset - Offset into the data buffer to start writing at
 * @param data - Bytes to write
 * @param len - Number of bytes to write, up to SX1262_MAX_PAYLOAD_LEN
 *
 * Sends the WriteBuffer (0x0E) command to store a TX payload.
 *
 * @return HAL_OK on success, HAL_ERROR if len > SX1262_MAX_PAYLOAD_LEN, or
 *         the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_write_buffer(sx1262_t *p, uint8_t offset, const uint8_t *data, size_t len);

/**
 * sx1262_read_buffer
 * @param p - Pointer to an initialized sx1262_t struct
 * @param offset - Offset into the data buffer to start reading from
 * @param out_data - Receives the bytes read
 * @param len - Number of bytes to read, up to SX1262_MAX_PAYLOAD_LEN
 *
 * Sends the ReadBuffer (0x1E) command to retrieve a received payload.
 *
 * @return HAL_OK on success, HAL_ERROR if len > SX1262_MAX_PAYLOAD_LEN, or
 *         the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_read_buffer(sx1262_t *p, uint8_t offset, uint8_t *out_data, size_t len);

/**
 * sx1262_set_tx
 * @param p - Pointer to an initialized sx1262_t struct
 * @param timeout - Timeout in steps of 15.625us, or SX1262_RX_TX_TIMEOUT_NONE
 *                  for Tx single mode with no timeout
 *
 * Sends the SetTx (0x83) command, starting transmission of the payload
 * previously written with sx1262_write_buffer().
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_tx(sx1262_t *p, uint32_t timeout);

/**
 * sx1262_set_rx
 * @param p - Pointer to an initialized sx1262_t struct
 * @param timeout - Timeout in steps of 15.625us, SX1262_RX_TX_TIMEOUT_NONE
 *                  for Rx single mode with no timeout, or
 *                  SX1262_RX_TIMEOUT_CONTINUOUS for Rx continuous mode
 *
 * Sends the SetRx (0x82) command, putting the radio in receive mode.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_rx(sx1262_t *p, uint32_t timeout);

/**
 * sx1262_set_dio_irq_params
 * @param p - Pointer to an initialized sx1262_t struct
 * @param irq_mask - Bitmask of IRQs to enable in the IRQ status register
 *                   (Table 13-29), e.g. SX1262_IRQ_ALL
 * @param dio1_mask, dio2_mask, dio3_mask - Bitmasks of IRQs to route to
 *                   each DIO pin; 0 if not using DIO-driven interrupts
 *
 * Sends the SetDioIrqParams (0x08) command. By default all IRQs are
 * masked, so the IRQ status register never latches any event until this
 * is called - required even when polling GetIrqStatus rather than using
 * the DIO pins.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_set_dio_irq_params(sx1262_t *p, uint16_t irq_mask, uint16_t dio1_mask, uint16_t dio2_mask, uint16_t dio3_mask);

/**
 * sx1262_get_irq_status
 * @param p - Pointer to an initialized sx1262_t struct
 * @param out_irq - Receives the 16-bit IRQ status register (Table 13-29)
 *
 * Sends the GetIrqStatus (0x12) command.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_get_irq_status(sx1262_t *p, uint16_t *out_irq);

/**
 * sx1262_clear_irq_status
 * @param p - Pointer to an initialized sx1262_t struct
 * @param clear_mask - Bitmask of IRQs to clear, e.g. SX1262_IRQ_ALL
 *
 * Sends the ClearIrqStatus (0x02) command.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_clear_irq_status(sx1262_t *p, uint16_t clear_mask);

/**
 * sx1262_get_packet_status
 * @param p - Pointer to an initialized sx1262_t struct
 * @param out_rssi_pkt - Receives the averaged RSSI of the last packet, in dBm
 * @param out_snr_pkt_quarter_db - Receives the raw signed SnrPkt register
 *                  value, in steps of 0.25dB (divide by 4 for dB)
 *
 * Sends the GetPacketStatus (0x14) command for the LoRa packet status
 * (Table 13-79). RssiPkt is reported as -RssiPkt/2 dBm. SnrPkt is
 * returned raw (not converted to float) since app_log()'s nano-newlib
 * vsnprintf does not support %f.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_get_packet_status(sx1262_t *p, int8_t *out_rssi_pkt, int8_t *out_snr_pkt_quarter_db);

/**
 * sx1262_get_device_errors
 * @param p - Pointer to an initialized sx1262_t struct
 * @param out_errors - Receives the 16-bit OpError register (Table 13-85)
 *
 * Sends the GetDeviceErrors (0x17) command.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_get_device_errors(sx1262_t *p, uint16_t *out_errors);

/**
 * sx1262_clear_device_errors
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Sends the ClearDeviceErrors (0x07) command.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_clear_device_errors(sx1262_t *p);

#endif /* INC_SX1262_H_ */
