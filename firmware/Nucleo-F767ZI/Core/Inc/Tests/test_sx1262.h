#ifndef INC_TESTS_TEST_SX1262_H_
#define INC_TESTS_TEST_SX1262_H_

#include "sx1262.h"
#include "ssd1309.h"

/* Comment out to exclude the sx1262 bench test from the build */
#define TEST_SX1262

/* Define (with TEST_SX1262) to bring up GFSK instead of LoRa */
//#define TEST_SX1262_GFSK

#ifdef TEST_SX1262

/**
 * test_sx1262_set_oled
 * @param p - Pointer to an initialized ssd1309_t struct, or NULL to
 *            disable the OLED update in test_sx1262_rx_done_handler()
 *
 * Registers the display used by test_sx1262_rx_done_handler() to show
 * the last received packet's payload, RSSI and SNR. NULL until set.
 */
void test_sx1262_set_oled(ssd1309_t *p);

/**
 * test_sx1262_hello
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Resets the SX1262, then sends a GetStatus (0xC0) command over SPI and
 * logs the result and decoded status byte over app_log(). Bench smoke test
 * for SPI2 framing, NSS and BUSY handling.
 */
void test_sx1262_hello(sx1262_t *p);

/**
 * test_sx1262_config
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Resets the SX1262, then sends SetPacketType (LoRa) followed by
 * SetRfFrequency (434.000MHz), SetModulationParams, SetPacketParams,
 * SetBufferBaseAddress, SetPaConfig and SetTxParams, logging the result
 * of each step over app_log(). Bench test for basic LoRa radio
 * configuration. Intended to be run once at startup.
 */
void test_sx1262_config(sx1262_t *p);

/**
 * test_sx1262_config_gfsk
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Resets the SX1262, then sends SetPacketType (GFSK) followed by
 * SetRfFrequency (434.000MHz), SetModulationParams (50kbps, BT=0.5,
 * 117.3kHz Rx BW, 25kHz deviation), SetPacketParams (fixed-length
 * SX1262_MAX_PAYLOAD_LEN payload, CRC-16, whitening on),
 * SetBufferBaseAddress, SetPaConfig and SetTxParams, logging the result
 * of each step over app_log(). Bench test for basic GFSK radio
 * configuration. Intended to be run once at startup, as an alternative
 * to test_sx1262_config() selected via TEST_SX1262_GFSK.
 */
void test_sx1262_config_gfsk(sx1262_t *p);

/**
 * test_sx1262_tx_start
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Writes a SX1262_MAX_PAYLOAD_LEN-byte payload, the first 8 bytes of
 * which are "PING000x" (x = rotating counter digit) and the remainder
 * filler, to the radio's TX buffer and triggers a single SetTx. Returns
 * immediately; completion is signalled by the DIO1 IRQ and handled by
 * sx1262_service_tx().
 */
void test_sx1262_tx_start(sx1262_t *p);

/**
 * test_sx1262_rx_start
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Triggers a SetRx in Rx Single mode with no timeout - the radio stays
 * in RX until a packet is received, however long that takes. Returns
 * immediately; completion is signalled by the DIO1 IRQ and handled by
 * sx1262_service_rx(), which calls this again to re-arm RX after each
 * reception.
 */
void test_sx1262_rx_start(sx1262_t *p);

/**
 * test_sx1262_rx_done_handler
 * @param p - Pointer to the sx1262_t instance that received the packet
 * @param payload - Pointer to the received payload, valid only for the
 *                   duration of this call
 * @param len - Length of payload in bytes (unused - the OLED only shows
 *               the first 8 bytes)
 * @param rssi - Averaged RSSI of the received packet, in dBm
 * @param snr_quarter_db - Raw SnrPkt register value, in steps of 0.25dB
 *                          (0 for GFSK, which has no SNR figure)
 *
 * Toggles LD2, then (if an OLED has been registered via
 * test_sx1262_set_oled()) clears the display and shows the first 8 bytes
 * of the received packet's payload, plus RSSI and SNR. Registered via
 * sx1262_set_rx_done_callback() in test_sx1262_config() - called from
 * sx1262_service_rx() whenever a packet is actually received.
 */
void test_sx1262_rx_done_handler(sx1262_t *p, const uint8_t *payload, size_t len, int8_t rssi, int8_t snr_quarter_db);

/**
 * test_sx1262_tx_done_toggle_led
 * @param p - Pointer to the sx1262_t instance that completed transmission
 *
 * Logs the completed TX payload over app_log() and toggles LD2.
 * Registered via sx1262_set_tx_done_callback() in test_sx1262_config() -
 * called from sx1262_service_tx() whenever a transmission actually
 * completes.
 */
void test_sx1262_tx_done_toggle_led(sx1262_t *p);

#endif /* TEST_SX1262 */

#endif /* INC_TESTS_TEST_SX1262_H_ */
